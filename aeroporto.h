#ifndef AEROPORTO.H
#define AEROPORTO.H

#include <semaphore.h>

typedef struct{
    int num_pistas;
    int num_torres;
    int num_portoes;

    sem_t sem_pistas;
    sem_t sem_torres;
    sem_t sem_portoes;

} Aeroporto;

void inicializar_aeroporto(Aeroporto* aeroporto, int num_pistas, int num_portoes, int num_torres);
void destruir_aeroporto(Aeroporto* aeroporto);

#endif