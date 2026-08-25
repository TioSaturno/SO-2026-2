#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        printf("Padre (PID: %d) terminando ya!\n", getpid());
        // El padre termina de inmediato sin esperar
    } else if (pid == 0) {
        printf("Hijo antes: mi padre es PID %d\n", getppid());
        sleep(3); // Le damos tiempo al padre de morir
        printf("Hijo después: mi padre es PID %d\n", getppid());
    }

    return 0;
}