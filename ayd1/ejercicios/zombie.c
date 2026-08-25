#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        printf("Padre (PID: %d) durmiendo 30 segundos...\n", getpid());
        sleep(30); // El padre no llama a wait()
    } else if (pid == 0) {
        printf("Hijo (PID: %d) terminando ya!\n", getpid());
        // El hijo termina de inmediato
    }

    return 0;
}