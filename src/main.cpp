#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
using namespace std;

int child=-1;

char** tokenize(string input, const char *delim);
int parseForArgs(string input, int io = -1, string fileName = "", string otherFile = "");

int parseForRedirection(string input, bool fromPipe = false);
int parseForPipes(string input);
int parseForOR(string input);
void parseForAND(string input);
void parseForCommands(string input);

int runCommand(char **argv, int io = -1, string fileName = "", string otherFile = "");
int myExec(char **argv, int io, string fileName, string otherFile = "");

string removeEdgeSpaces(string inString);
void runPipe(string src, string dest);

void childSig(int sig);
void parentSig(int sig);

int myExit(void* arg = NULL);
int cd(void* newPath);

void prompt(){
	char hostname[50];
	if(gethostname(hostname, 50) == -1){
		perror("gethostname() failed");
	}
	char *login = getlogin();
	if(login == NULL){
		perror("getlogin() failed");
	}
	char cwd[1024];
	if( getcwd(cwd, sizeof(cwd)) == NULL ){
		perror("getlogin() failed");
	}

	cin.clear();
	cout << login << "@" << hostname << ":" << cwd << "$ ";

	delete [] login;
}


int main(){



	if( signal(SIGINT, parentSig) == SIG_ERR ){
		perror("signal()");
	}

	string input;
	do{
		//display prompt
		prompt();
	
		string input;

		//get user input and store as string and cstring
		getline(cin, input);	

		if(input.find("#") != string::npos){
			input.resize(input.find("#"));
		}



		parseForCommands(input);

		

	}while(removeEdgeSpaces(input) != "exit");
	


	return 0;

}


int myExit(void* arg){
	exit(1);
	return 0;
}

int cd(void* newPath){
	string nPath = *((string*)newPath);

	if(chdir(nPath.c_str()) == -1){
		perror("chdir");
		return 1;
	}

	return 0;
}



void parentSig(int sig){
	if(sig == SIGINT){
	
		if(child != -1){
			cout << "killing " << child << endl;
			kill(child, SIGKILL);
			child = -1;
			cout << endl;
		}else{
			cout << endl;
			prompt();
			cout.flush();
		}

	}
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

	delete [] input_cstr;
	
	return tokArr;


}


int parseForArgs(string input, int io, string fileName, string otherFile){
	
	int success = 0;


	map<string, int (*)(void*)> commands;
	map<string, int (*)(void*)>::iterator it;
		commands["cd"] = cd;
		commands["exit"] = myExit;
	
	input = removeEdgeSpaces(input);
	int firstSpc = input.find(" ");
	string cmd = input.substr(0, firstSpc);	

	it = commands.find(cmd);
	if( it != commands.end() ){
		if(cmd == "exit"){
			success = commands.at(cmd)(NULL);
		}else if(cmd == "cd"){
			string nDir = input.substr(firstSpc, input.length()-firstSpc);
			nDir = removeEdgeSpaces(nDir);
			success = commands.at(cmd)(&nDir);
		}
		
	}else{		

		//get tokens from input in char* array
		char **toks = tokenize(input, " ");

		success = runCommand(toks, io, removeEdgeSpaces(fileName), otherFile);	

		//clear the toks array
		uint i=0;
		while(toks[i]){
			delete [] toks[i];
			i++;
		}

		//delete toks
		delete [] toks;

	}
	

	return success;

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


	}else if( i == 3){
		uint inDir = input.find("<");
		uint outDir = input.find(">>");
		bool append = true;
		if(outDir == string::npos){
			outDir = input.find(">");
			append = false;
		}


		if(inDir != string::npos && outDir != string::npos){

			string cmd;
			if(inDir < outDir){
				cmd = input.substr(0,inDir);
			}else{
				cmd = input.substr(0, outDir);
			}

			
			uint firstAlpha;
			for(firstAlpha=inDir+1; firstAlpha<input.length(); firstAlpha++){
				if(input[firstAlpha] != ' '){
					break;
				}
			}
			uint endInFile = input.find(" ", firstAlpha);
			string inFile = input.substr( firstAlpha, endInFile - firstAlpha);

			cout << "in: " << inFile << endl;
			
			int off = 1;
			if(append){
				off = 2;
			}
			for(firstAlpha=outDir+off; firstAlpha<input.length(); firstAlpha++){
				if(input[firstAlpha] != ' '){
					break;
				}
			}
			uint endOutFile = input.find(" ", firstAlpha);
			string outFile = input.substr( firstAlpha, endOutFile - firstAlpha);
			
			cout << "out: " << outFile << endl;


			if(inDir < outDir){
				cmd += input.substr(endInFile, outDir - endInFile);
				//cmd += " " + input.substr( endOutFile, input.length() - endOutFile-1);
			}else{
				cmd += input.substr(endOutFile, inDir - endOutFile);
				//cmd += " " + input.substr( endInFile, input.length() - endInFile-1);
			}
			

			cout << "cmd: " << cmd << endl;

			if(fromPipe){
				char **cmdToks = tokenize(cmd, " ");
				if(append){
					success = myExec(cmdToks, 4, inFile, outFile);
				}else{
					success = myExec(cmdToks, 3, inFile, outFile);
				}
			}else{
				if(append){
					success = parseForArgs(cmd, 4, inFile, outFile);
				}else{
					success = parseForArgs(cmd, 3, inFile, outFile);
				}
			}	

		}

	}else{
		success = parseForArgs(toks[0]);
	}

	i=0;
	while(toks[i]){
		delete [] toks[i];	
		i++;
	}
	delete [] toks;
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
		delete [] toks[i];	
		i++;
	}
	delete [] toks;
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
		delete [] toks[i];	
		i++;
	}
	delete [] toks;

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
		delete [] toks[i];	
		i++;
	}
	delete [] toks;
	
}







