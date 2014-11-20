#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
using namespace std;



char** tokenize(string input, const char *delim);
int parseForArgs(string input, int io = -1, string fileName = "");

int parseForRedirection(string input, bool fromPipe = false);
int parseForPipes(string input);
int parseForOR(string input);
void parseForAND(string input);
void parseForCommands(string input);

int runCommand(char **argv, int io = -1, string fileName = "");
int myExec(char **argv, int io, string fileName);

string removeEdgeSpaces(string inString);
void runPipe(string src, string dest){
	

	int fd[2];
	pipe(fd);


	int pid = fork();


	if(pid == 0){
		//write to the pipe
		if(-1 == dup2(fd[1],1)){//make stdout the write end of the pipe 
		      perror("dup2()");
		}

		if(-1 == close(fd[0])){//close the read end of the pipe because we're not doing anything with it right now
		      perror("close()");
		}

		//char **toks = tokenize(src, " ");
		//execvp(toks[0], toks);
		parseForRedirection(src);
		
		exit(1);
	}else{
		wait(NULL);

		//read end of the pipe
		int savestdin;
		if(-1 == (savestdin = dup(0))){//need to restore later or infinite loop
		      perror("dup()");
		}
		if(-1 == dup2(fd[0],0)){//make stdin the read end of the pipe 
		      perror("dup2()");
		}
		if(-1 == close(fd[1])){//close the write end of the pipe because we're not doing anything with it right now
		      perror("close()");
		}

		parseForPipes(removeEdgeSpaces(dest));

		dup2(savestdin,0);

	}

	//parseForRedirection(src);
	//parseForPipes(dest);


}



int main(){




	char *hostname = new char[50];
	if(gethostname(hostname, 50) == -1){
		perror("gethostname() failed");
	}
	char *login = getlogin();
	if(login == NULL){
		perror("getlogin() failed");
	}
	

	string input;
	do{
		//display prompt
		cout << login << "@" << hostname <<"$ ";

		//get user input and store as string and cstring
		getline(cin, input);
		
		if(input.find("#") != string::npos){
			input.resize(input.find("#"));
		}

		//if user inputs "exit" quit the shell
		if(input == "exit"){
			exit(1);
		}	

		parseForCommands(input);

		

	}while(input != "exit");
	
	return 0;

}


