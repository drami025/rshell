#include<iostream>
#include<fcntl.h>
#include<pwd.h>
#include<sstream>
#include<stdio.h>
#include<boost/tokenizer.hpp>
#include<errno.h>
#include<string>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/unistd.h>

using namespace std;
using namespace boost;

typedef tokenizer<char_separator<char> > Tok;

void readCommands(string str);
int conjunct(int n, stringstream& ss, const Tok::iterator &it);
void skipCommand(Tok::iterator &it, Tok &tokens);
void splitString(char** args, stringstream& ss, int n);
void ioRedir(const Tok::iterator &it, bool inRedir, bool append);
void resetIO(int in0, int out1);
void pipeRedir(int fd[], bool pipeOut);
void resetPipe(int fd, bool isOut);

int main(){
    
    cout << endl;
    string str = "";
    struct passwd *pass;
    
    pass = getpwuid(getuid());
    if(pass == NULL){
        perror("getpwuid()");
    }

    char* usrname = pass->pw_name;
    char hostname[128];
    
    int success = gethostname(hostname, sizeof hostname);
    if(success == -1){
        perror("gethostname()");
    }
    
    string userinfo = "";
    string username = "";

    if(usrname != NULL && success == 0){
        username = usrname;
        userinfo = username + "@" + hostname;
    }

    bool notExited = true;

    while(notExited){
        cout << userinfo << "$ ";
        getline(cin, str);
        readCommands(str);
    }

    return 0;
}

void readCommands(string str){

    char_separator<char> sep("\" ", ";#|&<>");

    Tok tokens(str, sep);
    stringstream ss;
    int in0;
    int out1;
    int fd[2];
    bool pipeOut = false;
    bool didRedir = false;
    bool didPipe = false;
    int n = 0;

    if((in0 = dup(0)) == -1){
        perror("input-0");
        exit(-1);
    }

    if((out1 = dup(1)) == -1){
        perror("output-1");
        exit(-1);
    }

    if(pipe(fd) == -1){
        perror("pipe");
        exit(-1);
    }

    for(Tok::iterator it = tokens.begin(); it != tokens.end(); it++){
        
        if(*it == ";"){
            conjunct(n, ss, it);
            n = 0;
            it++;
            if(it == tokens.end()) break;
        }
        else if(*it ==  "&"){
            if(conjunct(n, ss, it) == -1){
                skipCommand(it, tokens);
            }
            n = 0;
            if(it == tokens.end()) break;
            it++;
            if(it == tokens.end()) break;
        }
        else if(*it == "|"){
            Tok::iterator copy = it;
            copy++;

            if(copy == tokens.end() || *copy == ";" || *copy == "&"){
                conjunct(0, ss, it);
                skipCommand(it, tokens);
                return;
            }

            if(*copy == "|"){
                copy++;
                if(copy == tokens.end() || *copy == ";" || *copy == "&"){
                    conjunct(0, ss, it);
                    skipCommand(it, tokens);
                    return;
                }

                if(conjunct(n, ss, it) != -1){
                    skipCommand(it, tokens);
                }
            }  
            else{
                didPipe = true;
                pipeOut = true;
                pipeRedir(fd, pipeOut);
                if(conjunct(n, ss, it) == -1){
                    skipCommand(it, tokens);
                }
                pipeOut = false;
                pipeRedir(fd, pipeOut);

                n = 0;
                continue;
            }
//TODO Piping
            n = 0;
            if(it == tokens.end()) break;
            it++;
        }
        else if(*it == "<" || *it == ">"){

            another:

            bool inRedir = false;
            bool append = false;
            
            if(*it == "<") inRedir = true;
            
            it++;

            here:

            if(it == tokens.end()){
                cerr << "Bash: syntax error near unexpected token \'newline\'" << endl;
                exit(-1);
            }

            if(*it == ">"){
                append = true;
                it++;
                goto here;
            }

            ioRedir(it, inRedir, append);
            
            didRedir = true;
            
            it++;
            
            if(it == tokens.end()) break;

            if(*it == ">" || *it == "<") goto another;
        }

        if(*it == "#"){
            conjunct(n, ss, it);
            return;
        }

        if(*it != "&" && *it != "|"){
            ss << *it;
            ss << " ";
            n++;
        }
    }

    if(n > 0){
        Tok::iterator it = tokens.begin();
        if(didPipe){
            resetPipe(out1, true);
        }
        conjunct(n, ss, it);
        if(didPipe){
            resetPipe(in0, false);
        }
    }

    if(didRedir){
        resetIO(in0, out1);
    }
}

