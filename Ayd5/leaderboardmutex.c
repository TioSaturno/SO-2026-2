
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define K              3     /* máximo de lectores simultáneos (parte g) */
#define NUM_JUGADORES  8
#define NUM_SERVIDORES 3
#define RONDAS         4
#define TAM_TABLA      5


/* ---- Recurso compartido: el leaderboard ---- */
int tabla[TAM_TABLA];

/* ---- Estado de sincronización (protegido por 'mutex') ---- */
int jugadores = 0;        /* jugadores leyendo el leaderboard ahora */
int servidor = 0;     /* 1 si un servidor está actualizando */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cambio = PTHREAD_COND_INITIALIZER;   /* el estado cambió */

void *lector(void *arg) {
    long id = (long)arg;

    for (int r = 0; r < RONDAS; r++) {
        /* ---- entrar ---- */
        pthread_mutex_lock(&mutex);
        while (servidor || jugadores >= K)     /* esperar si hay servidor actualizando o no hay cupo */
            pthread_cond_wait(&cambio, &mutex);  /* libera el mutex mientras espera */
        jugadores++;
        pthread_mutex_unlock(&mutex);

        /* ---- leer el leaderboard (fuera del mutex: lectores en paralelo) ---- */
        int version = tabla[0];
        int consistente = 1;
        for (int i = 1; i < TAM_TABLA; i++) {
            usleep(1000);
            if (tabla[i] != version)
                consistente = 0;
        }
        printf("Lector %ld leyó la versión %d %s\n", id, version,
               consistente ? "" : "<-- INCONSISTENTE");

        /* ---- salir ---- */
        pthread_mutex_lock(&mutex);
        jugadores--;
        pthread_cond_broadcast(&cambio);         /* hay cupo y quizá entra un servidor */
        pthread_mutex_unlock(&mutex);

        usleep(2000);
    }
    return NULL;
}

void *escritor(void *arg) {
    long id = (long)arg;

    for (int r = 0; r < RONDAS; r++) {
        /* ---- entrar ---- */
        pthread_mutex_lock(&mutex);
        while (servidor || jugadores > 0)      /* esperar a que no haya nadie */
            pthread_cond_wait(&cambio, &mutex);
        servidor = 1;
        pthread_mutex_unlock(&mutex);

        /* ---- actualizar el leaderboard (fuera del mutex) ---- */
        int nueva = tabla[0] + 1;
        for (int i = 0; i < TAM_TABLA; i++) {
            tabla[i] = nueva;
            usleep(1000);
        }
        printf("Servidor %ld actualizó a la versión %d\n", id, nueva);

        /* ---- salir ---- */
        pthread_mutex_lock(&mutex);
        servidor = 0;
        pthread_cond_broadcast(&cambio);         /* despertar a lectores y escritores */
        pthread_mutex_unlock(&mutex);

        usleep(3000);
    }
    return NULL;
}

int main(void) {
    pthread_t hl[NUM_SERVIDORES], he[NUM_JUGADORES];

    for (int i = 0; i < TAM_TABLA; i++)
        tabla[i] = 0;

    for (long i = 0; i < NUM_JUGADORES; i++)
        pthread_create(&hl[i], NULL, lector, (void *)(i + 1));
    for (long i = 0; i < NUM_SERVIDORES; i++)
        pthread_create(&he[i], NULL, escritor, (void *)(i + 1));

    for (int i = 0; i < NUM_JUGADORES; i++)
        pthread_join(hl[i], NULL);
    for (int i = 0; i < NUM_SERVIDORES; i++)
        pthread_join(he[i], NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cambio);
    return 0;
}