#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main() {
    pid_t childPid = fork();

    // CHILD
    if (childPid == 0) {
        printf("----- Child ----- \nCurrent PID: %d and Child PID: %d\n", getpid(), childPid);
    }
    else {
        printf("----- Parent -----\nCurrent PID: %d and Child PID: %d\n", getpid(), childPid);
    }

    return 0;
}