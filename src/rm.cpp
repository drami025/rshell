#include<iostream>
#include<dirent.h>
#include<errno.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<cstdlib>
#include<string.h>

using namespace std;

int removeDirs(string dirName){

	DIR *dirp = opendir(dirName.c_str());

	if(dirp == NULL){
		perror("opendir");
		return -1;
	}

	dirent *direntp;

	while((direntp = readdir(dirp))){
		if(direntp == NULL){
			perror("readdir");
			return -1;
		}
	
		if((direntp->d_type & DT_DIR) && strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name,"..") != 0){

			string directory = dirName + "/";
			directory = directory + direntp->d_name;
			removeDirs(directory.c_str());
	
			if(rmdir(directory.c_str()) == -1){
				perror("rmdir");
				return -1;
			}
		}
		else if(strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name,"..") != 0){
			
			string path = direntp->d_name;
			path = dirName + "/" + path;

			if(unlink(path.c_str()) == -1){
				perror("unlink");
				return -1;
			}
		}

	}

	return 0;
}

int main(int argc, char** argv){

	struct stat buf;

	bool hasR = false;

	for(int i = 0; i < argc; i++){
		if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0)
			hasR = true;
	}

	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-r") != 0 && strcmp(argv[i], "-R") != 0){
			if(stat(argv[i], &buf) == -1){
				perror("stat");
				return -1;
			}

			if(hasR && S_ISDIR(buf.st_mode)){
				string dir = argv[i];
				dir = "./" + dir;
				if(removeDirs(dir) == -1){
					perror("removeDirs");	
					return -1;
				}

				if(rmdir(dir.c_str()) == -1){
					perror("rmdir main");
					return -1;
				}
			}
			else if(!hasR && S_ISDIR(buf.st_mode)){
				cout << "rm: cannot remove `" << argv[i] << "\': Is a directory" << endl;
			}
			else{
				if(unlink(argv[i]) == -1){
					perror("unlink");
					return -1;
				}
			}
		}
	}

	return 0;

}
