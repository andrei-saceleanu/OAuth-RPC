#include "tema1.h"
#include "token.h"
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

// extern variables are initialized in another object file
// (svc file where main and init functions can be found)
extern unordered_set<string> users;
extern unordered_set<string> resources;
extern vector<unordered_map<string, string>> approvals;
extern int availability;

// map authorization/access token to permissions
// the reason why both types of tokens are used is detailed below
unordered_map<string, unordered_map<string, string>> token2res;

// map token to remaining available operations until expiration 
unordered_map<string, int> token2avail;

// map token to user id and also the reverse
unordered_map<string, string> token2user;
unordered_map<string, string> user2token;

// request authorization token
// if the input user id cannot be found in the users database,
// return with appropriate error code
// else, pass to client an authorization token
auth_response* request_auth_token_1_svc(char **id, struct svc_req *cl)
{
    static auth_response res;
    printf("BEGIN %s AUTHZ\n", *id);
    string s(*id);
    if(users.find(s) == users.end()){
        res.code = USER_NOT_FOUND;
        return &res;
    }
    char *token = generate_access_token(*id);
    res.token = strdup(token);
    res.code = OK;
    printf("  RequestToken = %s\n", token);
    return &res;
}

// request access token
// if the authorization token was not approved, send request denied
access_response* request_access_token_1_svc(struct access_params *p, struct svc_req *cl)
{
    static access_response acc_res;
    string s(p->token);

// the presence of token in token2res map would mean it was approved
    if(token2res.find(s) == token2res.end()){
        acc_res.code = REQUEST_DENIED;
        return &acc_res;
    }

    char *token = generate_access_token(p->token);
    acc_res.token = strdup(token);
    acc_res.availability = availability;

// if refresh is active, also generate a refresh token
// else, duplicate token(it is not used, but some errors were thrown
// if refresh_token field was not set)
    if(p->refresh)
        acc_res.refresh_token = generate_access_token(token);
    else
        acc_res.refresh_token = strdup(token);

    acc_res.code = OK;

    string t(token);
    string user_id(p->id);

// the authorization token is replaced with the access token
// in the corresponding maps
    token2user[t] = user_id;
    user2token[user_id] = t;
    token2res[t] = token2res[s];
    token2res.erase(s);
    token2avail[t] = availability;

    printf("  AccessToken = %s\n", token);
    if(p->refresh)
        printf("  RefreshToken = %s\n", acc_res.refresh_token);
    return &acc_res;
}

// validate delegated action
int * validate_delegated_action_1_svc(struct valid_action_params *p, struct svc_req *cl)
{
    static int res;
    string t(p->access_token);
    string current_resource(p->resource);

// the permissions are stored in a map in a key:value pair
// of the form (RESOURCE:OPCODES), where OPCODES are chars
// in the set RIMDX
// that is why the opcode is extracted from the full name of the action
    char opcode = p->op[0];
    if(strstr(p->op,"EXECUTE")==p->op){
        opcode = 'X';
    }

    if(token2user.find(t) == token2user.end()){
        res = PERMISSION_DENIED;
    }else if(token2avail[t] <= 0){
        memset(p->access_token, 0, strlen(p->access_token) * sizeof(char));
        res = TOKEN_EXPIRED;
    }else if(resources.find(current_resource) == resources.end()){
        res = RESOURCE_NOT_FOUND;
    }else if(token2res[t][current_resource].find(opcode) == string::npos){
        res = OPERATION_NOT_PERMITTED;
    }else{
        res = PERMISSION_GRANTED;
    }

// decrease available operations counter for current token
    token2avail[t] = (token2avail[t] - 1 < 0)? 0 : token2avail[t] - 1;

    if(res == PERMISSION_GRANTED){
        printf("PERMIT (%s,%s,%s,%d)\n",
            p->op, p->resource, p->access_token, token2avail[t]);
    }else{
        printf("DENY (%s,%s,%s,%d)\n",
            p->op, p->resource, p->access_token, token2avail[t]);
    }
    return &res;
}

// approve request/authorization token
char ** approve_auth_token_1_svc(char **token, struct svc_req *cl)
{
// each request increments this index
// the index is basically the line index in the approvals.db file
// approvals unordered_map comes with initialized content from the svc file
    static int op_idx = 0;

    string s(*token);
    unordered_map<string, string> perms = approvals[op_idx];

// if we find "*" (i.e. * -> -), the token is denied
    if(perms.find("*") != perms.end()){
        op_idx++;
        return token;
    }

    token2res[s] = perms;
    op_idx++;
    return token;
}

// get new access token(and a new refresh token) using a refresh token
access_response* get_fresh_token_1_svc(struct refresh_params *p, struct svc_req *cl)
{
    static access_response acc_res;
    string user_token(p->access_token);
    printf("BEGIN %s AUTHZ REFRESH\n", token2user[p->access_token].c_str());

    char *token = generate_access_token(p->refresh_token);
    acc_res.token = strdup(token);
    acc_res.availability = availability;
    acc_res.refresh_token = generate_access_token(token);
    acc_res.code = OK;
    
    string t(token);
    
// update correspoding associations between tokens and user ids
// and set the number of available operations for the new token
    token2res[t] = token2res[user_token];
    token2res.erase(user_token);

    token2user[t] = token2user[user_token];
    user2token[token2user[user_token]] = t;
    token2avail[t] = availability;

    printf("  AccessToken = %s\n", token);
    printf("  RefreshToken = %s\n", acc_res.refresh_token);
    return &acc_res;
}