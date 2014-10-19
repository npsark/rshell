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

char** tokenize(string input, const char *delim);
int parseForArgs(string input);
void parseForLogic(string input);
void parseForCommands(string input);
void runCommand(char **argv);

int main(){

	string input;
	do{
		//display prompt
		cout << "$ ";

		//get user input and store as string and cstring
		getline(cin, input);
		


		//if user inputs "exit" quit the shell
		if(input == "exit"){
			exit(1);
		}	

		parseForCommands(input);

		

	}while(input != "exit");
	
	return 0;

}

char** tokenize(string input, const char *delim){
	
	char *input_cstr = new char[input.length()+1];
		strcpy(input_cstr, input.c_str());

	vector<char*> argvVect;	
	argvVect.clear();

	char *token = strtok(input_cstr, delim);
	
	

	while(token != NULL){
		argvVect.push_back(token);
		token = strtok(NULL, delim);
	}

	uint tokCount = argvVect.size();
	char **tokArr = new char*[tokCount+1];
	tokArr[tokCount] = 0;

	

	for(uint i=0; i<tokCount; i++){

		int len = strlen(argvVect[i]) + 1;
		tokArr[i] = new char[len];
		strcpy(tokArr[i], argvVect[i]);

	}
	
	return tokArr;


}


int parseForArgs(string input){
	
	
	//get tokens from input in char* array
	char **toks = tokenize(input, " ");

	
	string which = "[ -e /bin/";
	which += string(toks[0]) + " ] || [ -e /usr/bin/" + string(toks[0]) + " ] || [ -e " + string(toks[0]) + " ]";

	bool success = !(  system(  which.c_str()) == 256 );

	//run the command
	if(success){
		runCommand(toks);
	}else{
		cout << "command not found." << endl;
		return 1;
	}
	//clear the toks array
	uint i=0;
	while(toks[i]){
		delete toks[i];
		i++;
	}

	//delete toks
	delete toks;

	return 0;

}


void parseForLogic(string input){

	uint i=0;

	char **toks = tokenize(input, "&|");
	i=0;

	bool andStatement = (input.find("&&") != string::npos);
	bool orStatement = (input.find("||") != string::npos);

	while(toks[i]){
		//parse each bit between | and &
		int success = parseForArgs(toks[i]);


		if( andStatement ){
			if(success == 1){
				return;
			}
		}else if( orStatement ){
			if(success == 0){
				return;
			}
		}

		i++;
	}

	


	i=0;
	while(toks[i]){
		//parse each bit between semicolons
		delete toks[i];	
		i++;
	}
	delete toks;

}

//split up input by semicolons
void parseForCommands(string input){

	char **toks = tokenize(input, ";");
	uint i=0;
	while(toks[i]){
		//parse each bit between semicolons
		parseForLogic(toks[i]);	
		i++;
	}
	
	i=0;
	while(toks[i]){
		//parse each bit between semicolons
		delete toks[i];	
		i++;
	}
	delete toks;
	
}

void runCommand(char **argv){

	int pid = fork();
	if(pid == -1){

		perror("fork() had an error.\n");
		exit(1);

	}else if(pid == 0){//child

		if(execvp(argv[0], argv) == -1){
			perror("execvp() had an error.\n");
		}
		
		exit(1);

	}else if(pid > 0){//parent

		if(wait(0) == -1){
			perror("wait() had an error.\n");
		}
 
	}


}


