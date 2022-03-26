#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>


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



/***************** INSTRUCTIONS THAT HAVE YET TO BE HANDLED/UNDERSTANDED *****************

1. User commands might be invalid (e.g., a non-existing program or redirecting to a file without
write permission). Such cases should be treated as an error in the child process (i.e., they must
not terminate the shell).

2. Assume that the results of the provided parser are correct.

3. Actual implementations of the operations:
	
	b. Assume background processes don’t read input (stdin).
	
	c. Use the same array for all execvp() calls by referencing items in arglist. There’s no need
	to allocate a new array and duplicate parts of the original array.
	
	d. If arglist contains the word "|" (a single pipe symbol), run two child processes, with the
	output (stdout) of the first process (executing the command that appears before the pipe)
	piped to the input (stdin) of the second process (executing the command that appears after
	the pipe).
	

4. figure out the use for wait and waitpid

5. If an error occurs in a signal handler in the shell parent process, there’s no need to notify
process_arglist(). Just print a proper error message and terminate the shell process with
exit(1).

6. If wait/waitpid in the shell parent process return an error for one of the following reasons, it is
not considered an actual error that requires exiting the shell:
• ECHILD
• EINTR. (You can also avoid an EINTR “error” in the first place. Hint: read about the
SA_RESTART option in sigaction.)

7. In the original (shell/parent) process, process_arglist() should return 1 if no error occurs. (This
makes sure the shell continues processing user commands.) If process_arglist() encounters an
error, it should print an error message and return 0. (See below for what constitutes an error.)

8. The process_arglist() function should not return until every foreground child process it
created exits.

****************************************************************************************/


// arglist - a list of char* arguments (words) provided by the user
// it contains count+1 items, where the last item (arglist[count]) and *only* the last is NULL
// RETURNS - 1 if should continue, 0 otherwise
int process_arglist(int count, char** arglist);

// prepare and finalize calls for initialization and destruction of anything required
int prepare(void);
int finalize(void);


/******************* STATIC AUXILIARY FUNCTION DECLARATIONS *******************/

/* Duplicates file descriptor safely.
 * On error, terminates process if is a child process.
 * Returns 0 on success and -1 on failure. */
static int dup2_safe(int new_fd, int old_fd, bool is_child);

/* Opens a file safely. 
 * On error, terminates process if is a child process.
 * Returns the file descriptor on success and -1 on failure. */
static int open_safe(char* filename, bool is_child);

/* This function gets a file descriptor, and if it's non-negative, it tries to close it safely.
 * On error, terminates a process if is a child process.
 * Returns 0 on success and -1 on failure. */
static int close_safe(int fd, bool is_child);

/* Print an error, and if this function was called from a child process, also exit(1) it.
 * In case <is_child> is true, any errors will terminate the calling process. */
static void print_err(char* error_message, bool is_child);

/* Given a command line argument array (including the binary's name), execute it with its arguments as a child process.
 * IF the given <output_fd> is lower than 0, then the output should go out to stdout.
 * ELSE, the output should be redirected to <output_fd>.
 * 
 * IF the given <input_fd> is lower than 0, then the input should go out to stdin.
 * ELSE, the input should be redirected to <input_fd>.
 *
 * This function returns a negative number in case of an error. 
 * ELSE, this function returns the process id of the child process it has created.
 */
static pid_t execute(char** argv, bool background, int output_fd, int input_fd);

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
/********************************************************************/



/******************** STATIC MAIN MECHANISM'S FUNCTION DECLARATIONS **************/

/* Runs the command in the background.
 * <count> is the amonut of command line arguments that were in the issued command.
 * <arglist> is the parsed array of command line arguments.
 * Returns 0 on success, and -1 on failure. */
static int handle_background(int count, char** arglist);

/* Perform the pipe command.
 * <index> is the index of the pipe character in the word array <arglist> of the command line arguments.
 * <count> is the amonut of command line arguments that were in the issued command.
 * <arglist> is the parsed array of command line arguments.
 * Returns 0 on success, and -1 on failure. */
static int handle_pipe(int count, char** arglist, int index);

/* Perform the output redirection command.
 * <count> is the amonut of command line arguments that were in the issued command.
 * <arglist> is the parsed array of command line arguments.
 * Returns 0 on success, and -1 on failure. */
static int handle_output_redirection(int count, char** arglist);

/**********************************************************************************/





/******************** STATIC AUXILIARY FUNCTION DEFINITIONS ********************/

static int dup2_safe(int new_fd, int old_fd, bool is_child) {
	if (dup2(new_fd, old_fd) < 0) {
		print_err(DUP_ERR, is_child);
		return -1;
	}
	
	return 0;
} 

