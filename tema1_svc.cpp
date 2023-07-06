/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "tema1.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>



#define MAX_RESOURCE_LEN 100
#define MAX_ENTRY_LEN 300
#define SEP ","

using namespace std;

unordered_set<string> users;
unordered_set<string> resources;
vector<unordered_map<string, string>> approvals;
int availability;

// add user ids from database file in memory (users set)
static void init_clients(char *clients_filename)
{
	FILE *fin = fopen(clients_filename, "r");
	if(!fin){
		fprintf(stderr, "unable to open user IDs file\n");
		exit(1);
	}
	int cnt_users, i;
	fscanf(fin, "%d", &cnt_users);
	char *user_id = (char *)calloc(TOKEN_LEN, sizeof(char));
	for(i = 0; i < cnt_users; i++){
		fscanf(fin, "%s", user_id);
		string current_user(user_id);
		users.insert(current_user);
	}
	free(user_id);
	fclose(fin);
}

// add available resources from database file in memory (resources set)
static void init_resources(char *resources_filename)
{
	FILE *fin = fopen(resources_filename, "r");
	if(!fin){
		fprintf(stderr, "unable to open resource database");
		exit(1);
	}

	int cnt_resources, i;
	fscanf(fin, "%d", &cnt_resources);
	char *resource = (char *)calloc(MAX_RESOURCE_LEN, sizeof(char));

	for(i = 0; i < cnt_resources; i++){
		fscanf(fin, "%s", resource);
		string current_res(resource);
		resources.insert(current_res);
	}
	free(resource);
	fclose(fin);
}

// add available permissions at each request step(i.e. each line from file)
// in memory (vector approvals of unordered maps)
static void init_approvals(char *approvals_filename){
	FILE *fin = fopen(approvals_filename, "r");
	if(!fin){
		fprintf(stderr, "unable to open approvals file");
		exit(1);
	}

	char *entry = (char *)calloc(MAX_ENTRY_LEN, sizeof(char));
	char *p;
	int pos;
	string resource;
	string allowed_ops;

	while(fgets(entry, MAX_ENTRY_LEN, fin)){

		if(entry[strlen(entry)-1] == '\n')
			entry[strlen(entry)-1] = '\0';
		p = strtok(entry, SEP);

		pos = 0;
		unordered_map<string, string> permissions;
		while(p){
			if(pos % 2 == 0){
				resource = p;
			}else{
				allowed_ops = p;
				permissions[resource] = allowed_ops;
			}
			p = strtok(NULL , SEP);
			pos++;
		}
		approvals.push_back(permissions);
	}
	fclose(fin);
	free(entry);
}



#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

static void
tema1_prog_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	union {
		char *request_auth_token_1_arg;
		struct access_params request_access_token_1_arg;
		struct valid_action_params validate_delegated_action_1_arg;
		char *approve_auth_token_1_arg;
		struct refresh_params get_fresh_token_1_arg;
	} argument;
	char *result;
	xdrproc_t _xdr_argument, _xdr_result;
	char *(*local)(char *, struct svc_req *);

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
		return;

	case request_auth_token:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_auth_response;
		local = (char *(*)(char *, struct svc_req *)) request_auth_token_1_svc;
		break;

	case request_access_token:
		_xdr_argument = (xdrproc_t) xdr_access_params;
		_xdr_result = (xdrproc_t) xdr_access_response;
		local = (char *(*)(char *, struct svc_req *)) request_access_token_1_svc;
		break;

	case validate_delegated_action:
		_xdr_argument = (xdrproc_t) xdr_valid_action_params;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (char *(*)(char *, struct svc_req *)) validate_delegated_action_1_svc;
		break;

	case approve_auth_token:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_wrapstring;
		local = (char *(*)(char *, struct svc_req *)) approve_auth_token_1_svc;
		break;

	case get_fresh_token:
		_xdr_argument = (xdrproc_t) xdr_refresh_params;
		_xdr_result = (xdrproc_t) xdr_access_response;
		local = (char *(*)(char *, struct svc_req *)) get_fresh_token_1_svc;
		break;

	default:
		svcerr_noproc (transp);
		return;
	}
	memset ((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode (transp);
		return;
	}
	result = (*local)((char *)&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t) _xdr_result, result)) {
		svcerr_systemerr (transp);
	}
	if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		fprintf (stderr, "%s", "unable to free arguments");
		exit (1);
	}
	return;
}

int
main (int argc, char **argv)
{
	register SVCXPRT *transp;
// disable buffering of stdout
// useful/necessary for checker
	setbuf(stdout, NULL);
	pmap_unset (TEMA1_PROG, TEMA1_VERS);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, TEMA1_PROG, TEMA1_VERS, tema1_prog_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (TEMA1_PROG, TEMA1_VERS, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, TEMA1_PROG, TEMA1_VERS, tema1_prog_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (TEMA1_PROG, TEMA1_VERS, tcp).");
		exit(1);
	}

	init_clients(argv[1]);
	init_resources(argv[2]);
	init_approvals(argv[3]);
	availability = atoi(argv[4]);
	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	exit (1);
	/* NOTREACHED */
}
