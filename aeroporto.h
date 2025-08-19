#ifndef AEROPORTO_H
#define AEROPORTO_H

#include <pthread.h>
#include <semaphore.h>

typedef struct{
    sem_t sem_pistas;
    sem_t sem_torres;
    sem_t sem_portoes;

    pthread_mutex_t mutex_prioridade;
    pthread_cond_t cond_domestico;
    int internacionais_esperando;
    int domesticos_esperando;
} Aeroporto;

void inicializar_aeroporto(Aeroporto* aeroporto, int num_pistas, int num_portoes, int num_torres);
void destruir_aeroporto(Aeroporto* aeroporto);
#endif