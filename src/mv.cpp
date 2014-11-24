#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<errno.h>

using namespace std;

int main(int argc, char** argv){
	
	string args[2];

	args[0] = "";
	args[1] = "";

	bool argOneExists = false;
	bool argTwoExists = false;

	int j = 0;

	for(int i = 1; i < argc; i++){
		if(argv[i][0] != '-'){
			if(j < 2){
				args[j] = argv[i];
				j++;
			}
			else{
				cerr << "Too many arguments" << endl;
				return -1;
			}
		}
	}

	struct stat buf;

	if(access(args[0].c_str(), F_OK) == -1){
		cerr << "mv: cannot stat \'" << args[0] << "\': No such file or directory" << endl;
	}
	else{
		argOneExists = true;
	}

	if(stat(args[1].c_str(), &buf) != -1){
		argTwoExists = true;
	}

	if(argTwoExists && S_ISDIR(buf.st_mode)){

		args[1] = args[1] + "/" + args[0];

		if(link(args[0].c_str(), args[1].c_str()) == -1){
			perror("link");
			return -1;
		}

		if(unlink(args[0].c_str()) == -1){
			perror("unlink in dir");
			return -1;
		}
	}
	else if(argTwoExists){
		if(S_ISREG(buf.st_mode)){
			cerr << "\'" << args[1] << "\' is an existing file" << endl;
			return -1;
		}	
	}
	else{
		if(link(args[0].c_str(), args[1].c_str()) == -1){
			perror("rename");
			return -1;
		}

		if(unlink(args[0].c_str()) == -1){
			perror("unlink");
			return -1;
		}	
	}
	

	
	
	return 0;
}
