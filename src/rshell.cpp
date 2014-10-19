#include<iostream>
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

int main(){
    
    cout << endl;
    string str;
    string usrname = getlogin();
    char hostname[128];
    int success = gethostname(hostname, sizeof hostname);
    string userinfo = "";

    if(usrname != "\0" && success == 0){
        userinfo = usrname + "@" + hostname;
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

    char_separator<char> sep("\" ", ";");

    Tok tokens(str, sep);
    stringstream ss;
    int n = 0;

    for(Tok::iterator it = tokens.begin(); it != tokens.end(); it++){
        if(*it == "#"){
            conjunct(n, ss, it);
            return;
        }
        else if(*it == ";"){
            conjunct(n, ss, it);
            n = 0;
            it++;
            if(it == tokens.end()) break;
        }
        else if(*it ==  "&&"){
           if(conjunct(n, ss, it) == -1){
                skipCommand(it, tokens);
            }
            n = 0;
            if(it == tokens.end()) break;
            it++;
        }
        else if(*it == "||"){
            if(conjunct(n, ss, it) != -1){
                skipCommand(it, tokens);
            }
            n = 0;
            if(it == tokens.end()) break;
            it++;
        }
        
        ss << *it;
        ss << " ";
        n++;
    }

    if(n > 0){
        Tok::iterator it = tokens.begin();
        conjunct(n, ss, it);
    }
}

int conjunct(int n, stringstream& ss, const Tok::iterator &it){
    
    if(n == 0 && *it != "#"){
        cerr << "Bash: syntax error near unexpected token \'" << *it << "\'" << endl;
        return -2;
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
            exit(1);
        }
    }
    else if(pid > 0){
        if(-1 == wait(0)){
            perror("There was an error with wait().");
            return -1;
        }
    }
    return 0;
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
