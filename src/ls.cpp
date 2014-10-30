#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>

#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>


#define FLAG_a 1
#define FLAG_l 2
#define FLAG_R 4

using namespace std;




bool strCustomCompare( string A, string B);


int main(int argc, char **argv)
{
	int flags = 0;

	string dirName = ".";

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
			dirName = argv[i];
		}
	}

	if(flags & FLAG_a);// cout << "a" << endl;
	if(flags & FLAG_l);// cout << "l" << endl;
	if(flags & FLAG_R);// cout << "R" << endl;

	vector<string> files;
	//char *dirName = new char[5];
	//strcpy(dirName, ".");


	DIR *dirp = opendir(dirName.c_str());
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

	sort( files.begin(), files.end(), strCustomCompare );

	if(!(flags & FLAG_a)){
		for(uint i=0; i<files.size(); i++){		
			if(files[i][0] == '.'){
				//cout << "erasing: " << files[i] << endl;
				files.erase(files.begin()+i);
				i=-1;
			}
		}
	}

	struct stat statBuff;

	int widest = 0;
	
	if(flags & FLAG_l){
		
		int biggest = 0;

		for(uint i=0; i<files.size(); i++){
			stat(files[i].c_str(), &statBuff);
			if( biggest < statBuff.st_size){
				biggest = statBuff.st_size;
			}
		}
		
		char *intStr = new char[10];
		widest = to_string(biggest).length();
	
	}


	

	for(uint i=0; i<files.size(); i++){
		if(flags & FLAG_l){
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
			cout << " " << getpwuid(statBuff.st_uid)->pw_name;
			cout << " " << getgrgid(statBuff.st_gid)->gr_name;

			cout << " " << setw(widest) << statBuff.st_size;


			time_t t = statBuff.st_mtime;
			
			tm * tminfo = localtime(&t);
			char buffer[50]; //c-string buffer of size 50

			strftime(buffer, 50, "%b %d %R", tminfo);
			cout << " " << buffer << " "; //puts prints the content of the c-string, which in this case contains the time


			//cout << " " << statBuff.st_ctime;



			cout << files[i] << "\t" << endl;
		}else{		
			cout << files[i] << "  ";
		}
	}
	cout << endl;



	


	if(closedir(dirp) == -1){
		perror("closedir");
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


