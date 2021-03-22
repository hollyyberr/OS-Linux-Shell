#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main() {
    pid_t childPid = fork();

    // CHILD
    if(childPid == 0) {
        printf("----- Child -----\nCurrent PID: %d and Child PID: %d\n", getpid(), childPid);
    }
    else {
        sleep(1); // Tells shell to sleep for (1) second
        printf("----- Parent -----\nCurrent PID: %d and Child PID: %d\n", getpid(), childPid);
    }

    return 0;
}