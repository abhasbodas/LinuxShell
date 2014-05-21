/*
UNI: ab3599
myshell
COMS 4118 OS Homework 1
*/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "linkedlist.c"
//#include <>

const char *SUPPORTED_COMMANDS[] = {"cd", "exit", "path"};
const char *SPECIAL_CHARS[] = {" ", "<", ">", "2>", "|"}; //strings used as delims for tokenizer calls

struct node *pathlist = NULL;

void removeNewLineChar(char *consoleinput)
{
	int length = strlen(consoleinput);
	if (consoleinput[length-1] == '\n')
		{
			consoleinput[length-1] = '\0';
		}
}

void removeWhiteSpaces(char *consoleinput)
{
	int length = strlen(consoleinput);
	int i=0;
	while(consoleinput[length-1] == ' ')	//remove whitespaces after input
	{
		consoleinput[length-1] = '\0';
		length = strlen(consoleinput);
	}
	int spaces = 0;							//remove whitespaces before input
	for(i=0;consoleinput[i]==' ';i++,spaces++);
	
	for(i=0;i<=strlen(consoleinput);i++)
	{
		consoleinput[i] = consoleinput[i+spaces];
	}
}

void readCommand(char *consoleinput)
{
	consoleinput = fgets(consoleinput, 256, stdin);
}

char *parseDir(char * consoleinput)	//returns first arg after command, using whitespace as delimiter
{
	char *token = (char *)strtok(consoleinput, SPECIAL_CHARS[0]);
	
	token = (char *)strtok(NULL, SPECIAL_CHARS[0]);
	//printf("\npath given to cd:%s", token);
	return token;
}

int runChangeDir(char *consoleinput)
{
	int result =0;
	if(strlen(consoleinput)<=3)
	{
		printf("cd error: no path specified.");
	}
	else
	{
		char *pathname = parseDir(consoleinput);
		result = chdir(pathname);
	}
	return (result==0)?0:errno;
}

void getCurrentDir(char * cwd)
{
	getcwd(cwd, 256);
}

int runPath(char *consoleinput)
{
	if(strlen(consoleinput) == 4)	//path without any args
	{
		if(pathlist!=NULL)
		{
			printList(pathlist);
		}
		else
		{
			printf("[path variable not set]");
		}
	}
	else						//path with args
	{
		char *args = strndup(consoleinput+4, strlen(consoleinput)-4);	//remove "path" from input
		removeWhiteSpaces(args);
		
		char *newpath = strndup(args+1, strlen(args)-1);	//remove + or - from input			
		removeWhiteSpaces(newpath);
		
		if(args[0]=='+')
		{
			//add to linkedlist
			printf("\nAdding new path:[%s]\n", newpath);
			pathlist = addNode(pathlist, newpath);
			printList(pathlist);
			return 0;
		}
		else if(args[0]=='-')
		{
			//remove from linkedlist
			printf("\nDeleting path:[%s]\n", newpath);
			pathlist = deleteNode(pathlist, newpath);
			printList(pathlist);
			return 0;
		}
		else
		{
			free(args);
			free(newpath);
			return 22;
		}
		free(newpath);
		free(args);
	
	}
	return 12;
}

void executeCommand(char **argv)
{
		int child_pid = fork();
		//int child_status;

		if(child_pid < 0)
		{
			//errno = 3;
			perror("fork error:");
		}
		else if(child_pid == 0)		//fork success
		{
			if(execv(argv[0], argv) == -1)
			{
				perror("Bad command or filename:");
			}
			
		}
		// else	//fork success, parent
		// {
		// 	child_pid = waitpid(child_pid, &child_status, 0);
		// 	if(child_pid < 0)
		// 	{
		// 		perror("waitpid error:");
		// 	}
		// }
}

char ** parseArgv(char * consoleinput, const char *delim)
{
	/*parse into an array of pointers that point to null terminated strings
		-first pointer points to command
		-followed by pointers to other arguments
		-last element points to NULL			
	*/
	char ** argv = (char **)malloc( (strlen(consoleinput)) * sizeof(char*) );	//we can never have more char *'s than length of input
	assert(argv!=NULL);
	//memset(argv, 0, sizeof(char*)*strlen(consoleinput));

	char* curToken = strtok(consoleinput, delim);
	int argcount = 1;
	//printf("\nDelimiter: [%s]", delim);
	for (argcount = 1; curToken != NULL; argcount++)
	{
  		//printf("\nToken picked:%s", curToken);
  		argv[argcount-1] = strdup(curToken);
  		removeWhiteSpaces(argv[argcount-1]);
  		curToken = strtok(NULL, delim);
		//printf("\nargv[%d] : [%s]", argcount-1, argv[argcount-1]);
	}

	argv[argcount-1] = NULL;		//argcount is arguments given + 1
	//printf("\nNULL at argv[%d] : [%s]", argcount-1, argv[argcount]);

	argv = (char**) realloc(argv, (argcount) * sizeof(char*));
	
	free(curToken);

	return argv;
}

