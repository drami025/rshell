#include <sys/types.h>
#include <math.h>
#include <list>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <iomanip>
#include <vector>
#include <sstream>
#include <set>

using namespace std;

struct classComp{
    bool operator()(char* a, char* b){
        int compare = strcmp(a, b);

        return compare <= 0;
    }
};

void readDirectories(const vector<string>& directories, const string& allFlags, int numOfDir);
void outputLS(char* dirName, const string& flags, char* orig, bool moreDir);
void displayFiles(const multiset<char*, classComp>& sortedFiles, bool hasL, string dirName, int maxFileSize, int compareSize);
void printColumns(vector<vector<char*> > table, vector<int> widths, string dirName);

int main(int argc, char** argv)
{
    //Size I'll make vector after tranversing command line arguments
    int numOfDir = 0; stringstream dir;
    stringstream flags;

    //Traversing command line here and populate stream for directories and flags
    for(int i = 1; i < argc; i++){
        if(argv[i][0] != '-'){
            numOfDir++;
            dir << argv[i] << " ";
        }
        else{
            flags << argv[i];
        }
    }

    //Populate vector for directories
    vector<string> directories(numOfDir);
    for(int i = 0; i < numOfDir; i++){
        dir >> directories.at(i);
    }

    //Populate string for flags
    string allFlags;
    flags >> allFlags;  

    readDirectories(directories, allFlags, numOfDir);

    return 0;
}

//function to read directories
void readDirectories(const vector<string>& directories, const string& flags, int numOfDir){

    bool moreDir = numOfDir > 1;

    if(numOfDir == 0){
        char* dirName;
        dirName = (char*) ".";
        outputLS(dirName, flags, dirName, moreDir);
    }

    //Loop through all directories inputted. Only once if no arguments.
    for(unsigned i = 0; numOfDir > 0; numOfDir--, i++){
        char* dirName;

        //If less than one, then current directory. Else, passed in directory.

        dirName = (char*) directories.at(i).c_str();


        outputLS(dirName, flags, dirName, moreDir);


        if(numOfDir > 1) cout << endl;        
    }
}

//function to begin displaying all files in directory
void outputLS(char* dirName, const string& flags, char* orig, bool moreDir){

    struct stat buf;

    if(!(stat(dirName, &buf) == -1)){
 
        if(S_ISREG(buf.st_mode)){

            bool hasOtherL = false;

            for(unsigned i = 0; i < flags.size(); i++){
                if(flags.at(i) == 'l'){
                    hasOtherL = true;
                }
            }

            if(hasOtherL){
                multiset<char*, classComp> some;
                some.insert(dirName);
                displayFiles(some, hasOtherL, dirName, 0, 0);
                return;
            }
            
            if((buf.st_mode & S_IRWXU & S_IXUSR))
                cout << "\033[1;32m";
            cout << dirName << "\033[0m" << endl;
            return;
        }
    }
    else{
        perror(dirName);
        exit(-1);
    }
    
    if(moreDir){
        cout << dirName << ":" << endl;
    }

    //Opens directory name in "dirName"
    DIR *dirp = opendir(dirName);
    if(dirp == NULL){
        perror("opendir()");
        exit(-1);
    }

    bool hasA = false, hasL = false, hasR = false;

    for(unsigned i = 0; i < flags.size(); i++){
        if(flags.at(i) == 'a') hasA = true;

        if(flags.at(i) == 'l') hasL = true;

        if(flags.at(i) == 'R') hasR = true;
    }

    //Reads contents of directory, saving contents and info into "dirent" struct
    dirent *direntp;
    multiset<char*, classComp> sortedFiles;
    int maxFileSize = 0;
    long numOfChars = 0;
    int compareSize = 0;
    list<string> dirs;
    string copy = dirName;
    string test = "";

    //Branches to handle different flags.
    while ((direntp = readdir(dirp))){
        if(direntp == NULL){
            perror("readdir");
            exit(-1);
        }
        if(!hasA && (direntp->d_name[0] != '.')){
            sortedFiles.insert(direntp->d_name); // use stat here to find attributes of file
        }
        else if(hasA){
            sortedFiles.insert(direntp->d_name);
        }

        if(hasR && (direntp->d_type & DT_DIR) && strcmp(direntp->d_name, ".") && strcmp(direntp->d_name, "..")){
            dirs.push_back(direntp->d_name);
        }

        compareSize = strlen(direntp->d_name);

        if(maxFileSize < compareSize)
            maxFileSize = compareSize;

        if(numOfChars <= 80)
            numOfChars += (compareSize + 1);
        else
            numOfChars = 81;
    }    

    if(hasR){
        cout << dirName << ":" << endl;
    }

    displayFiles(sortedFiles, hasL, copy, maxFileSize, numOfChars);

    while(hasR && !dirs.empty()){
        cout << endl;
        if(!hasL) cout << endl;
        copy = dirName;
        copy += "/" + dirs.front();
        outputLS((char*) copy.c_str(), flags, orig, moreDir);
        dirs.pop_front();
    }

    if(strcmp(dirName, orig) == 0 && !hasL) cout << endl;

    if(closedir(dirp) == -1){
        perror("closedir()");
    }
}

