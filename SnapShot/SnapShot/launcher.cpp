#include<unistd.h>
#include <sys/wait.h>
#include<iostream>
using namespace std;
int main(int argc, char** argv){
    if(argc != 3){
        cout <<"./launcher <peer file> <number of snapshot>"<<endl;
        return 1;
    }
    int snapshots = atoi(argv[2]);    
    if(snapshots %4 != 0){
        cout << "please use correct number of snapshots"<<endl;
        return 1;
    }
    int i = 0;
    int pid = 0;
    int pids[4];
    int child_status;
    char** params = new char*[4];
    snapshots = snapshots / 4;
    params[0] = new char[100];
    params[1] = new char[100];
    params[2] = new char[100];
    params[3] = new char[100];
    strcpy(params[0], "application");
    strcpy(params[2],  argv[1]);
    sprintf(params[3], "%d", snapshots);

    while(i < 4){
        if((pid = fork()) == 0){
            sprintf(params[1], "%d", i);
            execl("./application", params[0], params[1], params[2], params[3], (char*)0); 

        }else{
            pids[i] = pid;
        }
        ++i;
    }

    for(int i = 0; i < 4; ++i){
        waitpid(pids[i], &child_status, WUNTRACED);
    }
    return 0;
}
