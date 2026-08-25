#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        printf("Soy el padre, mi PID es %d\n", getpid());
    } else if (pid == 0) {
        printf("Soy el hijo, mi PID es %d\n", getpid());
    }

    return 0;
}