int runCommand(char **argv, int io, string fileName, string otherFile){

	int status = 0;

	int pid = fork();
	if(pid == -1){

		perror("fork()");
		exit(1);

	}else if(pid == 0){//child
		
		//setpgid(pid, 0);
		myExec(argv, io, fileName, otherFile);
		exit(1);

	}else if(pid > 0){//parent
		
		child = pid;
		if( signal(SIGINT, SIG_IGN) == SIG_ERR ){
			perror("signal()");
		}

		if(wait(&status) == -1){
			perror("wait()");
		}

		child = -1;
		if( signal(SIGINT, parentSig) == SIG_ERR ){
			perror("signal()");
		}


	}

	
	return status;

}



int myExec(char **argv, int io, string fileName, string otherFile){


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
	}else if (io == 3){//input and output redirection < infile > outfile
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

		int fd1 = open(removeEdgeSpaces(otherFile).c_str(), O_RDWR | O_CREAT, 0644);
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


	}else if (io == 4){//input and output redirection < infile > outfile
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

		int fd1 = open(removeEdgeSpaces(otherFile).c_str(), O_RDWR | O_CREAT | O_APPEND, 0644);
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

	string PATH = getenv("PATH");

	string cmd = string(argv[0]);

	char **pathToks = tokenize(PATH, ":");

	vector<string> paths;
	int i=0;
	while(pathToks[i]){
		paths.push_back(string(pathToks[i]));
		i++;
	}

	paths.push_back( "." );

	bool ran = false;

	for(uint i=0; i < paths.size(); i++){
		string path = string(paths[i]) + "/" + cmd;

		delete [] argv[0];
		argv[0] = new char[path.length() + 1];
		strcpy( argv[0], path.c_str() );

		if( execv(argv[0], argv) == -1){
			//return -1;
		}else{
			ran = true;
			break;
		}

	}
	


	if(!ran){
		perror("execv()");
		return -1;
	}
	
	return 0;


}

void runPipe(string src, string dest){
	

	int fd[2];
	if( pipe(fd) == -1){
		perror("pipe()");
		exit(1);
	}


	int pid = fork();


	if(pid == -1){
		perror("fork()");
		exit(1);
	}else if(pid == 0){

		//write to the pipe
		if(-1 == dup2(fd[1],1)){//make stdout the write end of the pipe 
		      perror("dup2()");
		}

		if(-1 == close(fd[0])){//close the read end of the pipe because we're not doing anything with it right now
		      perror("close()");
		}

		parseForRedirection(src);
		
		exit(1);
	}else if(pid > 0){
		child = pid;
		if( signal(SIGINT, SIG_IGN) == SIG_ERR ){
			perror("signal()");
		}

		if( wait(NULL) == -1){
			perror("wait()");
			exit(1);
		}

		child = -1;
		if( signal(SIGINT, parentSig) == SIG_ERR ){
			perror("signal()");
		}

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

		if( dup2(savestdin,0) == -1){
			perror("dup2");
			exit(1);
		}

	}


}