static int open_safe(char* filename, bool is_child) {
	int fd = -1;
	
	if (filename != NULL) {
		fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); 
		
		if (fd < 0) {
			print_err(OPEN_ERR, is_child);
			return -1;
		}
	}
	
	return fd;
}

static int close_safe(int fd, bool is_child) {
	if (fd >= 0) {
		if (close(fd) < 0) {
			print_err(CLOSE_ERR, is_child);
			return -1;
		}
	}
	
	return 0;
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
	} else { // if it's a pipe operation, or a non "output redirection" / "background process", then the placement will be random
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

static pid_t execute(char** argv, bool background, int output_fd, int input_fd) {
	/* This functions's implementation is inspired by: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html */
	pid_t  pid;
	int status;

	if ((pid = fork()) < 0) { // fork a child process:
		print_err("*** ERROR: forking child process failed\n", false);
		return -1;
	}
	else if (pid == 0) { // for the child process:
		if (dup2_safe(output_fd, STDOUT_FILENO, true) < 0) { // redirect output file
			return -1;
		}
		if (dup2_safe(input_fd, STDIN_FILENO, true) < 0) { // redirect input file
			return -1;
		}
		
		if (background) { // background processes shouldn't be affected by SIGINT
			signal(SIGINT, SIG_IGN);
		} else { // foreground processes should be terminated upon a SIGINT signal
			signal(SIGINT, SIG_DFL);
		}
		
		/* Execute */
		if (execvp(*argv, argv) < 0) {     /* execute the command  */
			print_err("*** ERROR: exec failed\n", true);
			exit(1);
		}
	
	}
	else if (!background) { // for the parent:
		while (wait(&status) != pid); // wait for completion 
	}

	return pid;
}
/***************************************************************/


/***************************** STATIC MAIN MECHANISM'S FUNCTION DEFINITIONS ****************************/
static int handle_pipe(int count, char** arglist, int index) {
	pid_t pid1, pid2;
	int status;
	
	/* Creating the pipe named <pfds> */
	int pfds[2];
	pipe(pfds);
	
	/* Executing the first program */
	arglist[index] = NULL; // nullifying the arglist at the pipe stage, so that the first process would see it as the end of its command line argument list
	if ( (pid1 = execute(arglist, true, pfds[1], -1)) < 0) { // executing the program as a background process with its output redirected to the pipe[1] writing pipe
		return -1;
	}
	
	/* Executing the second program */
	if ( (pid2 = execute(arglist + index + 1, true, -1, pfds[0])) < 0) { // executing the program as a background process with its input redirected to the pipe[0] reading pipe
		return -1;
	}
	
	/* Waiting for both processes to finish */
	waitpid(pid1, &status, 0);
	waitpid(pid2, &status, 0);
	
	/* Closing the pipe */
	if (close_safe(pfds[0], false) < 0) return -1;
	if (close_safe(pfds[1], false) < 0) return -1;
	
	return 0;
}

static int handle_output_redirection(int count, char** arglist) {

	/* Open the output file */
	int output_fd;
	if ( (output_fd = open_safe(arglist[count - 1], false)) < 0) {
		return -1;
	}
	
	/* Format the arglist according to our needs */
	arglist[count - 1] = NULL; // the filename
	arglist[count - 2] = NULL; // the ">>" symbol
	
	/* Run the program */
	if (execute(arglist, false, output_fd, -1) < 0) {
		return -1;
	}
	
	return 0;
}


static int handle_background(int count, char** arglist) {
	arglist[count - 1] = NULL; // removing "&" from the arglist
	if (execute(arglist, true, -1, -1) < 0) { // executing the program as a background process without output/input redirections
		return -1;
	}
	return 0;
}



/******************* MAIN MECHANISM's FUNCTION DEFINITIONS *******************/
int process_arglist(int count, char** arglist) {
	
	int index;
	
	if ( (index = contains(count, arglist, "&")) >= 0 ) { // a background process
		if (handle_background(count, arglist) < 0) {
			return 0;
		}
	} else if ( (index = contains(count, arglist, "|")) >= 0 ) { // piped processes running concurrently		
		if (handle_pipe(count, arglist, index) < 0) {
			return 0;
		}
	} else if ( (index = contains(count, arglist, ">>")) >= 0 ) { // output redirection process
		if (handle_output_redirection(count, arglist) < 0) {
			return 0;
		}
		
	} else { // regularly executing a program with a non-complex command line argument array
		if (execute(arglist, false, -1, -1) < 0) {
			return 0;
		} 
	}

	return 1;
}



int prepare(void) {
	signal(SIGCHLD, SIG_IGN); // Silently (and portably) terminate children who had stopped their execution.
	signal(SIGINT, SIG_IGN); // the parent (shell) should not terminate upon SIGINT.
	return 0;
}



int finalize(void) {
	return 0;
}
/***************************************************************************/

