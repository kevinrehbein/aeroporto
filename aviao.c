#include "aviao.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void simular_operacao(char* operacao, int id_aviao, int tempo_segundos) {
    printf("Avião %d Iniciando %s...\n", id_aviao, operacao);
    sleep(tempo_segundos);
    printf("Avião %d Finalizou %s.\n", id_aviao, operacao);
}

void alocar_recursos_domestico(Aviao* aviao) {
    pthread_mutex_lock(&aviao->aeroporto->mutex_prioridade);
    while (aviao->aeroporto->internacionais_esperando > 0) {
        pthread_cond_wait(&aviao->aeroporto->cond_domestico, &aviao->aeroporto->mutex_prioridade);
    }
    pthread_mutex_unlock(&aviao->aeroporto->mutex_prioridade);
}

void alocar_recursos_internacional(Aviao* aviao, int esperar) {
    pthread_mutex_lock(&aviao->aeroporto->mutex_prioridade);
    if(esperar) 
        aviao->aeroporto->internacionais_esperando++;
    else aviao->aeroporto->internacionais_esperando--;
    pthread_mutex_unlock(&aviao->aeroporto->mutex_prioridade);
}

void liberar_recursos_para_domesticos(Aviao* aviao){
    pthread_mutex_lock(&aviao->aeroporto->mutex_prioridade);
    pthread_cond_broadcast(&aviao->aeroporto->cond_domestico);
    pthread_mutex_unlock(&aviao->aeroporto->mutex_prioridade);
}

void* rotina_aviao(void* arg) {
    Aviao* aviao = (Aviao*)arg;
    Aeroporto* aeroporto = aviao->aeroporto;
    char* tipo_str = (aviao->tipo_aviao == DOMESTICO) ? "Doméstico" : "Internacional";

    //Pouso

    printf("Avião %d (%s) se aproximando do aeroporto e aguardando pouso.\n", aviao->id_aviao, tipo_str);
    sleep(1);
    aviao->status_aviao = AGUARDANDO_POUSO;
    if (aviao->tipo_aviao == DOMESTICO) {           // Doméstico: Torre → Pista
        alocar_recursos_domestico(aviao);
        sem_wait(&aeroporto->sem_torres);
        sem_wait(&aeroporto->sem_pistas);
    } else {                                        // Internacional: Pista → Torre
        alocar_recursos_internacional(aviao, 1);
        sem_wait(&aeroporto->sem_pistas);
        sem_wait(&aeroporto->sem_torres);
    }
    printf("Avião %d (%s) utilizando torre e pista.\n", aviao->id_aviao, tipo_str);
    sleep(1);
    aviao->status_aviao = POUSANDO;
    simular_operacao("pouso", aviao->id_aviao, 10);
    sleep(1);
    sem_post(&aeroporto->sem_pistas);
    sem_post(&aeroporto->sem_torres);
    printf("Avião %d (%s) liberando pista e torre.\n", aviao->id_aviao, tipo_str);    
    if (aviao->tipo_aviao == INTERNACIONAL) {
       alocar_recursos_internacional(aviao, 0);
       liberar_recursos_para_domesticos(aviao);
    }

    // Desembarque
    sleep(1);
    printf("Avião %d (%s) Aguardando desembarque.\n", aviao->id_aviao, tipo_str);
    sleep(1);
    aviao->status_aviao = AGUARDANDO_DESEMBARQUE;
     if (aviao->tipo_aviao == DOMESTICO) {          // Doméstico: Torre → Portão
        alocar_recursos_domestico(aviao);
        sem_wait(&aeroporto->sem_torres);
        sem_wait(&aeroporto->sem_portoes);
    } else {                                        // Internacional: Portão → Torre
        alocar_recursos_internacional(aviao, 1);
        sem_wait(&aeroporto->sem_portoes);
        sem_wait(&aeroporto->sem_torres);
    }
    printf("Avião %d (%s) ocupando portão e torre.\n", aviao->id_aviao, tipo_str);   
    sleep(1);
    aviao->status_aviao = DESEMBARCANDO;
    simular_operacao("desembarque", aviao->id_aviao, 10);
    sleep(1);
    sem_post(&aeroporto->sem_torres);
    printf("Avião %d (%s) liberando torre.\n", aviao->id_aviao, tipo_str);
    sleep(5); // Tempo de espera para liberar o portão.
    sem_post(&aeroporto->sem_portoes);
    printf("Avião %d (%s) liberando portão\n", aviao->id_aviao, tipo_str);
     if (aviao->tipo_aviao == INTERNACIONAL) {
        alocar_recursos_internacional(aviao, 0);
        liberar_recursos_para_domesticos(aviao);
    }

    // Decolagem
    sleep(1);
    printf("Avião %d (%s) aguardando decolagem.\n", aviao->id_aviao, tipo_str);
    sleep(1);
    aviao->status_aviao = AGUARDANDO_DECOLAGEM;
    if (aviao->tipo_aviao == DOMESTICO) {           // Doméstico: Torre → Portão → Pista
        alocar_recursos_domestico(aviao);
        sem_wait(&aeroporto->sem_torres);
        sem_wait(&aeroporto->sem_portoes);
        sem_wait(&aeroporto->sem_pistas);
    } else {                                        // Internacional: Portão → Pista → Torre
        alocar_recursos_internacional(aviao, 1);
        sem_wait(&aeroporto->sem_portoes);
        sem_wait(&aeroporto->sem_pistas);
        sem_wait(&aeroporto->sem_torres);
    }
    printf("Avião %d (%s) ocupando torre, portão e pista.\n", aviao->id_aviao, tipo_str);
    sleep(1);
    aviao->status_aviao = DECOLANDO;
    simular_operacao("decolagem", aviao->id_aviao, 10);
    sleep(1);
    sem_post(&aeroporto->sem_torres);
    sem_post(&aeroporto->sem_pistas);
    sem_post(&aeroporto->sem_portoes);
    printf("Avião %d (%s) liberando torre, pista e portão.\n", aviao->id_aviao, tipo_str);
    if (aviao->tipo_aviao == INTERNACIONAL) {
        alocar_recursos_internacional(aviao, 0);
        liberar_recursos_para_domesticos(aviao);
    }

    aviao->status_aviao = FINALIZADO;
    printf("Avião %d (%s) finalizou todas as operações e deixou o aeroporto.\n", aviao->id_aviao, tipo_str);

    return NULL;
}
