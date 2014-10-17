#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

void runCommand(char **argv);

int main(){
/*
	char **argv = new char*[2];
	argv[0] = new char[3];
		strcpy(argv[0], "ls");
	argv[1] = new char[3];
		strcpy(argv[1], "-l");
	
	
	runCommand(argv);

	delete argv[0];
	delete argv[1];
	delete argv;

	argv = new char*[1];

	argv[0] = new char[6];
		strcpy(argv[0], "clear");
	
	runCommand(argv);
*/
	
	string input;
	do{
		cout << "$ ";

		getline(cin, input);
		char *input_cstr = new char[input.length()+1];
		strcpy(input_cstr, input.c_str());

		cout << "You typed " << input << endl;
		
		uint count = 0;
		char *token = strtok(input_cstr, " ");
		while(token != NULL){
			count++;
			cout << token << endl;
			token = strtok(NULL, " ");
		}

		cout << "count: " << count << endl;
	}while(input != "exit");
	
	return 0;

}

void runCommand(char **argv){
	int pid = fork();
	if(pid == -1){

		perror("fork() had an error.\n");
		exit(1);

	}else if(pid == 0){

		cout << "This is the child process.\n";
		if(execvp(argv[0], argv) == -1){
			perror("execvp() had an error.\n");
		}
		exit(1);

	}else if(pid > 0){

		if(wait(0) == -1){
			perror("wait() had an error.\n");
		}
		cout << "This is the parent process.\n";
	}


}


