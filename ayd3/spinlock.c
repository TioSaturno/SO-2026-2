#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int lock = 0;

void *thread_func(void *arg) {
    while(lock);
    lock = 1;
    // SC
    lock = 0;
}
int main() {
    pthread_t threads[4];
    for(int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }
    for(int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

}