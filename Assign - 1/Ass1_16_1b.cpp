#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

int main() {
	int finisher[2];
	pipe(finisher);	//pipe to communicate between original parent and every new child process
	while(1) {
		int x = fork();
		if(x==0) {	//new process to read and execute program
			cout << "\033[1;33mpshell(:-:)|$ \033[0m";
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
			if(strcmp(parsed[0],"quit")==0) y = 5;
			else y = 10;
			write(finisher[1],(void*)&y,sizeof(int));
			delete input;
			delete token;
			if(y==5) exit(0);	//so to exit the function after entering quit command
			execlp(parsed[0],parsed[0],parsed[1],parsed[2],parsed[3],parsed[4],(char *)NULL);
		}
		else {
			int y;
			read(finisher[0],(void*)&y,sizeof(int));
			wait(NULL);	//blocks execution of parent untill one of the child process exits
			if(y==5) break;	//when quit command is issued exit from while loop
		}
	}
	return 0;
}




