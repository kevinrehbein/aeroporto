#include "aeroporto.h"
#include <stdio.h>
#include <stdlib.h>

void inicializar_aeroporto(Aeroporto* aeroporto, int num_pistas, int num_portoes, int num_torres) {
    
    if (num_pistas <= 0 || num_portoes <= 0 || num_torres <= 0) {
        perror("Número inválido de pistas, portões e torres.");
        exit(EXIT_FAILURE);
    }

    aeroporto->internacionais_esperando = 0;
    aeroporto->domesticos_esperando = 0;

    if (sem_init(&aeroporto->sem_pistas, 0, num_pistas) != 0 ||
        sem_init(&aeroporto->sem_portoes, 0, num_portoes) != 0 ||
        sem_init(&aeroporto->sem_torres, 0, num_torres * 2) != 0) {     // Torres atendem duas operações
        perror("Erro ao inicializar semáforos");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&aeroporto->mutex_prioridade, NULL) != 0 ||
        pthread_cond_init(&aeroporto->cond_domestico, NULL) != 0) {
        perror("Erro ao inicializar mutex/condição de prioridade");
        exit(EXIT_FAILURE);
    }

    printf("Aeroporto inicializado com sucesso: %d pistas, %d portões, %d torres.\n", 
        num_pistas, num_portoes, num_torres);
    printf("Aguardando aviões...\n\n");
    sleep(1);
}

void destruir_aeroporto(Aeroporto* aeroporto) {

    sem_destroy(&aeroporto->sem_torres);
    sem_destroy(&aeroporto->sem_pistas);
    sem_destroy(&aeroporto->sem_portoes);
    pthread_mutex_destroy(&aeroporto->mutex_prioridade);
    pthread_cond_destroy(&aeroporto->cond_domestico);
    printf("Recursos do aeroporto liberados com sucesso.\n");
}