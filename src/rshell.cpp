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
    string input = "";
    string output = "";
    int n = 0;

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
// Remove this part once we deal with piping            
            if(copy != tokens.end() && *copy != "|") {
                conjunct(0, ss, it);
                skipCommand(it, tokens);
            }
//======
            if(copy != tokens.end() && *copy == "|"){
                if(conjunct(n, ss, it) != -1){
                    skipCommand(it, tokens);
                }
            }    
            n = 0;
            if(it == tokens.end()) break;
            it++;
        }
        else if(*it == "<" || *it == ">"){
//TODO INPUT REDIRECTION
            bool inRedir = false;
            bool append = false;
            
            if(*it == "<") inRedir = true;
            
            it++;

            here:

            if(it == tokens.end()){
                cout << "Bash: syntax error near unexpected token \'newline\'" << endl;
                exit(-1);
            }

            if(*it == ">"){
                append = true;
                it++;
                goto here;
            }

            ioRedir(it, inRedir, append);
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
        conjunct(n, ss, it);
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

    if(close(0) == -1){
        perror("close input fd");
        exit(-1);
    }

    if(dup(0) == -1){
        perror("dup input");
        exit(-1);
    }

    if(close(1) == -1){
        perror("close output fd");
        exit(-1);
    }
    
    if(dup(1) == -1){
        perror("dup output");
        exit(-1);
    }
    return 0;
}


//TODO INPUT REDIRECTION
void ioRedir(const Tok::iterator &it, bool inRedir, bool append){

    if(*it == "&" || *it == "|" || *it == ";"){
        cout << "Bash: syntax error near unexpected token \'" << *it << "\'" << endl;
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

        if(append){
            if((fdo = open((char*) output.c_str(), O_RDWR | O_CREAT | O_APPEND)) == -1){
                perror("open output");
                exit(-1);
            }
        }
        else{
            if((fdo = open((char*) output.c_str(), O_RDWR | O_CREAT | O_TRUNC)) == -1){
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
