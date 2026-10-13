
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define MODO          2
#define K             3      /* lectores simultáneos permitidos */
#define NUM_SERVIDORES  8
#define NUM_JUGADORES 3
#define RONDAS        4
#define TAM_TABLA     5

/* ---- Recurso compartido: el leaderboard ----
 * Invariante: todas las posiciones tienen el mismo valor (la "versión").
 * Un escritor las actualiza una por una; si un lector mira a mitad de
 * la actualización, verá valores distintos: leaderboard inconsistente. */
int tabla[TAM_TABLA];

/* ---- Sincronización ---- */
int jugadores = 0;                                            /* jugadores viendo el leaderboard dentro ahora */
pthread_mutex_t mutex_jugadores = PTHREAD_MUTEX_INITIALIZER;  /* protege 'lectores' */
sem_t bd;      /* = 1. Acceso exclusivo al leaderboard (lo toma el primer lector o un escritor) */
sem_t turno;   /* = 1. Torniquete: evita la inanición de escritores (solo MODO 2) */
sem_t cupo;    /* = K. Limita cuántos jugadores leen a la vez */

void *lector(void *arg) {
    long id = (long)arg;

    for (int r = 0; r < RONDAS; r++) {
        if (MODO == 2) {
            sem_wait(&turno);      /* pasa el torniquete... */
            sem_post(&turno);      /* ...y lo libera de inmediato */
        }
        sem_wait(&cupo);           /* (g) un lugar de lectura, máximo K */

        pthread_mutex_lock(&mutex_jugadores);
        jugadores++;
        if (jugadores == 1)
            sem_wait(&bd);         /* el primer lector bloquea a los escritores */
        pthread_mutex_unlock(&mutex_jugadores);

        /* ---- leer el leaderboard ---- */
        int version = tabla[0];
        int consistente = 1;
        for (int i = 1; i < TAM_TABLA; i++) {
            usleep(1000);
            if (tabla[i] != version)
                consistente = 0;
        }
        printf("Lector %ld leyó la versión %d %s\n", id, version,
               consistente ? "" : "<-- INCONSISTENTE");

        pthread_mutex_lock(&mutex_jugadores);
        jugadores--;
        if (jugadores == 0)
            sem_post(&bd);         /* el último lector deja entrar a los escritores */
        pthread_mutex_unlock(&mutex_jugadores);

        sem_post(&cupo);
        usleep(2000);
    }
    return NULL;
}

void *escritor(void *arg) {
    long id = (long)arg;

    for (int r = 0; r < RONDAS; r++) {
        if (MODO == 2)
            sem_wait(&turno);      /* cierra el torniquete: lectores nuevos esperan */
        sem_wait(&bd);             /* espera a que salgan los lectores actuales */

        /* ---- actualizar el leaderboard ---- */
        int nueva = tabla[0] + 1;
        for (int i = 0; i < TAM_TABLA; i++) {
            tabla[i] = nueva;
            usleep(1000);
        }
        printf("Servidor %ld actualizó a la versión %d\n", id, nueva);

        if (MODO == 2)
            sem_post(&turno);      /* reabre el torniquete */
        sem_post(&bd);
        usleep(3000);
    }
    return NULL;
}

int main(void) {
    pthread_t hl[NUM_LECTORES], he[NUM_ESCRITORES];

    sem_init(&bd, 0, 1);
    sem_init(&turno, 0, 1);
    sem_init(&cupo, 0, K);

    for (int i = 0; i < TAM_TABLA; i++)
        tabla[i] = 0;

    for (long i = 0; i < NUM_LECTORES; i++)
        pthread_create(&hl[i], NULL, lector, (void *)(i + 1));
    for (long i = 0; i < NUM_ESCRITORES; i++)
        pthread_create(&he[i], NULL, escritor, (void *)(i + 1));

    for (int i = 0; i < NUM_LECTORES; i++)
        pthread_join(hl[i], NULL);
    for (int i = 0; i < NUM_ESCRITORES; i++)
        pthread_join(he[i], NULL);

    sem_destroy(&bd);
    sem_destroy(&turno);
    sem_destroy(&cupo);
    pthread_mutex_destroy(&mutex_lectores);
    return 0;
}