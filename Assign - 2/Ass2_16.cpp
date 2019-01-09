#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>

using namespace std;

int main() {
	cout << "\n	A.      Run an internal command\n";
	cout << "	B.      Run an external command\n";
	cout << "	C.      Run an external command by redirecting standard input from a file\n";
	cout << "	D.      Run an external command by redirecting standard output to a file\n";
	cout << "	E.      Run an external command in the background\n";
	cout << "	F.      Run several external commands in the pipe mode\n";
	cout << "	G.      Quit the shell\n";
	while(1) {
		printf("\nEnter the option - ");
		char c;
		c = getchar();getchar();
		switch(c) {
			case 'A': {
						string command;
						getline(cin,command);
						system(command.c_str());
						continue;					
					}
			case 'E': {
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
					int x = fork();
					if(x) break;
					delete input;
					delete token;
					execlp(parsed[0],parsed[0],parsed[1],parsed[2],parsed[3],parsed[4],(char *)NULL);
					exit(0);		//if execlp fails					
					}
			case 'B': {
					int x = fork();
					if(x) break;
					string command;
					cin >> command;
					//getline(cin,command);
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
					int x = fork();
					if(x) break;
					string command;
					getline(cin,command);
					char *par1;
					char *input = new char[command.size()+1];
					strcpy(input,command.c_str());
					char *token = strtok(input,"<");
					par1 = new char[strlen(token) + 1];
					strcpy(par1,token);
					token = strtok(NULL,"<");
					token = strtok(token," ");
					char *parsed[5];
					for(int i=0;i<5;i++) parsed[i] = NULL;
					input=strtok(par1, " ");
					int count = 0;
					while (input!=NULL) {
						parsed[count] = new char[strlen(input) + 1];
						strcpy(parsed[count++],input);
						input=strtok(NULL, " ");
					}
					//the part of parsing the input string into command and input file
					int fp = open(token, O_RDONLY);
					fclose(stdin);
					dup(fp);
					execlp(parsed[0],parsed[0],parsed[1],parsed[2],parsed[3],parsed[4],(char *)NULL);
					//the execlp call to execute the command
					exit(0);		//if execlp fails					
					}
			case 'D': {	
					int x = fork();
					if(x) break;
					string command;
					getline(cin,command);
					char *par1;
					char *input = new char[command.size()+1];
					strcpy(input,command.c_str());
					char *token = strtok(input,">");
					par1 = new char[strlen(token) + 1];
					strcpy(par1,token);
					token = strtok(NULL,">");
					token = strtok(token," ");
					char *parsed[5];
					for(int i=0;i<5;i++) parsed[i] = NULL;
					input=strtok(par1, " ");
					int count = 0;
					while (input!=NULL) {
						parsed[count] = new char[strlen(input) + 1];
						strcpy(parsed[count++],input);
						input=strtok(NULL, " ");
					}
					//the part of parsing the input string into command and input file
					int fp = open(token, O_WRONLY);
					fclose(stdout);
					dup(fp);
					execlp(parsed[0],parsed[0],parsed[1],parsed[2],parsed[3],parsed[4],(char *)NULL);
					//the execlp call to execute the command
					exit(0);		//if execlp fails					
					}
			case 'F': {
					string command;
					getline(cin,command);
					int n = 1;
					char *input = new char[command.size()+1];
					strcpy(input,command.c_str());
					for(int i=0;i<command.size();i++) if(input[i]=='|') n++;
					char *commands[n];
					int count = 0;
					char *token = strtok(input,"|");
					while(token!=NULL) {
						commands[count++] = token;
						token = strtok(NULL,"|");
					}
					//for(int i=0;i<n;i++) printf("%s\n",commands[i]);
					delete input;
					//the part of parsing the input string into N external commands with in between	 					
					int pip1[2],pip2[2];
					char *parsed[5];
					for(int i=0;i<5;i++) parsed[i] = NULL;
					pipe(pip1);	//in place for 0
					pipe(pip2);	//in place for 1
					for(int i=0;i<n;i++) {
						int x = fork();
						if(x==0) { //in the child process
							if(i==0) {
								fclose(stdout);
								dup(pip2[1]);				//pip2 as output line
							}
							else if(i==n-1) {
								fclose(stdin);
								if(n%2) dup(pip1[0]);		//pip1 as input line
								else dup(pip2[0]);			//pip2 as input line
							}
							else if(i%2==0) {
								fclose(stdout);dup(pip2[1]);	//pip2 as output line
								fclose(stdin);dup(pip1[0]);	//pip1 as input line
							} else {
								fclose(stdout);dup(pip1[1]);	//pip1 as output line
								fclose(stdin);dup(pip2[0]);	//pip2 as input line
							}

							input = commands[i];
							token = strtok(input," ");
							int count = 0;
							while(token!=NULL) {
								parsed[count++] = token;
								token = strtok(NULL," ");
							}
							for(int i=0;i<5;i++) printf("%s\n",parsed[i]);
							//execlp statement using commands[i] as the argument
							execlp(parsed[0],parsed[0],parsed[1],parsed[2],parsed[3],parsed[4],(char *)NULL);
							exit(0);		//if execlp fails
						} else {	//in the parent process
							wait(NULL);		//wait for the child process to exit before going to create another process
						}
					}
					continue;					
					}
			case 'G': return 0;
			default: continue;	//try again for the correct letter
		}
		if(c!='E') wait(NULL);	//if not the background command then wait for the child processes to exit
	}
	return 0;
}
