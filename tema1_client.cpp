#include "tema1.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <rpc/rpc.h>
#include <unordered_map>
#include <iostream>

#define MAX_CMD_LEN 100
#define SEP ","
#define PROTOCOL "tcp"

using namespace std;

// map user to count of remaining ops until his current token expires
unordered_map<string,int> user2rem; 

// map user to current access token
unordered_map<string,string> user2token;

// map user to current refresh token(if any)
unordered_map<string,string> user2ref_token;

// map user to refresh flag(refresh is enabled or not)
unordered_map<string,int> user2ref;


int main(int argc, char **argv){
    if(argc != 3){
        fprintf(stderr,"Usage: ./client <server_address> <client_operations_file>\n");
        exit(-1);
    }

    FILE *fin = fopen(argv[2], "r");
    if(!fin){
        fprintf(stderr,"unable to open operations file\n");
        exit(-1);
    }

    CLIENT *handle;
    handle = clnt_create(argv[1], TEMA1_PROG, TEMA1_VERS, PROTOCOL);
	if (!handle) {
		perror("Failed to create client handle");
		clnt_pcreateerror(argv[0]);
		return -2;
	}

// init params and response structures
    auth_response *res;
    access_response *acc_res;
    access_params *aparams;
    refresh_params *rparams;
    valid_action_params *vparams;
    vparams = (valid_action_params *)malloc(sizeof(valid_action_params));
    aparams = (access_params *)malloc(sizeof(access_params));
    rparams = (refresh_params *)malloc(sizeof(refresh_params));

    string st;
    char *command = (char *)calloc(MAX_CMD_LEN, sizeof(char));

// parsed elements from an operation
// e.g. for a request, action=REQUEST, param=0/1
// for RIMDX, action=ACTION, param=RESOURCE
    char *id, *action, *param;
    int *status;

// when the number of available operations for an user is 0,
// we need this for the reset that happens if refresh is active 
    int global_avail = 0;

    
    while(fgets(command, MAX_CMD_LEN, fin)){
        if(command[strlen(command)-1]=='\n')
            command[strlen(command)-1] = '\0';
        id = strtok(command, SEP);
        action = strtok(NULL, SEP);
        param = strtok(NULL, SEP);
        string user_id(id);

        if(strstr(action, "REQUEST") == action){
            res = request_auth_token_1(&id, handle);
            if(res->code == OK){

                approve_auth_token_1(&(res->token), handle);
                aparams->id = strdup(id);
                aparams->token = strdup(res->token);
                aparams->refresh = atoi(param);

                acc_res = request_access_token_1(aparams, handle);
                user2rem[user_id] = acc_res->availability;
                global_avail = acc_res->availability;
                user2ref[user_id] = aparams->refresh;

                if(acc_res->code == OK){
                    st = acc_res->token;
                    user2token[user_id] = st;
                    if(user2ref[user_id] == 0)
                        printf("%s -> %s\n", aparams->token, acc_res->token);
                    else{
                        st = acc_res->refresh_token;
                        user2ref_token[user_id] = st;
                        printf("%s -> %s,%s\n", aparams->token, acc_res->token, acc_res->refresh_token);
                    }
                }else{
                    printf("REQUEST_DENIED\n");
                }
            }else{
                printf("USER_NOT_FOUND\n");
            }

        }else{

// if the current action is bound to be associated with TOKEN_EXPIRED
// (i.e. remaining ops <=0) and refresh is enabled, do necessary action
// (i.e. request fresh access token using the refresh token) in order to
// avoid the error code
            if(user2rem[user_id] <= 0 && user2ref[user_id] == 1){
                rparams->access_token = strdup(user2token[user_id].c_str());
                rparams->refresh_token = strdup(user2ref_token[user_id].c_str());
                rparams->refresh = 1;

                acc_res = get_fresh_token_1(rparams, handle);

// update corresponding association structures
                st = acc_res->token;
                user2token[user_id] = st;
                st = acc_res->refresh_token;
                user2ref_token[user_id] = st;
                user2rem[user_id] = global_avail;
            }

// proceed with delegated action
            vparams->op = strdup(action);
            vparams->resource = strdup(param);
            vparams->access_token = strdup(user2token[user_id].c_str());
            status = validate_delegated_action_1(vparams, handle);
            switch (*status)
            {
                case PERMISSION_DENIED:
                    printf("PERMISSION_DENIED\n");
                    break;
                case TOKEN_EXPIRED:
                    printf("TOKEN_EXPIRED\n");
                    break;
                case RESOURCE_NOT_FOUND:
                    printf("RESOURCE_NOT_FOUND\n");
                    break;
                case OPERATION_NOT_PERMITTED:
                    printf("OPERATION_NOT_PERMITTED\n");
                    break;
                case PERMISSION_GRANTED:
                    printf("PERMISSION_GRANTED\n");
                    break;
                default:
                    break;
            }
            user2rem[user_id]--;
        }
    }

    clnt_destroy(handle);
    fclose(fin);
    free(aparams);
    free(vparams);
    free(rparams);
    free(command);
    return 0;
}