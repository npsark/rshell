//C++ Headers
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>

//C Headers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>




#define FLAG_a 1
#define FLAG_l 2
#define FLAG_R 4

using namespace std;


struct windowStats {
	int totalCharCount;
	uint windowWidth;
	int howManyPrinted;
	int widestName;
	int widestSize;
};

int runCommand(char **argv);
void outputFlag_NOT_L(struct stat statBuff, vector<string> files, struct windowStats *winStats);
void outputFlag_L(struct stat statBuff, vector<string> files, struct windowStats *winStats);
bool strCustomCompare( string A, string B);
void populateWindowStats(struct windowStats *winStats, vector<string> files, struct stat *statBuff);


int main(int argc, char **argv)
{
	struct windowStats winStats;

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	winStats.windowWidth = w.ws_col;


	int flags = 0;

	vector<string> dirNames;
		dirNames.push_back(".");

	for(uint i=1; argv[i]!=0; i++){
		if(argv[i][0] == '-'){
			for(uint j=1; argv[i][j]!=0; j++){
				if(argv[i][j] == 'a'){
					flags |= FLAG_a;
				}else if(argv[i][j] == 'l'){
					flags |= FLAG_l;
				}else if(argv[i][j] == 'R'){
					flags |= FLAG_R;
				}
			}
		}else{
			if(dirNames[0] == "."){
				dirNames.pop_back();
			}
			dirNames.push_back(string(argv[i]));
		}
	}

	sort( dirNames.begin(), dirNames.end(), strCustomCompare );//alphabetize directory names	


	for( uint p=0; p<dirNames.size(); p++){
		
		vector<string> files;
		DIR *dirp = NULL;

		struct stat isDirStatBuff;
		stat(dirNames[p].c_str(), &isDirStatBuff);

		if( !(S_ISDIR(isDirStatBuff.st_mode)) ){
			files.push_back(dirNames[p]);
		}else{

			if( !(flags & FLAG_R) ){
				cout << dirNames[p] << ":" << endl;
			}


			DIR *dirp = opendir(dirNames[p].c_str());
			if(dirp == NULL){
				perror("opendir");
				return 1;
			}

			dirent *direntp;


			while ( (direntp = readdir(dirp)) ){
				if(direntp == NULL){
					perror("readdir");
				}

				string currFile = direntp->d_name;
				files.push_back(currFile);
			}

			sort( files.begin(), files.end(), strCustomCompare );//alphabetize file names
		}


		if(!(flags & FLAG_a)){//if -a is not a parameter, delete all file names that start with '.'
			for(uint i=0; i<files.size(); i++){		
				if(files[i][0] == '.'){
					files.erase(files.begin()+i);
					i=-1;
				}
			}
		}
		struct stat statBuff;
		
		populateWindowStats(&winStats, files, &statBuff);
	
	
		vector<string> dirs;


		if(flags & FLAG_R){
			cout << dirNames[p] << ":" << endl;
		}

		if(flags & FLAG_l){
		
			outputFlag_L(statBuff, files, &winStats);

		}else{	

			outputFlag_NOT_L(statBuff, files, &winStats);

		}

		if(flags & FLAG_R){


			struct stat dirStatBuff;
			for(uint i=0; i<files.size(); i++){//collect all the file names that represent directories
				string fullFilePath = dirNames[p] + "/" + files[i];
				stat(fullFilePath.c_str(), &dirStatBuff);
				if( S_ISDIR(dirStatBuff.st_mode) ){
					dirs.push_back(files[i]);
				}
			}


			cout << endl << endl;
			if( !(flags && FLAG_l) ){
				cout << endl;
			}
		
			for(uint k=0; k<dirs.size(); k++){
				if(dirs[k] != "." && dirs[k] != ".."){
					//cout << dirs[k] << ":" << endl;

					char **args = new char*[4];//build arguments array to pass to execvp via runCommand().
			
					args[0] = new char[100];//first element will contain the path to this program.
					strcpy( args[0], "/home/nat/cs100/rshell/bin/ls" );
			
					string childDir = dirNames[p];

					if(dirNames[p] != "/"){
						childDir += "/" + dirs[k];
					}else{
						childDir += dirs[k];
					}
					args[1] = new char[childDir.length() + 1];
					strcpy( args[1], childDir.c_str() );

					string params = "-";
					if( flags & FLAG_a ){
						params += "a";
					}
					if( flags & FLAG_l ){
						params += "l";
					}
					if( flags & FLAG_R ){
						params += "R";
					}

					args[2] = new char[params.length() + 1];
					strcpy( args[2], params.c_str() );

					args[3] = NULL;

					runCommand(args);
					//cout << "HEYO" << endl;
					//if( k < dirs.size()-1 ){
					//	cout << endl;
					//}
				}
			}
		}



		if( !(flags & FLAG_l) && !(flags & FLAG_R) ) cout << endl;

		if(dirp){
			if(closedir(dirp) == -1){
				perror("closedir");
			}	
		}
	}
}


