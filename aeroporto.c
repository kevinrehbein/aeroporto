#include "aeroporto.h"
#include <stdio.h>
#include <stdlib.h>

void inicializar_aeroporto(Aeroporto* aeroporto, int num_pistas, int num_portoes, int num_torres) {
    int teste[2];

    aeroporto->num_pistas = num_pistas;
    aeroporto->num_portoes = num_portoes;
    aeroporto->num_torres = num_torres;

    //arg '0' indica que o semáforo é compartilhado entre threads do mesmo processo
    teste[0] = sem_init(&aeroporto->sem_pistas, 0, num_pistas);
    teste[1] = sem_init(&aeroporto->num_portoes, 0, num_portoes);
    teste[2] = sem_init(&aeroporto->sem_torres, 0, num_torres);

    for (int i=0; i<=2; i++){
        if (teste[i] != 0){
            perror("Erro ao criar semáforo!");
            exit(EXIT_FAILURE);
        }
    }

    printf("Aeroporto inicializado com sucesso: %d pistas, %d portões, %d torres.\n", 
        num_pistas, num_portoes, num_torres);
}

void destruir_aeroporto(Aeroporto* aeroporto) {

    sem_destroy(&aeroporto->sem_torres);
    sem_destroy(&aeroporto->sem_pistas);
    sem_destroy(&aeroporto->num_portoes);
    printf("Recursos do aeroporto liberados com sucesso.\n");
}