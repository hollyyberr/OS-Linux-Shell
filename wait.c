#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main() {
    pid_t childPid;
    pid_t waitResult;
    int statLoc;

    childPid = fork();

    // CHILD
    if (childPid == 0) {
        printf("----- Child -----\nCurrent PID: %d and Child PID: %d\n", getpid(), childPid);
    }
    else {
        waitResult = waitpid(childPid, &statLoc, WUNTRACED);
        printf("----- Parent -----\nCurrent PID: %d and Child PID: %d\n", getpid(), childPid);
    }

    return 0;
}