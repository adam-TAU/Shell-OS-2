#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define bool int
#define true 1
#define false 0


/* compile with: 
 * gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 shell.c myshell.c
 */



/* Submitter: Adam
 * 2022A, Tel Aviv University
 * ==========================
 * Operating Systems, Assignment 2.0
 * Shell Implementation
 */




// arglist - a list of char* arguments (words) provided by the user
// it contains count+1 items, where the last item (arglist[count]) and *only* the last is NULL
// RETURNS - 1 if should continue, 0 otherwise
int process_arglist(int count, char** arglist);

// prepare and finalize calls for initialization and destruction of anything required
int prepare(void);
int finalize(void);


/******************* STATIC FUNCTION DECLARATIONS *******************/

/* Check if the string array contains the given string. Definitions:
 1. In case the string is "&", it must be the last non-NULL string in arglist
 2. In case the string is ">>", it must be the second-to-last non-NULL string in arglist
 * This function also returns the lowest index at which the string was found.
 * If the returned integer is lower than 0, then the string wasn't found.
 * If the returned integer is higher than count, then that is an undefined behavior.
  */
static int contains(int count, char** arglist, char* string);

/* Searches through a string array of size <count>, and searches if a particular <string> is contained within it.
 * This function also returns the lowest index at which the string was found.
 * If the returned integer is lower than 0, then the string wasn't found.
 * If the returned integer is higher than count, then that is an undefined behavior.
 */
static int index_of(int count, char** arglist, char* string);

/* Given a command line argument array (including the binary's name), execute it with its arguments as a child process.
 * IF the given <output_file> == NULL, then the output should go out to stdout.
 * ELSE, the output should be redirected to <output_file>.
 * 
 * IF the given <input_file> == NULL, then the input should go out to stdin.
 * ELSE, the input should be redirected to <input_file>.
 */
static int execute(char** argv, char* output_file, char* input_file);
/********************************************************************/




/* STATIC FUNCTION DEFINITIONS */
static int contains(int count, char** arglist, char* string) {
	if (strcmp(string, "&")) {
		return strcmp(arglist[count - 1], "&");
	} else if (strcmp(string, ">>")) {
		return strcmp(arglist[count - 2], ">>");
	} else if (strcmp(string, "|")) {
		return index_of(count, arglist, "|");
	} else {
		return index_of(count, arglist, string);
	}
}

static int index_of(int count, char** arglist, char* string) {
	int i;
	
	for (i = 0; i < count; i++) {
		if (strcmp(arglist[i], string)) {
			return i;
		}
	}
	
	return -1;
}

static int execute(char** argv, char* output_file, char* input_file) {
	return 0;
}
/******************************/






/******************* MAIN MECHANISM's FUNCTION DEFINITIONS *******************/
int process_arglist(int count, char** arglist) {
	return 0;
}



int prepare(void) {
	return 0;
}



int finalize(void) {
	return 0;
}
/***************************************************************************/

