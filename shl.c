// C Program to design a shell in Linux 
#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include <fcntl.h>
/*#include<readline/readline.h> 
#include<readline/history.h> */

#define MAXCOM 1000 // max number of letters to be supported 
#define MAXLIST 100 // max number of commands to be supported 

// Clearing the shell using escape sequences 
#define clear()  
char history[MAXLIST]; 


// Greeting shell during startup 
void init_shell() 
{ 
	printf("\033[H\033[J");
	printf("\n\n\n\t****MY SHELL****"); 
	printf("\n\n\n\t****WELCOME****");
	sleep(1); 
	printf("\033[H\033[J"); 
} 

/*void historyLog()
{
    int index = 0;
    int histIndex=1;

    while ( index < MAXLIST && history[index] != NULL ) ++index;
    printf("%s\n", history[index-2] );

    if(index > 15){
    	for(int i = index-16; i < index-1; i++)
    		printf("%d- %s\n", histIndex++ , history[i] );
    }
    else
    {
    	for(int i = 0; i < index-1; i++)
    		printf("%d- %s\n", i , history[i] );
    }

}*/

// Function to take input 
int takeInput(char* str) 
{ 
	int len;
	char buf[MAXLIST]; 
	char* username = getenv("USER");
	printf("\n%s >>> ", username);  
	fgets(buf, MAXLIST, stdin);
	len = strlen(buf);
    buf[len-1]='\0';
	//printf("1%s1\n",buf ); 
	//buf = readline(" >>> "); 
	if (strlen(buf) != 0) { 
		//add_history(buf); 
		strcpy(str, buf); 
		return 0; 
	} else { 
		return 1; 
	} 
} 

// Function to print Current Directory. 
void currentPath() 
{ 
	char cwd[1024]; 
	getcwd(cwd, sizeof(cwd)); 
	printf("%s\n", cwd); 
} 

// Function where the system command is executed 
void execArgs(char** parsed) 
{ 
	// Forking a child 
	pid_t pid = fork(); 

	if (pid == -1) { 
		printf("\nFailed forking child.."); 
		return; 
	} else if (pid == 0) { 
		if (execvp(parsed[0], parsed) < 0) { 
			printf("\nCould not execute command.."); 
		} 
		exit(0); 
	} else { 
		// waiting for child to terminate 
		wait(NULL); 
		return; 
	} 
} 

// Function where the piped system commands is executed 
void execArgsPiped(char** parsed, char** parsedpipe) 
{ 
	// 0 is read end, 1 is write end 
	int pipefd[2]; 
	pid_t p1, p2; 

	if (pipe(pipefd) < 0) { 
		printf("\nPipe could not be initialized"); 
		return; 
	} 
	p1 = fork(); 
	if (p1 < 0) { 
		printf("\nCould not fork"); 
		return; 
	} 

	//printf("%d\n", p1);

	if (p1 == 0) { 
		//printf("%davc\n", p1);
		// Child 1 executing.. 
		// It only needs to write at the write end 
		close(pipefd[0]); 
		dup2(pipefd[1], STDOUT_FILENO); 
		close(pipefd[1]); 

		if (execvp(parsed[0], parsed) < 0) { 
			printf("\nCould not execute command 1.."); 
			exit(0); 
		} 
	} else { 
		// Parent executing 
		p2 = fork(); 

		if (p2 < 0) { 
			printf("\nCould not fork"); 
			return; 
		}

		//printf("%s\n",parsedpipe[1] );

		

		// Child 2 executing.. 
		// It only needs to read at the read end 
		if (p2 == 0) { 
			close(pipefd[1]); 
			dup2(pipefd[0], STDIN_FILENO); 
			close(pipefd[0]); 
			if (execvp(parsedpipe[0], parsedpipe) < 0) { 
				printf("\nCould not execute command 2.."); 
				exit(0); 
			} 
		} else { 
			// parent executing, waiting for two children 
			wait(NULL); 
		//	wait(NULL); 
		} 
	} 
}
// Function to execute (parsed) > (parsedpipe) command
void execRedirectedArgs(char** parsed, char** parsedpipe){

	char *argv[] = {"cat", parsed[1] , 0};
	int kidpid;
	int fd = open(parsedpipe[0], O_WRONLY|O_TRUNC|O_CREAT, 0644);
	if (fd < 0) { perror("open"); abort(); }
	switch (kidpid = fork()) {
	  case -1: perror("fork"); abort();
	  case 0:
	    if (dup2(fd, 1) < 0) { perror("dup2"); abort(); }
	    close(fd);
	    execvp(argv[0], argv); perror("execvp"); abort();
	  default:
	    close(fd);
	/* do whatever the parent wants to do. */
	}

} 

