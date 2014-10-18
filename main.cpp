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
	vector<char*> argvVect;

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

	char **argv;
	
	string input;
	do{
		//display prompt
		cout << "$ ";

		//get user input and store as string and cstring
		getline(cin, input);
		char *input_cstr = new char[input.length()+1];
		strcpy(input_cstr, input.c_str());


		//if user inputs "exit" quit the shell
		if(input == "exit"){
			exit(1);
		}	



		//this will count how many tokens there are
		uint count = 0;

		//clear the argv Vector
		argvVect.clear();
	
		//get first token
		char *token = strtok(input_cstr, " ");
		
		//push the first token to the vector
		argvVect.push_back(token);
		
		//this loop collects the rest of the tokens and counts them
		while(token != NULL){
			count++;
			token = strtok(NULL, " ");
			argvVect.push_back(token);
		}

		//initialize the arcv array
		argv = new char*[count+1];
		argv[count] = 0;
		//copy the contents of the vector to the argv array
		for(uint i=0; i<count; i++){
			int len = strlen(argvVect[i])+1;

			argv[i] = new char[len];
			argv[i][len-1] = 0;		
	
			strcpy(argv[i], argvVect[i]);
			cout << "argv[" << i << "] = " << argv[i] << endl << endl;
		}

		//run the command the user has requested
		runCommand(argv);

		//clear the argv array
		for(uint i=0; i<count; i++){
			delete argv[i];
		}
		//delete argv;		


	}while(input != "exit");
	
	return 0;

}

void runCommand(char **argv){
	int pid = fork();
	if(pid == -1){

		perror("fork() had an error.\n");
		exit(1);

	}else if(pid == 0){

		//cout << "This is the child process.\n";
		if(execvp(argv[0], argv) == -1){
			perror("execvp() had an error.\n");
		}
		exit(1);

	}else if(pid > 0){

		if(wait(0) == -1){
			perror("wait() had an error.\n");
		}
		//cout << "This is the parent process.\n";
	}


}


