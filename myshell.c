#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* compile with: 
 * gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 shell.c myshell.c
 */


// arglist - a list of char* arguments (words) provided by the user
// it contains count+1 items, where the last item (arglist[count]) and *only* the last is NULL
// RETURNS - 1 if should continue, 0 otherwise
int process_arglist(int count, char** arglist);

// prepare and finalize calls for initialization and destruction of anything required
int prepare(void);
int finalize(void);






int process_arglist(int count, char** arglist) {
	return 0;
}



int prepare(void) {
	return 0;
}



int finalize(void) {
	return 0;
}
