#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

using namespace std;

int main() {
	int input[2],output[2];
	pipe(input);	//pipe for a childs input
	pipe(output);	//pipe for a childs ouput
	cout << "\n	A.      Run an internal command\n";
	cout << "	B.      Run an external command\n";
	cout << "	C.      Run an external command by redirecting standard input from a file\n";
	cout << "	D.      Run an external command by redirecting standard output to a file\n";
	cout << "	E.      Run an external command in the background\n";
	cout << "	F.      Run several external commands in the pipe mode\n";
	cout << "	G.      Quit the shell\n\n";
	while(1) {
		printf("\nEnter the option - ");
		char c;
		scanf("%c",&c);
		printf("\n");
		switch(c) {
			case 'A': {
						break;					
					}
			case 'E':	//option E and B work the same except the waiting in the parent call
			case 'B': {
					string command;
					getline(cin,command);
					char *parsed[5];
					for(int i=0;i<5;i++) parsed[i] = NULL;
					char *input = new char[command.size()+1];
					strcpy(input,command.c_str());
					int count = 0;
					char * token=strtok(input, " ");
					while (token!=NULL) {
						parsed[count] = new char[strlen(token) + 1];
						strcpy(parsed[count++],token);
						token=strtok(NULL, " ");
					}
					int y;
					delete input;
					delete token;
					execlp(parsed[0],parsed[0],parsed[1],parsed[2],parsed[3],parsed[4],(char *)NULL);
					exit(0);		//if execlp fails					
					}
			case 'C': {	
					string command;
					getline(cin,command);
					char *par1,*file1;
					char *input = new char[command.size()+1];
					strcpy(input,command.c_str());

					//the part of parsing the input string into command and input file
					FILE * fp = fopen(file1,'r');
					close(stdin);
					dup(fp);
					//the execlp call to execute the command
					exit(0);		//if execlp fails					
					}
			case 'D': {	
					string command;
					getline(cin,command);
					char *par1,*file1;
					char *input = new char[command.size()+1];
					strcpy(input,command.c_str());

					//the part of parsing the input string into command and output file
					FILE * fp = fopen(file1,'w');
					close(stdout);
					dup(fp);
					//the execlp call to execute the command
					exit(0);		//if execlp fails					
					}
			case 'F': {
					string command;
					getline(cin,command);
					int N;
					//the part of parsing the input string into N external commands with in between	 					
					int pip1[2],pip2[2];
					pipe(pip1);	//in place for stdin
					pipe(pip2);	//in place for stdout
					char ** commands[N];
					for(int i=0;i<N;i++) {
						int x = fork();
						if(x==0) { //in the child process
							if(i==0) {
								close(stdout);
								dup(pip2[1]);				//pip2 as output line
							}
							if(i==n-1) {
								close(stdin);
								if(n%2) dup(pip1[0]);		//pip1 as input line
								else dup(pip2[0]);			//pip2 as input line
							}
							if(i%2==0) {
								close(stdout);dup(pip2[1]);	//pip2 as output line
								close(stdin);dup(pip1[0]);	//pip1 as input line
							} else {
								close(stdout);dup(pip1[1]);	//pip1 as output line
								close(stdin);dup(pip2[0]);	//pip2 as input line
							}
							//execlp statement using commands[i] as the argument
							exit(0);		//if execlp fails
						} else {	//in the parent process
							wait(NULL);		//wait for the child process to exit before going to create another process
						}
					}
					exit(0);					
					}
			case 'G': return 0;
			case default: continue;	//try again for the correct letter
		}
		if(c!='E') wait(NULL);	//if not the background command then wait for the child processes to exit
	}
	return 0;
}
