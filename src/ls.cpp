#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/*
 * This is a BARE BONES example of how to use opendir/readdir/closedir.  Notice
 * that there is no error checking on these functions.  You MUST add error 
 * checking yourself.
 */

//bool strCustomCompare( string A, string B ){
	//alphabetize
//}

int main()
{

	vector<string> files;
	char *dirName = new char[5];
	strcpy(dirName, ".");

	DIR *dirp = opendir(dirName);

	dirent *direntp;


	while ( (direntp = readdir(dirp)) ){
		string currFile = direntp->d_name;
		files.push_back(currFile);
		//cout << currFile << endl;  // use stat here to find attributes of file
	}

	//sort( files.begin()+2, files.end() );//, strCustomCompare );

	for(uint i=0; i<files.size(); i++){
		cout << files[i] << endl;
	}



	


	closedir(dirp);
}

