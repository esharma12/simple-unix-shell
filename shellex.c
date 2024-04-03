/////////////////////////////////////////////////////////////////////////////
//
// File				: shellex.c
// Description		: This is a file with code for a simple shell for CMSC 257 Project 3
//
// Author			: Esha Sharma 
// Last Modified 	: 12/03/21 1:20 PM
//
/* $begin shellmain */
#include "csapp.h"
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
  

int main(int argc, char **argv) 
{
	char cmdline[MAXLINE]; /* Command line */

	signal(SIGINT, SIG_IGN); /* Handle SIGINT signal and ignore it */ 

	while (1) {
		/* Read */
		if (argc > 1) { /* Check for # of arguments to see if user wants to change the prompt */
			if (!strcmp(argv[1], "-p")) {
				printf("%s> ", argv[2]);
			}
		}
		else {	/* default prompt */
			printf("sh257> ");
		}              
		Fgets(cmdline, MAXLINE, stdin); 
		if (feof(stdin))
			exit(0);

		/* Evaluate */
		eval(cmdline);
	} 
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
	char *argv[MAXARGS]; /* Argument list execve() */
	char buf[MAXLINE];   /* Holds modified command line */
	int bg;              /* Should the job run in bg or fg? */
	pid_t pid;           /* Process id */
	int exit_status;     /* Exit status of child process */
	int status;	     /* Status of child process */ 
	strcpy(buf, cmdline);
	bg = parseline(buf, argv); 
	if (argv[0] == NULL)  
		return;   /* Ignore empty lines */

	if (!builtin_command(argv)) { 
		if ((pid = Fork()) == 0) {   /* Child runs user job */
			if (execvp(argv[0], argv) < 0) {/* check for unsuccessful command*/
				printf("Execution failed (in fork)\n");
				printf("%s: Command not found.\n", argv[0]);
				exit(1); /*execvp is unsuccessful, so exit status of 1 */
			}
		}

		/* Parent waits for foreground job to terminate */
		if (!bg) {
			if (waitpid(pid, &status, 0) < 0)
				unix_error("waitfg: waitpid error");
		}
		else
			printf("%d %s", pid, cmdline);
		/* Evaluates exit status of child processes and prints it out */	
		if (WIFEXITED(status)) {
			exit_status = WEXITSTATUS(status);
			printf("Process exited with status code %d\n", exit_status);			
		}
	}
	return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
	if (!strcmp(argv[0], "quit")) /* quit command */
		exit(0);  
	if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
		return 1;
	if (!strcmp(argv[0], "exit")) { /* exit command using raise() */
		raise(SIGTERM);		
	}
	if (!strcmp(argv[0], "pid")) { /* pid command prints pid of shell */
		printf("%d\n", getpid());
		return 1;
	}
	if (!strcmp(argv[0], "ppid")) { /* ppid command prints parent pid of shell */
		printf("%d\n", getppid());
		return 1;
	}
	if (!strcmp(argv[0], "help")) { /* prints shell developer name, usage information and how to use man for non-built-in commands */
		printf("****************************************************************************\n");
		printf("A Custom Shell for CMSC 257\n");
		printf("  -  esharma\n");
		printf("Usage: \n");
		printf("  -  To change the prompt, type <executable> -p <prompt> when running the executable program file.\n");
		printf("****************************************************************************\n");
		printf("\nBUILT-IN COMMANDS: \n");
		printf("  -  exit: Exits the shell\n");
		printf("  -  pid:  Prints the process id of the shell\n");
		printf("  -  ppid: Prints the parent process id of the shell\n");
		printf("  -  help: Prints helpful information about the shell\n");
		printf("  -  cd:   Prints the current working directory, 'cd <path>' will change the current working directory\n");
		printf("SYSTEM COMMANDS: \n");
		printf("  -  Refer to the Linux man pages for more information on system commands by typing 'man <command_name>'\n");
		printf("\n");
		return 1; 
	}
	if (!strcmp(argv[0], "cd")) {
		if (argv[1] != '\0') { /*change the current working directory */
			chdir(argv[1]);
		}
		else { /* prints the current working directory */
			char cwd[100];
			getcwd(cwd, 100);
			printf("%s\n", cwd);
		}
		return 1;	
	}
	return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
	char *delim;         /* Points to first space delimiter */
	int argc;            /* Number of args */
	int bg;              /* Background job? */

	buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
	while (*buf && (*buf == ' ')) /* Ignore leading spaces */
		buf++;

	/* Build the argv list */
	argc = 0;
	while ((delim = strchr(buf, ' '))) {
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while (*buf && (*buf == ' ')) /* Ignore spaces */
			buf++;
	}
	argv[argc] = NULL;

	if (argc == 0)  /* Ignore blank line */
		return 1;

	/* Should the job run in the background? */
	if ((bg = (*argv[argc-1] == '&')) != 0)
		argv[--argc] = NULL;

	return bg;
}
/* $end parseline */