int conjunct(int n, stringstream& ss, const Tok::iterator &it){
    
    int status;

    if(n == 0 && *it != "#"){
        cerr << "Bash: syntax error near unexpected token \'" << *it << "\'" << endl;
        return -1;
    }
    else if(n == 0 && *it == "#") return 0; 
    char** args = new char*[n + 1];

    splitString(args, ss, n);

    int pid = fork();

    if(pid == -1){
        perror("There was an error with the fork().");
        exit(1);
    }
    else if(pid == 0){
        if(-1 == execvp((const char*) args[0], (char* const*) args)){
            perror(args[0]);
            exit(3);
        }
    }
    else if(pid > 0){
        if(-1 == wait(&status)){
            perror("There was an error with wait().");
            return -1;
        }

        if(WIFEXITED(status)){
            if(WEXITSTATUS(status) == 3){
                delete[] args;
                return -1;
            }
        }
    }
    delete[] args;

    return 0;
}


//TODO INPUT REDIRECTION
void ioRedir(const Tok::iterator &it, bool inRedir, bool append){

    if(*it == "&" || *it == "|" || *it == ";"){
        cerr << "Bash: syntax error near unexpected token \'" << *it << "\'" << endl;
        exit(-1);
    }

    if(inRedir){
        string input = *it;

        int fdi;

        if((fdi = open((char*) input.c_str(), O_RDONLY)) == -1){
            perror("open input");
            exit(-1);
        }
        
        if(close(0) == -1){
            perror("close input");
            exit(-1);
        }

        if(dup(fdi) == -1){
            perror("dup input");
            exit(-1);
        }
        return;
    }
    else{
        string output = *it;
        
        int fdo;
        mode_t readWrite = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

        if(append){
            if((fdo = open((char*) output.c_str(), O_RDWR | O_CREAT | O_APPEND,  readWrite)) == -1){
                perror("open output");
                exit(-1);
            }
        }
        else{
            if((fdo = open((char*) output.c_str(), O_RDWR | O_CREAT | O_TRUNC,  readWrite)) == -1){
                perror("open output append");
                exit(-1);
            }
        }
            
        if(close(1) == -1){
            perror("close output");
            exit(-1);
        }

        if(dup(fdo) == -1){
            perror("dup output");
            exit(-1);
        }
    }
}

void resetIO(int in0, int out1){

    if(close(0) == -1){
        perror("close fd");
        exit(-1);
    }

    if(dup2(in0, 0) == -1){
        perror("dup input - 0");
        exit(-1);
    }

    if(close(1) == -1){
        perror("close fd");
        exit(-1);
    }

    if(dup2(out1, 1) == -1){
        perror("dup output - 1");
        exit(-1);
    }
}

void resetPipe(int fd, bool isOut){
    if(isOut){
        if(dup2(fd, 1) == -1){
            perror("dup2 pipe output reset");
            exit(-1);
        }
    }
    else{
        if(dup2(fd, 0) == -1){
            perror("dup2 pipe input reset");
            exit(-1);
        }
    }
}

void pipeRedir(int fd[], bool pipeOut){
    if(pipeOut){
        if(dup2(fd[1], 1) == -1){
            perror("dup2: pipe out");
            exit(-1);
        }
    }
    else{
        if(dup2(fd[0], 0) == -1){
            perror("dup2: pipe in");
            exit(-1);
        }
    }
}

void skipCommand(Tok::iterator &it, Tok &tokens){
    while(it != tokens.end() && *it != ";"){
        it++;
    }
}

void splitString(char** args, stringstream& ss, int n){

    string *str = new string[n];

    for(int i = 0; i < n; i++){

        ss >> str[i];

        args[i] = (char*) str[i].c_str();
    }

    args[n] = NULL;

    char exitC[] = "exit";

    if(strcmp(args[0], exitC) == 0){
        cout << endl;
        exit(0);
    }

    delete[] str;
}
