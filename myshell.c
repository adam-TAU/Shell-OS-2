#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Auxiliary macros */
#define bool int
#define true 1
#define false 0

/* Error messages */
#define OPEN_ERR "An error with opening a file has occurred"
#define CLOSE_ERR "An error with closing a file has occurred"
#define DUP_ERR "An error duplicating a file descriptor (dup2) has occurred"

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

/* This function gets a file descriptor, and if it's non-negative, it tries to close it */
static void close_safe(int fd, bool is_child);

/* Print an error, and if this function was called from a child process, also exit(1) it */
static void print_err(char* error_message, bool is_child);

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
static int execute(char** argv, bool background, char* output_file, char* input_file);
/********************************************************************/




/******************** STATIC FUNCTION DEFINITIONS ********************/

static void close_safe(int fd, bool is_child) {
	if (fd >= 0) {
		if (close(fd) < 0) {
			print_err(CLOSE_ERR, is_child);
		}
	}
}

static void print_err(char* error_message, bool is_child) {
	fprintf(stderr, error_message);
	
	if (is_child) {
		exit(1);
	}
}

static int contains(int count, char** arglist, char* string) {
	if (strcmp(string, "&")) {
		return strcmp(arglist[count - 1], "&");
	} else if (strcmp(string, ">>")) {
		return strcmp(arglist[count - 2], ">>");
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

static int execute(char** argv, bool background, char* output_file, char* input_file) {
	/* This functions's implementation is inspired by: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html */
	pid_t  pid;
	int status;
	int out_fd = -1;
	int in_fd = -1;

	if ((pid = fork()) < 0) {     /* fork a child process           */
		print_err("*** ERROR: forking child process failed\n", false);
	}
	else if (pid == 0) {          /* for the child process:         */
		
		/* Redirect output file */
		if (output_file != NULL) {
			out_fd = open(output_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); 
			
			if (out_fd < 0) { // Error with opening file
				print_err(OPEN_ERR, true);
			}
			if (dup2(out_fd, STDOUT_FILENO) < 0) { // Error with duplicating a file descriptor
				print_err(DUP_ERR, true);
			}
		}
		
		/* Redirect input file */
		if (input_file != NULL) {
			in_fd = open(input_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
			
			if (in_fd < 0) { // Error with opening file
				print_err(OPEN_ERR, true);
			}
			if (dup2(in_fd, STDIN_FILENO) < 0) { // Error with duplicating a file descriptor
				print_err(DUP_ERR, true);
			}
		}
		
		/* Execute */
		if (execvp(*argv, argv) < 0) {     /* execute the command  */
			if (out_fd != -1) close(out_fd);
			if (in_fd != -1) close(in_fd);
			print_err("*** ERROR: exec failed\n", true);
			exit(1);
		}
	
	}
	else if (!background) {                    /* for the parent:      */
		while (wait(&status) != pid);       /* wait for completion  */
		
		/* Closing the input/output files that were for the purpose of redirection, if any were used */
		close_safe(out_fd, false);
		close_safe(in_fd, false);
	}

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