void redirectOutput()
{

}

// Function to execute builtin own commands 
int ownCmdHandler(char** parsed) 
{ 
	int NoOfOwnCmds = 6, i, switchOwnArg = 0; 
	char* ListOfOwnCmds[NoOfOwnCmds]; 
	char* username; 

	ListOfOwnCmds[0] = "exit"; 
	ListOfOwnCmds[1] = "cd"; 
	ListOfOwnCmds[2] = "listdir"; 
	ListOfOwnCmds[3] = "currentpath";
	ListOfOwnCmds[4] = "printfile";
	ListOfOwnCmds[5] = "footprint"; 
 

	for (i = 0; i < NoOfOwnCmds; i++) { 
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) { 
			switchOwnArg = i + 1; 
			break; 
		} 
	} 

	switch (switchOwnArg) { 
	case 1: 
		printf("\nGoodbye\n"); 
		exit(0); 
	case 2: 
		chdir(parsed[1]); 
		return 1; 
	case 3: 
		parsed[0]="ls";
		return 0; 
	case 4: 
		currentPath(); 
		return 1;
	 case 5:
		parsed[0]="cat";
		return 0;	
	 case 6:
		//historyLog();
		return 1;

	default: 
		break; 
	} 

	return 0; 
} 

// function for finding pipe 
int parsePipe(char* str, char** strpiped) 
{ 
	int i; 
	for (i = 0; i < 2; i++) { 
		strpiped[i] = strsep(&str, "|"); 
		if (strpiped[i] == NULL) 
			break; 
	} 

	if (strpiped[1] == NULL) 
		return 0; // returns zero if no pipe is found. 
	else { 
		return 1; 
	} 
} 
// function for finding redirection (>) 
int parseRedirection(char* str, char** strpiped) 
{ 
	int i; 
	for (i = 0; i < 2; i++) { 
		strpiped[i] = strsep(&str, ">"); 
		if (strpiped[i] == NULL) 
			break; 
	} 

	if (strpiped[1] == NULL) 
		return 0; // returns zero if no pipe is found. 
	else { 
		return 1; 
	} 
}

// function for parsing command words 
void parseSpace(char* str, char** parsed) 
{ 
	int i; 

	for (i = 0; i < MAXLIST; i++) { 
		parsed[i] = strsep(&str, " "); 

		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
}

// function for parsing " char in grep 
void parseQuote(char* str, char** parsed) 
{ 
	int i; 

	for (i = 0; i < MAXLIST; i++) { 
		parsed[i] = strsep(&str, "\""); 

		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
}  

// function for processing input what to do
int processString(char* str, char** parsed, char** parsedpipe) 
{ 

	char* strpiped[2];
	char* parsedpipe1[2]; 
	char* strredirected[2];
	int piped = 0;
	int redirected = 0; 

	piped = parsePipe(str, strpiped);
	redirected = parseRedirection(str,strredirected);

	if (piped) { 
		parseSpace(strpiped[0], parsed); 
		parseQuote(strpiped[1], parsedpipe);
		parseSpace(parsedpipe[0], parsedpipe1); 
		strcpy(parsedpipe[0], parsedpipe1[0]);

	}else if(redirected){ 

		parseSpace(strredirected[0], parsed); 
		parseSpace(strredirected[1], parsedpipe);
		execRedirectedArgs(parsed,parsedpipe);
		return 0;
	} 
	 else{ 

		parseSpace(str, parsed); 
	} 

	if (ownCmdHandler(parsed)) 
		return 0; 
	else
		return 1 + piped; 
} 



int main() 
{ 
	char inputString[MAXCOM], *parsedArgs[MAXLIST]; 
	char* parsedArgsPiped[MAXLIST]; 
	int execFlag = 0;
	int count = 0; 
	init_shell();
	//char* history[MAXLIST]; 

	while (1) { 
		// print shell line 
		//printDir(); 
		// take input
		//printf("whilw\n" );

		//int i;
		if (takeInput(inputString)) 
			continue; 
		// process

		/*history[count] = inputString;
		//history[count+1] = NULL;
		printf("%s\n", history[count] );
		count++;*/

		
		execFlag = processString(inputString, 
		parsedArgs, parsedArgsPiped); 
		// execflag returns zero if there is no command 

		// execute 
		if (execFlag == 1) 
			execArgs(parsedArgs); 

		if (execFlag == 2){
			execArgsPiped(parsedArgs, parsedArgsPiped); 
		}
				

	} 
	return 0; 
} 