string removeEdgeSpaces(string inString){

	string inStr = inString;
	while(inStr[0] == ' '){
		inStr.erase(inStr.begin());
	}

	while( *(inStr.end()-1) == ' '){
		inStr.erase( inStr.end()-1 );
	}

	return inStr;
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


int parseForArgs(string input, int io, string fileName){
	
	
	//get tokens from input in char* array
	char **toks = tokenize(input, " ");

	int prg = runCommand(toks, io, removeEdgeSpaces(fileName));	

	//clear the toks array
	uint i=0;
	while(toks[i]){
		delete toks[i];
		i++;
	}

	//delete toks
	delete toks;

	
	

	return prg;

}





int parseForRedirection(string input, bool fromPipe){

	uint i=0;

	char **toks = tokenize(input, "<>");
	while(toks[i]){
		i++;
	}
	

	bool inputRedir = (input.find("<") != string::npos);


	int success;



	if(i == 2){		

		//toks[0] will be the command with args
		//toks[1] will be the file from which the input will come
	
		
		//parse each bit between pipes(|).
		uint redirPos;
		if(inputRedir){
			
			string cmd = toks[0];
			string file = "";

			redirPos = input.find("<");

			uint nextNotSpc = input.length()-1;

			for(uint k=redirPos + 1; k<input.length(); k++){
				if(input[k] != ' '){
					nextNotSpc = k;
					break;
				}
			}

			uint nextSpc = input.find(" ", nextNotSpc);
			if( nextSpc < input.length() ){
				file = input.substr(nextNotSpc, nextSpc-nextNotSpc);
			
				if( nextSpc != string::npos ){
					cmd += input.substr( nextSpc, (input.length() - nextSpc) );
				}

				if(fromPipe){
					char **cmdToks = tokenize(cmd, " ");
					success = myExec(cmdToks, 0, file);
				}else{
					success = parseForArgs(cmd, 0, file);
				}
			}else{
				if(fromPipe){
					success = myExec(toks, 0, string(toks[1]));
				}else{
					success = parseForArgs(toks[0], 0, string(toks[1]));
				}
			}
			

		}else{
			bool append = (input.find(">>") != string::npos);
			if( append ){
				redirPos = input.find(">>");


				string cmd = toks[0];
				string file = "";

				uint nextNotSpc = input.length()-1;

				for(uint k=redirPos + 2; k<input.length(); k++){
					if(input[k] != ' '){
						nextNotSpc = k;
						break;
					}
				}

				uint nextSpc = input.find(" ", nextNotSpc);
				if( nextSpc < input.length() ){
					file = input.substr(nextNotSpc, nextSpc-nextNotSpc);
				
					if( nextSpc != string::npos ){
						cmd += input.substr( nextSpc, (input.length() - nextSpc) );
					}

					success = parseForArgs(cmd, 2, file);
				}else{
					success = parseForArgs(toks[0], 2, string(toks[1]));
				}



			}else{
				redirPos = input.find(">");


				string cmd = toks[0];
				string file = "";

				uint nextNotSpc = input.length()-1;

				for(uint k=redirPos + 2; k<input.length(); k++){
					if(input[k] != ' '){
						nextNotSpc = k;
						break;
					}
				}

				uint nextSpc = input.find(" ", nextNotSpc);
				if( nextSpc < input.length() ){
					file = input.substr(nextNotSpc, nextSpc-nextNotSpc);
				
					if( nextSpc != string::npos ){
						cmd += input.substr( nextSpc, (input.length() - nextSpc) );
					}

					success = parseForArgs(cmd, 1, file);
				}else{
					success = parseForArgs(toks[0], 1, string(toks[1]));
				}
			}
		}


	}else{
		success = parseForArgs(toks[0]);
	}

	i=0;
	while(toks[i]){
		delete toks[i];	
		i++;
	}
	delete toks;
	return success;

}







int parseForPipes(string input){

	uint i=0;

	char **toks = tokenize(input, "|");
	while(toks[i]){
		//cout << "toks" << "[" << i << "]: " << toks[i] << endl;
		i++;
	}

	int success;

	//cout << "SIZE: " << i << endl;
	if(i >= 2){
		
		int res = input.find("|");

		runPipe( string(toks[0]), input.substr(res+1)); 
	}else{
		i=0;
		
		while(toks[i]){

			success = parseForRedirection(toks[i]);
		
			i++;
		}


	}

	//clean up
	i=0;
	while(toks[i]){
		delete toks[i];	
		i++;
	}
	delete toks;
	return success;

}







int parseForOR(string input){

	int success;

	bool orStatement = (input.find("||") != string::npos);
	
	int orLoc = 0;
	bool a = false;
	if(orStatement){

		vector<string> toksV;
		while(  !a   ){
			int old = orLoc ;
			a = (orLoc = input.find("||", orLoc + 1)) == -1;
			if(old == 0){
				toksV.push_back( removeEdgeSpaces(input.substr(old, orLoc - old)) );
			}else{
				toksV.push_back( removeEdgeSpaces(input.substr(old + 2, orLoc - old - 2)) );
			}

		}

		for(uint l=0; l<toksV.size(); l++){
			success = parseForOR(toksV[l]);

			if( orStatement ){
				if(success == 0){
					return 0;
				}
			}
		}

	
	}else{

		success = parseForPipes(input);		

	}
	
	return success;

}





void parseForAND(string input){

	uint i=0;

	char **toks = tokenize(input, "&");
	i=0;

	//bool andStatement = (input.find("&&") != string::npos);
	bool andStatement = (input.find("&&") != string::npos);


	while(toks[i]){
		//cout << "AND: " << toks[i] << endl;

		//parse each bit between | and &
		//int success = parseForArgs(toks[i]);
		int success = parseForOR(toks[i]);
		
	
		if( andStatement ){
			if(success != 0){
				return;
			}
		}

		i++;
	}
	

	//clean up
	i=0;
	while(toks[i]){
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
		parseForAND(toks[i]);	
		i++;
	}
	

	//clean up
	i=0;
	while(toks[i]){
		delete toks[i];	
		i++;
	}
	delete toks;
	
}


int runCommand(char **argv, int io, string fileName){

	int status = 0;

	int pid = fork();
	if(pid == -1){

		perror("fork()");
		exit(1);

	}else if(pid == 0){//child

		myExec(argv, io, fileName);
		exit(1);

	}else if(pid > 0){//parent
		
		if(wait(&status) == -1){
			perror("wait()");
		}

	}

	
	return status;

}



int myExec(char **argv, int io, string fileName){


	if (io == 0){//input redirection <
		int fd0 = open(removeEdgeSpaces(fileName).c_str(), O_RDONLY, 0);
		if(fd0 == -1){
			perror("open");
		}else{
			if(dup2(fd0, STDIN_FILENO) == -1){
				perror("dup2");
			}
			if(close(fd0) == -1){
				perror("close");
			}
		}
	}else if (io == 1){//output redirection >
		int fd1 = open(removeEdgeSpaces(fileName).c_str(), O_RDWR | O_CREAT, 0644);
		if(fd1 == -1){	
			perror("open");
		}else{
			if(dup2(fd1, STDOUT_FILENO) == -1){
				perror("dup2");
			}
			if(close(fd1) == -1){
				perror("close");
			}
		}
	}else if (io == 2){//output redirection >>
		int fd1 = open(removeEdgeSpaces(fileName).c_str(), O_RDWR | O_CREAT | O_APPEND, 0644);
		if(fd1 == -1){	
			perror("open");
		}else{
			if(dup2(fd1, STDOUT_FILENO) == -1){
				perror("dup2");
			}
			if(close(fd1) == -1){
				perror("close");
			}
		}
	}



	if(execvp(argv[0], argv) == -1){
		perror("execvp()");
		return -1;
	}
	
	return 0;


}