int runCommand(char **argv){

	int status = 0;

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
		
		if(wait(&status) == -1){
			perror("wait() had an error.\n");
		}

	}
	
	return status;

}



void outputFlag_L(struct stat statBuff, vector<string> files, struct windowStats *winStats){
	cout << "total " << files.size() << endl;

	for(uint i=0; i<files.size(); i++){
		stat(files[i].c_str(), &statBuff);

		if(S_ISDIR(statBuff.st_mode)){
			cout << "d";
		}else if(S_ISLNK(statBuff.st_mode)){
			cout << "s";
		}else{ cout << "-"; }
	

		if(statBuff.st_mode & S_IRWXU & S_IRUSR){
			cout << "r";
		}else{ cout << "-"; }
		if(statBuff.st_mode & S_IRWXU & S_IWUSR){
			cout << "w";
		}else{ cout << "-"; }
		if(statBuff.st_mode & S_IRWXU & S_IXUSR){
			cout << "x";
		}else{ cout << "-"; }


		if(statBuff.st_mode & S_IRWXG & S_IRGRP){
			cout << "r";
		}else{ cout << "-"; }
		if(statBuff.st_mode & S_IRWXG & S_IWGRP){
			cout << "w";
		}else{ cout << "-"; }
		if(statBuff.st_mode & S_IRWXG & S_IXGRP){
			cout << "x";
		}else{ cout << "-"; }
	

		if(statBuff.st_mode & S_IRWXO & S_IROTH){
			cout << "r";
		}else{ cout << "-"; }
		if(statBuff.st_mode & S_IRWXO & S_IWOTH){
			cout << "w";
		}else{ cout << "-"; }
		if(statBuff.st_mode & S_IRWXO & S_IXOTH){
			cout << "x";
		}else{ cout << "-"; }

		cout << " " << statBuff.st_nlink;
	
		struct passwd *uID = getpwuid(statBuff.st_uid);
		struct group *gID = getgrgid(statBuff.st_gid);

		if(uID){
			cout << " " << getpwuid(statBuff.st_uid)->pw_name;
		}else{
			cout << " ";
			//perror("getpwuid");
		}
	
		if(gID){		
			cout << " " << getgrgid(statBuff.st_gid)->gr_name;
		}else{
			cout << " ";
			//perror("getgrgid");
		}

		cout << " " << setw(winStats->widestSize) << statBuff.st_size;


		time_t t = statBuff.st_mtime;
	
		tm * tminfo = localtime(&t);
		char buffer[50]; //c-string buffer of size 50

		strftime(buffer, 50, "%b %d %R", tminfo);
		cout << " " << buffer << " "; //puts prints the content of the c-string, which in this case contains the time


		cout << files[i] << "\t" << endl;
	}
}

void outputFlag_NOT_L(struct stat statBuff, vector<string> files, struct windowStats *winStats){
	for(uint i=0; i<files.size(); i++){
		cout << "YO" << endl;

		if(int(winStats->totalCharCount) > int(winStats->windowWidth)){
			if( uint(winStats->howManyPrinted + 1) > winStats->windowWidth/(winStats->widestName+1) ){
				cout << endl;
				winStats->howManyPrinted = 0;
			}
			winStats->howManyPrinted += 1;	
			cout << left << setw(winStats->widestName+1) << files[i];
		}else{
			cout << files[i] << "  ";
		}

	}
}


bool strCustomCompare( string A, string B){
	uint i=0;

	string a = A, b = B;
	
	transform(a.begin(), a.end(), a.begin(), ::tolower);
	transform(b.begin(), b.end(), b.begin(), ::tolower);

	while(a[i] == b[i]){
		if( i < a.length() && i < b.length() ){
			i++;
		}else{
			if(a.length() > b.length()){
				return false;
			}else{
				return true;
			}
		}

	}

	if(a[i] > b[i] ){
		return false;
	}else{
		return true;
	}
}


void populateWindowStats(struct windowStats *winStats, vector<string> files, struct stat *statBuff){
	winStats->totalCharCount = 0;//this will contain the total number of charachters to be printed.
	
	for(uint i=0; i<files.size(); i++){
		winStats->totalCharCount += files[i].length();//add the number of chars in each file name to be printed.
	}
	winStats->totalCharCount += (files.size())*2;//add 2 for each space between file names.

	

	int biggestSize = 0;//this will contain the largest file size of the files to be printed.
	winStats->widestName = 0;//this will contain the length of the longest file name to be printed.
	for(uint i=0; i<files.size(); i++){

		stat(files[i].c_str(), statBuff);

		if( biggestSize < statBuff->st_size){
			biggestSize = statBuff->st_size;
		}
		if( int(winStats->widestName) < int(files[i].length()) ){
			winStats->widestName = files[i].length();
		}

	}


	winStats->widestSize = 0;//this will contain the number of chars in the largest file size to be printed.	
	stringstream ss;//convert the largest file size to a string so we can get the number of chars.
	ss << biggestSize;//
	winStats->widestSize = ss.str().length();//

	winStats->howManyPrinted = 0;


}