int findPath(char **argv)
{
	char *cmdfile = argv[0];
	
	struct stat st;
	if(stat(cmdfile,&st) == 0)	//command file found in current dir
    {
    	return 0;
    }
    else						//search pathlist for command file
    {
    	struct node *pathnode = pathlist;
    	char *filename = argv[0];

    	while(pathnode!=NULL)	//traverse pathlist
    	{
    		char *filepath = strdup(pathnode->elem);
    		if(filepath[strlen(filepath)-1] != '/')
    		{
    			filepath = realloc(filepath, strlen(filepath)+strlen(filename)+2);
    			strcat(filepath, "/");	
    		}
    		else
    		{
    			filepath = realloc(filepath, strlen(filepath)+strlen(filename)+1);
    		}
    		strcat(filepath, filename);	//filepath represents full path to command file now

    		//printf("\nFile Path being evaluated:[%s]", filepath);
    		if(stat(filepath,&st) == 0)	//command file found at current path
   			{
    			argv[0] = realloc(argv[0], strlen(filepath)+1);
    			argv[0] = strdup(filepath);
    			//printf("\nFile found at path:[%s]", argv[0]);
    			free(filepath);
    			return 0;
    		}
    		free(filepath);
    		return 0;
    	}

    }
    return 1;
}

int main(int argc, char **argv)
{
	while(1)	//shell runs till user enters exit
	{
		char * cwd = malloc(256);	//find current working directory for prompt
		assert(cwd != NULL);
		getCurrentDir(cwd);
		printf("\nMyShell:%s$ ", cwd);		//print shell prompt
		free(cwd);

		char *consoleinput = malloc(256);
		assert(consoleinput != NULL);
		//TO DO: Change buffer size to variable

		readCommand(consoleinput);				//accept input
		removeNewLineChar(consoleinput);		//remove '\n' character from input
		removeWhiteSpaces(consoleinput);		//remove all spaces before and after command

		if(strncmp(consoleinput, SUPPORTED_COMMANDS[1], 4) == 0)		//exit
		{
			free(consoleinput);
			printf("\nMyShell Terminated\n");
			return 0;
		}
		else if(strncmp(consoleinput, SUPPORTED_COMMANDS[0], 2) == 0)	//cd
		{
			int result = runChangeDir(consoleinput);
			if(result!=0)
			{
				perror("cd error:");
			}
		}
		else if(strncmp(consoleinput, SUPPORTED_COMMANDS[2], 4) == 0)	//path
		{
			int result = runPath(consoleinput);
			if(result!=0)
			{
				errno = result;
				perror("path error:");
			}
		}
		else
		{
			//handle pipes
			char **commands = parseArgv(consoleinput, SPECIAL_CHARS[4]);	//input destoyed

			int numcommands = 0;

			while( commands[numcommands]!=NULL )
			{
				numcommands++;
			}
			//printf("\nNumber of commands:[%d]", numcommands);

			const int  numpipes = 2*(numcommands-1);
			//printf("\nNumber of pipe file descriptors:[%d]", numpipes);

			/*read and write ends of pipes stay apart by 3
				-increment open pipe indexes by 2 after every command
				-close all pipes
			*/
			
			int pipefds[numpipes];
			
			int i=0;
			for(i=0; i<numpipes;i=i+2)
			{
				pipe(pipefds+i);
			}

			//printf("\npipe() call successful");
			// for(i=0;i<numpipes;i++)
			// {
			// 	printf("[%d]", pipefds[i]);
			// }
			int pipe_w = 1;
			int pipe_r = pipe_w - 3;
			int curcommand = 0;
			
			while(curcommand < numcommands)
			{	
				//printf("\nCommand number:[%d]", curcommand);
				//printf("\ninside pipe loop for command [%s]", commands[curcommand]);

				//Parse Command and Arguments into formatneeded by execv
				char **argv = parseArgv(commands[curcommand], SPECIAL_CHARS[0]);

				//printf("\nCurrent Command:[%s]", argv[0]);
				if(findPath(argv) == 0)
				{
					//executeCommand(argv);
					int child_pid = fork();
					//int child_status;

					if(child_pid < 0)
					{
						//errno = 3;
						perror("fork error:");
					}
					else if(child_pid == 0)		//fork success
					{
						if(pipe_w < numpipes)
						{
							//open write end
							//printf("\nWrite pipe:[%d] to stdout", pipefds[pipe_w]);
							if(dup2(pipefds[pipe_w], 1) < 0)
							{
								perror("pipe write-end error: ");
							}
						}

						if((pipe_r >= 0)&&(pipe_r < numpipes))
						{
							//open read end
							//printf("\nRead pipe:[%d] to stdin", pipefds[pipe_r]);
							if(dup2(pipefds[pipe_r], 0) < 0)
							{
								perror("pipe read-end error: ");
							}
						}

						for(i=0;i<numpipes;i++)	//close off all pipes
						{
							//printf("\nclosing all pipes");
							close(pipefds[i]);
						}

						if(execv(argv[0], argv) == -1)
						{
							perror("Bad command or filename:");
							exit(0);
							//TODO: child hangs here
						}
						//fflush(stdin);
					}		
				}
				else
				{
					printf("\nBad command or filename");
					//TODO: ForkBomb occuring here
					//exit(0);
				}
				free(argv);

				//printf("\nIncrementing pipe ends, moving to next command.");
				curcommand++;
				pipe_w = pipe_w + 2;
				pipe_r = pipe_r + 2;
			}

			//int i=0;
			for(i=0;i<numpipes;i++)	//close off all pipes
			{
				//printf("\nclosing all pipes");
				close(pipefds[i]);
			}

			int status;
			for(i=0;i<numcommands;i++)
			{
				wait(&status);
			}

			free(commands);
		}

		free(consoleinput);
	}
	freeList(pathlist);
}