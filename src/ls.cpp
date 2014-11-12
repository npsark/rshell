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
#include <math.h>



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
void outputFlag_L(struct stat statBuff,  vector<string> filePaths, vector<string> fileNames, struct windowStats *winStats, bool printTot = true);
bool strCustomCompare( string A, string B);
void populateWindowStats(struct windowStats *winStats, vector<string> filePaths, vector<string> fileNames, struct stat *statBuff);


int main(int argc, char **argv){
	
	char wdbuff[1024];
	getcwd(wdbuff, sizeof(wdbuff));
	string cwd = wdbuff;

	
	struct windowStats winStats;

	struct winsize w;

	if( ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 ){
		perror("ioctl");
		exit(1);
	}

	winStats.windowWidth = w.ws_col;


	int flags = 0;


	vector<string> dirNames, dirPaths;

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
			string toAdd = string( argv[i] );
			if(toAdd.length() > 1){
				if( toAdd[ toAdd.length()-1 ] == '/' ){
					toAdd.erase( toAdd.begin() + toAdd.length()-1 );
				}
			}

			if(toAdd != cwd){
				if(toAdd[0] == '/'){
					dirPaths.push_back(toAdd);
					dirNames.push_back(toAdd);
				}else{
					dirPaths.push_back( cwd + "/" + toAdd );
					dirNames.push_back( toAdd );
				}
			}
		}
	}

	if(dirPaths.size() == 0){
		dirPaths.push_back(cwd);
		dirNames.push_back(".");
	}

	sort( dirPaths.begin(), dirPaths.end(), strCustomCompare );//alphabetize directory paths
	sort( dirNames.begin(), dirNames.end(), strCustomCompare );//alphabetize directory names	

	vector<string> notDirFiles, notDirNames;


	for( uint p=0; p<dirPaths.size(); p++){
		struct stat isDirStatBuff;

		if( lstat(dirPaths[p].c_str(), &isDirStatBuff) == -1 ){
			perror("lstat");
		}


		if( !(S_ISDIR(isDirStatBuff.st_mode)) ){
			
			

			notDirFiles.push_back(dirPaths[p]);
			notDirNames.push_back(dirNames[p]);

			dirPaths.erase(dirPaths.begin() + p);
			dirNames.erase(dirNames.begin() + p);
			p = -1;
			continue;
		}

				

	}

	if(notDirFiles.size() > 0){

		struct stat statBuff2;
	
		populateWindowStats(&winStats, notDirFiles, notDirNames, &statBuff2);
		
		if(flags & FLAG_l){
			outputFlag_L(statBuff2, notDirFiles, notDirNames, &winStats, false);
		}else{	
			outputFlag_NOT_L(statBuff2, notDirFiles, &winStats);
		}

		cout << endl << endl;
	}

	

	for( uint p=0; p<dirPaths.size(); p++){
		vector<string> filePaths, fileNames;
		DIR *dirp = NULL;

		if(dirPaths.size() > 1){
			if( !(flags & FLAG_R) ){
				cout << dirNames[p] << ":" << endl;
			}
		}


		dirp = opendir(dirPaths[p].c_str());
		if(dirp == NULL){
			perror("opendir");
			return 1;
		}

		dirent *direntp;


		while ( (direntp = readdir(dirp)) ){
			if(direntp == NULL){
				perror("readdir");
			}

			
			string currFile = "";
			if(dirPaths[p] != "/"){
				currFile = dirPaths[p] + "/" + direntp->d_name;
			}else{
				currFile = "/";
				currFile += direntp->d_name;
			}
			filePaths.push_back(currFile);
			fileNames.push_back(direntp->d_name);
			
		}

		sort( filePaths.begin(), filePaths.end(), strCustomCompare );//alphabetize file names
		sort( fileNames.begin(), fileNames.end(), strCustomCompare );//alphabetize file names


		if(!(flags & FLAG_a)){//if -a is not a parameter, delete all file names that start with '.'
			for(uint i=0; i<fileNames.size(); i++){		
				if(fileNames[i][0] == '.'){
					filePaths.erase(filePaths.begin()+i);
					fileNames.erase(fileNames.begin()+i);					
					i=-1;
				}
			}
		}
		struct stat statBuff;

		populateWindowStats(&winStats, filePaths, fileNames, &statBuff);
	
	
		vector<string> dirs;


		if(flags & FLAG_R){
			if(dirNames[p].find(cwd) != string::npos){
				cout << dirNames[p].substr( cwd.length()+1 );
			}else{
				cout << dirNames[p];
			}
			cout << ":" << endl;
		}

		if(flags & FLAG_l){
			outputFlag_L(statBuff, filePaths, fileNames, &winStats);
		}else{	
			outputFlag_NOT_L(statBuff, fileNames, &winStats);
		}

		if( !(flags & FLAG_R) ){
				cout << endl;
		}

		if(flags & FLAG_R){

			struct stat dirStatBuff;

			for(uint i=0; i<filePaths.size(); i++){//collect all the file names that represent directories

				string fullFilePath = filePaths[i];// +  files[i];
				

				if( lstat(fullFilePath.c_str(), &dirStatBuff) == -1 ){
					perror("stat");
					exit(1);
				}

				if( S_ISDIR(dirStatBuff.st_mode) ){
					string fp;
					int res = fullFilePath.rfind("/");
					if(int(res) != int(string::npos)){
						fp = fullFilePath.substr( res+1 );
					}else{
						fp = fullFilePath;
					}
					
					if( fp != "." && fp != ".."){
						dirs.push_back(fullFilePath);
					}
				}

			}


			cout << endl;

			if( !(flags & FLAG_l) ){
				cout << endl;
			}
		
			for(uint k=0; k<dirs.size(); k++){
				//cout << "filePaths = " << filePaths
				//cout << "dirs[k] = " << dirs[k] << endl;

				if(dirs[k] != "." && dirs[k] != ".."){

					char **args = new char*[4];//build arguments array to pass to execvp via runCommand().
			
					args[0] = new char[100];//first element will contain the path to this program.
					strcpy( args[0], "bin/ls" );
			
					string childDir = dirs[k];

					args[1] = new char[dirs[k].length() + 1];//childDir.length() + 1];
					strcpy( args[1], dirs[k].c_str() );//childDir.c_str() );

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

		perror("fork()");
		exit(1);

	}else if(pid == 0){//child

		if(execvp(argv[0], argv) == -1){
			perror("execvp()");
		}
		
		exit(1);

	}else if(pid > 0){//parent
		
		if(wait(&status) == -1){
			perror("wait()");
		}

	}
	
	return status;

}



void outputFlag_L(struct stat statBuff, vector<string> filePaths, vector<string> fileNames, struct windowStats *winStats, bool printTot){
	uint mostLinks = 0;	
	int totalBlocks = 0;
	for(uint i=0; i<filePaths.size(); i++){
		if( lstat(filePaths[i].c_str(), &statBuff) == -1 ){
			perror("stat");
			exit(1);
		}
		if(4 >= statBuff.st_size/1024){
			totalBlocks += 4;
		}else{
			float toAdd = statBuff.st_size;
			int numOfDivs = 0;
			while( toAdd>10 ){
				toAdd /= 10;
				numOfDivs++;
			}

			toAdd = ceil(toAdd);

			for(int h=0; h<numOfDivs; h++){
				toAdd *= 10;
			}
			totalBlocks += int(ceil(toAdd/1024));
		}
		
		if(mostLinks < statBuff.st_nlink){
			mostLinks = statBuff.st_nlink;
		}


	}
	
	stringstream ss;//convert the largest link count to a string so we can get the number of chars.
	ss << mostLinks;//
	mostLinks = ss.str().length();//

	

	if(printTot)	
		cout << "total " << totalBlocks << endl;

	for(uint i=0; i<filePaths.size(); i++){
		if(lstat(filePaths[i].c_str(), &statBuff) == -1){
			perror("stat");
			exit(1);
		}

		if(S_ISDIR(statBuff.st_mode)){
			cout << "d";
		}else if(S_ISLNK(statBuff.st_mode)){
			cout << "l";
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

		
	

		cout << " " << setw(mostLinks) << statBuff.st_nlink;
	
		struct passwd *uID = getpwuid(statBuff.st_uid);
		struct group *gID = getgrgid(statBuff.st_gid);

		if(uID){
			cout << " " << getpwuid(statBuff.st_uid)->pw_name;
		}else{
			cout << " ?";
			//perror("getpwuid");
		}
	
		if(gID){		
			cout << " " << getgrgid(statBuff.st_gid)->gr_name;
		}else{
			cout << " ?";
			//perror("getgrgid");
		}

		cout << " " << setw(winStats->widestSize) << statBuff.st_size;


		time_t t = statBuff.st_mtime;
	
		tm * tminfo = localtime(&t);
		if( tminfo == NULL ){
			perror("localtime()");
			exit(1);
		}
		char buffer[50]; //c-string buffer of size 50

		strftime(buffer, 50, "%b %d %R", tminfo);
		cout << " " << buffer << " "; //puts prints the content of the c-string, which in this case contains the time


		cout << fileNames[i] << "\t";
		if(printTot) cout << endl;
	}
}

void outputFlag_NOT_L(struct stat statBuff, vector<string> fileNames, struct windowStats *winStats){
	for(uint i=0; i<fileNames.size(); i++){

		if(int(winStats->totalCharCount) > int(winStats->windowWidth)){
			if( uint(winStats->howManyPrinted + 1) > winStats->windowWidth/(winStats->widestName+1) ){
				cout << endl;
				winStats->howManyPrinted = 0;
			}
			winStats->howManyPrinted += 1;	
			cout << left << setw(winStats->widestName+1) << fileNames[i];
		}else{
			cout << fileNames[i] << "  ";
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


void populateWindowStats(struct windowStats *winStats,  vector<string> filePaths, vector<string> fileNames, struct stat *statBuff){
	winStats->totalCharCount = 0;//this will contain the total number of charachters to be printed.
	
	for(uint i=0; i<fileNames.size(); i++){
		winStats->totalCharCount += fileNames[i].length();//add the number of chars in each file name to be printed.
	}
	winStats->totalCharCount += (fileNames.size())*2;//add 2 for each space between file names.

	

	int biggestSize = 0;//this will contain the largest file size of the files to be printed.
	winStats->widestName = 0;//this will contain the length of the longest file name to be printed.
	for(uint i=0; i<fileNames.size(); i++){

		if( lstat(filePaths[i].c_str(), statBuff) == -1 ){	
			perror("stat");
			exit(1);
		}

		if( biggestSize < statBuff->st_size){
			biggestSize = statBuff->st_size;
		}
		if( int(winStats->widestName) < int(fileNames[i].length()) ){
			winStats->widestName = fileNames[i].length();
		}

	}


	winStats->widestSize = 0;//this will contain the number of chars in the largest file size to be printed.	
	stringstream ss;//convert the largest file size to a string so we can get the number of chars.
	ss << biggestSize;//
	winStats->widestSize = ss.str().length();//

	winStats->howManyPrinted = 0;


}