void displayFiles(const multiset<char*, classComp>& sortedFiles, bool hasL, string dirName, int maxFileSize, int compareSize){

    if(!hasL){
        struct stat buf;
        string copy = "";

        int rows = ceil (((double) compareSize) / maxFileSize) ;
        int columns = ceil(((double) sortedFiles.size()) / rows);

        int sizeSet = sortedFiles.size();

        for(int i = 0; i < sizeSet; i++){
            if(rows * columns < sizeSet){
                columns += 1;
            }
            else
                break;
        }

        int maxColumnWord = 0;
        int i = 0, j = 0;
        bool needsColumns = compareSize > 80;
        vector<vector<char*> > table(rows);
        vector<char*> tColumns(columns);
        vector<int> columnWidths;

        for(multiset<char*, classComp>::iterator it = sortedFiles.begin(); it != sortedFiles.end(); it++){

            copy = dirName +  "/";
            copy += *it;

            //Output for no or -a flag options with color
            if(stat((char*) copy.c_str(), &buf) == -1){
                perror("stat()");
                exit(-1);
            }

            if(!needsColumns && (*it)[0] == '.'){
                cout << "\033[2;47;33m";
            }

            if(!needsColumns && S_ISDIR(buf.st_mode)){
                cout << "\033[1;34m";
            }
            else if(!needsColumns && (buf.st_mode & S_IRWXU & S_IXUSR))
                cout << "\033[1;32m";

            if(!needsColumns)
                cout << *it << "\033[0m" << "  " <<  flush;
            else{
                tColumns.at(i) = *it;
                if(maxColumnWord < (int) strlen(*it)) maxColumnWord = strlen(*it);

                i++;
                if(i >= columns){
                    i = 0;
                    table.at(j) = tColumns;
                    j++;
                    columnWidths.push_back(maxColumnWord);
                    maxColumnWord = 0;
                }
            }
        }

        if(needsColumns){
            if(columns * rows != sizeSet){
                tColumns.resize(i);
                table.at(j) = tColumns;
                columnWidths.push_back(maxColumnWord);
            }
            printColumns(table, columnWidths, dirName);
        }
    }
    else{
        struct stat buf;
        int max = 0;

        string copy = "";

        if(maxFileSize != 0 && compareSize != 0){
            for(multiset<char*, classComp>::iterator it = sortedFiles.begin(); it != sortedFiles.end(); it++){
                copy = dirName + "/";
                copy += *it;

                if(stat((char*) copy.c_str(), &buf) == -1){
                    perror("stat():");
                    exit(-1);
                }

                max += buf.st_blocks;
            }
        }

        max /= 2;

        cout << "total " << max << endl;

        copy = "";

        for(multiset<char*, classComp>::iterator it = sortedFiles.begin(); it != sortedFiles.end(); it++){

            if(maxFileSize == 0 && compareSize == 0){
                copy = *it;
            }
            else{
                copy = dirName + "/";
                copy += *it;
            }

            if(stat((char*) copy.c_str(), &buf) == -1){
                perror("stat():");
                exit(-1);
            }

            //checks whether directory or file
            if(S_ISREG(buf.st_mode))
                cout << '-';
            else if(S_ISDIR(buf.st_mode))
                cout << 'd';

            //this will check for permissions for user

            (buf.st_mode & S_IRWXU & S_IRUSR) ? cout << 'r' : cout << '-';
            (buf.st_mode & S_IRWXU & S_IWUSR) ? cout << 'w' : cout << '-';
            (buf.st_mode & S_IRWXU & S_IXUSR) ? cout << 'x' : cout << '-';

            //this will check for permissions for groups

            (buf.st_mode & S_IRWXG & S_IRGRP) ? cout << 'r' : cout << '-';
            (buf.st_mode & S_IRWXG & S_IWGRP) ? cout << 'w' : cout << '-';
            (buf.st_mode & S_IRWXG & S_IXGRP) ? cout << 'x' : cout << '-';

            //this will check for permissions for all others

            (buf.st_mode & S_IRWXO & S_IROTH) ? cout << 'r' : cout << '-';
            (buf.st_mode & S_IRWXO & S_IWOTH) ? cout << 'w' : cout << '-';
            (buf.st_mode & S_IRWXO & S_IXOTH) ? cout << 'x' : cout << '-';

            //Used to get user name from user id
            struct passwd *pass;

            pass = getpwuid(buf.st_uid);
            if(pass == NULL){
                perror("getpwuid");
                exit(-1);
            }

            char* usr = pass->pw_name;

            //Used to get group name from group id
            struct group *gpass;

            gpass = getgrgid(buf.st_gid);
            if(gpass == NULL){
                perror("getgrgid");
            }

            char* grp = gpass->gr_name;

            //Used to get time format from time_t
            struct tm myTime;

            if(localtime_r(&buf.st_mtime, &myTime) == NULL){
                perror("localtime_r");
                exit(-1);
            }

            char month[5];
            char day[3];
            char time[6];

            if(strftime(month, sizeof(month), "%b", &myTime) == 0 || strftime(day, sizeof(day), "%d", &myTime) == 0 || strftime(time, sizeof(time), "%R", &myTime) == 0){
                perror("strftime");
                exit(-1);
            }


            //Output for -l flag with color

            cout << right <<  ' ' << buf.st_nlink << ' ' << usr << ' ' << grp << ' ' << setw (6) << buf.st_size << ' ' << month << ' ' << day << ' ' << time << ' ';
            if(S_ISDIR(buf.st_mode)){
                cout << "\033[1;34m";
            }

            if((*it)[0] == '.'){
                cout << "\033[2;47;33m";
            }

            cout << *it << "\033[0m" << endl;

        }
    }
}

void printColumns(vector<vector<char*> > table, vector<int> widths, string dirName){

    string copy = "";

    int widthSize = 0; 

    for(unsigned i = 0; i < table.at(0).size(); i++){
        for(unsigned j = 0; j < table.size(); j++){

            struct stat buf;

            if(i < table.at(j).size()){

                copy = dirName +  "/";
                copy += table.at(j).at(i);

                //Output for no or -a flag options with color
                if(stat((char*) copy.c_str(), &buf) == -1){
                    perror("stat()");
                    exit(-1);
                }

                if(table.at(j).at(i)[0] == '.'){
                    // cout << "\033[0;47;37m";
                }

                if(buf.st_mode & S_IRWXU & S_IXUSR)
                    cout << "\033[1;32m";

                if(S_ISDIR(buf.st_mode)){
                    cout << "\033[1;34m";
                }

                widthSize = widths.at(j) + 2;

                cout << left << setw(widthSize) << table.at(j).at(i) << "\033[0m" << flush;
            }
        }

        widthSize = 0;

        if(i < table.at(0).size() - 1)
            cout << endl;
    }
}
