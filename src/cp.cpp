#include <iostream>
#include<stdlib.h>
#include<sys/time.h>
#include<sys/wait.h>
#include<errno.h>
#include<fstream>
#include"Timer.h"
#include<unistd.h>
#include<stdio.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<vector>

using namespace std;

int main(int argc, char** argv){


    if(argc < 2){
        cout << "Missing file operand" << endl;
    }
    else if(argc < 3){
        cout << "Missing destination file operand after \'" << argv[1] << "\'";
    }

    int x = 0;

    vector<char*> allArgs(2);

    for(int i = 1; i < argc; i++){
        if(argv[i][0] != '-'){
            allArgs.at(x) = argv[i];
            x++;
        }
    }

    Timer t;
    double wallclock, user, system;

    //THIS IS USING in.get() & out.put()

    if(argc == 4){
        const int numArgs = argc - 2;
        int x = 0;

        vector<char*> allArgs(numArgs);

        for(int i = 1; i < argc; i++){


            if(argv[i][0] != '-'){

                allArgs.at(x) = argv[i];

                x++;

            }

        }



        t.start();



        ifstream fin;

        ofstream fout;



        fin.open(allArgs.at(0));

        fout.open(allArgs.at(1));



        while(!fin.eof()){


            fout.put(fin.get());

        }



        fin.close();

        fout.close();



        cout << "-----------Running time-----------" << endl;



        t.elapsedWallclockTime(wallclock);

        cout << "Wallclock: " <<  wallclock << endl;



        t.elapsedTime(wallclock, user, system);

        cout << "System: " << system << endl;

        cout << "User: " << user << endl;

        cout << endl;



        //--------------------------------



        // THIS IS USING read() & write() [SINGLE CHAR]





        t.start();

        int fileDes = open((const char*) allArgs.at(0), O_RDONLY);

        char data[128];



        if(fileDes == -1){


            perror("open()");

            return -1;

        }



        int fileLoc = open((const char*) allArgs.at(1), O_WRONLY | O_CREAT);



        if(fileLoc == -1){


            perror("open() #2");

            return -1;

        }
        int check;



        while((check = read(fileDes, data, sizeof(char))) != 0){




            if(check == -1)

                perror("read()");



            if(write(fileLoc, data, 1) == -1)

                perror("write()");

        }



        if(close(fileDes) == -1){


            perror("close() fileDes");

            return -1;

        }

        else if(close(fileLoc) == -1){


            perror("close() fileLoc");

            return -1;

        }



        cout << "-----------Running time-----------" << endl;



        t.elapsedWallclockTime(wallclock);

        cout << "Wallclock: " <<  wallclock << endl;



        t.elapsedTime(wallclock, user, system);

        cout << "System: " << system << endl;

        cout << "User: " << user << endl;

        cout << endl;

        //-----------------------------------

    }



    //THIS IS read() & write() WITH BUF





    t.start();



    int fileDes2 = open((const char*) allArgs.at(0), O_RDONLY);

    char otherData[BUFSIZ];
    if(fileDes2 == -1){


        perror("open #3");

        return -1;

    }



    int fileLoc2;



    if(argc <= 3)

        fileLoc2 = open((const char*) allArgs.at(1), O_WRONLY | O_CREAT | O_EXCL);

    else

        fileLoc2 = open((const char*) allArgs.at(1), O_WRONLY | O_CREAT);



    if(fileLoc2 == -1){


        perror("open() #4");

        return -1;

    }



    int check2;



    while((check2 = read(fileDes2, otherData, BUFSIZ)) != 0){


        if(check2 == -1){


            perror("read() #2");

            return -1;

        }



        if(write(fileLoc2, otherData, BUFSIZ) == -1){


            perror("write() #2");

            return -1;

        }

    }



    if(close(fileDes2) == -1){


        perror("close() fileDes");

        return -1;

    }

    else if(close(fileLoc2) == -1){


        perror("close() fileLoc");

        return -1;

    }



    if(argc == 4){


        cout << "-----------Running time-----------" << endl;

        t.elapsedWallclockTime(wallclock);

        cout << "Wallclock: " << wallclock << endl;



        t.elapsedTime(wallclock, user, system);

        cout << "System: " << system << endl;

        cout << "User: " << user << endl;

        cout << endl;

    }
    return 0;



}


