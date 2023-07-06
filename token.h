#ifndef _TOKEN_H
#define _TOKEN_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define TOKEN_LEN 15

/**
 * generate alpha-numeric string based on random char*
 * 
 * INPUT: fixed length of 16
 * OUTPUT: rotated string
 * */
extern char* generate_access_token(char* clientIdToken);

#endif