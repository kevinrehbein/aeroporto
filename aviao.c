#include "aviao.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

extern int TEMPO_POUSO;
extern int TEMPO_DESEMBARQUE;
extern int TEMPO_DECOLAGEM;
extern int conta_starvation;
int fila_prioridade = 0;

void simular_operacao(char* operacao, int id_aviao, char *tipo_str, Aeroporto *aeroporto,int tempo_segundos) {
    char acao[30];
    snprintf(acao, sizeof(acao), "iniciando %s...", operacao);
    log_aviao(id_aviao, tipo_str, acao, aeroporto);

    sleep(tempo_segundos);

    snprintf(acao, sizeof(acao), "finalizou %s.", operacao);
    log_aviao(id_aviao, tipo_str, acao, aeroporto);

    sleep(1);
}

void alocar_recursos_domestico(Aviao* aviao, time_t tempo_inicio) {
    pthread_mutex_lock(&aviao->aeroporto->mutex_prioridade);
    while (aviao->aeroporto->internacionais_esperando > 0 && aviao->estado_critico == 0) {
        if (difftime(time(NULL),tempo_inicio) < 60){
            pthread_cond_wait(&aviao->aeroporto->cond_domestico, &aviao->aeroporto->mutex_prioridade);
        } else {
            aviao->estado_critico = 1;
            printf ("Alerta crítico 60s: aviao %d movido para fila de prioridade.\n", aviao->id_aviao);
            conta_starvation++;
            aviao->starvation = 1;
        }
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

int lock_both(sem_t *s1, sem_t *s2) {
    if (sem_trywait(s1) == 0) {
        if (sem_trywait(s2) == 0) {
            return 0;
        } else {
            sem_post(s1);
        }
    }
    return -1; // falhou
}

int lock_three(sem_t *s1, sem_t *s2, sem_t *s3) {
    if (sem_trywait(s1) == 0) {
        if (sem_trywait(s2) == 0) {
            if (sem_trywait(s3) == 0) {
                return 0;
            } else {
                sem_post(s2);
                sem_post(s1);
            }
        } else {
            sem_post(s1);
        }
    }
    return -1; // não conseguiu os três
}
void* rotina_aviao(void* arg) {
    Aviao* aviao = (Aviao*)arg;
    Aeroporto* aeroporto = aviao->aeroporto;
    char* tipo_str = (aviao->tipo_aviao == DOMESTICO) ? "Domestico" : "Internacional";

    time_t tempo_inicio;    // Relógio para Starvation

    //Pouso

    log_aviao(aviao->id_aviao, tipo_str, "se aproximando do aeroporto e aguardando pouso.", aeroporto);
    sleep(1);
    aviao->status_aviao = AGUARDANDO_POUSO;
    if (aviao->tipo_aviao == DOMESTICO) {           // Doméstico: Torre → Pista
        tempo_inicio = time(NULL);
        aviao->estado_critico = 0;
        alocar_recursos_domestico(aviao, tempo_inicio);
        fila_prioridade++;
        while (lock_both(&aeroporto->sem_torres, &aeroporto->sem_pistas) != 0) {
            usleep(250000);     //250 ms
        }
        fila_prioridade--;
        if (difftime(time(NULL),tempo_inicio) >= 90){
            sem_post(&aeroporto->sem_torres);
            sem_post(&aeroporto->sem_pistas);
            printf("Aviao %d caiu.\n", aviao->id_aviao);
            aviao->status_aviao = CAIU;
            pthread_exit(NULL);
        }
    } else {                                        // Internacional: Pista → Torre
        alocar_recursos_internacional(aviao, 1);
        while (fila_prioridade > 0) {
            sleep(1);
        }
        while (lock_both(&aeroporto->sem_torres, &aeroporto->sem_pistas) != 0) {
            usleep(250000);
        }
    }
    log_aviao(aviao->id_aviao, tipo_str, "utilizando torre e pista.", aeroporto);
    sleep(1);
    aviao->status_aviao = POUSANDO;
    simular_operacao("pouso", aviao->id_aviao, tipo_str, aeroporto, TEMPO_POUSO);
    sleep(1);
    sem_post(&aeroporto->sem_pistas);
    sem_post(&aeroporto->sem_torres);
    log_aviao(aviao->id_aviao, tipo_str, "liberando pista e torre.", aeroporto);
    if (aviao->tipo_aviao == INTERNACIONAL) {
       alocar_recursos_internacional(aviao, 0);
       liberar_recursos_para_domesticos(aviao);
    }
    

    // Desembarque
    sleep(1);
    log_aviao(aviao->id_aviao, tipo_str, "aguardando desembarque.", aeroporto);
    sleep(1);
    aviao->status_aviao = AGUARDANDO_DESEMBARQUE;
     if (aviao->tipo_aviao == DOMESTICO) {          // Doméstico: Torre → Portão
        tempo_inicio = time(NULL);
        aviao->estado_critico = 0;
        alocar_recursos_domestico(aviao, tempo_inicio);
        fila_prioridade++;
        while (lock_both(&aeroporto->sem_torres, &aeroporto->sem_portoes) != 0) {
            usleep(250000);
        }
        fila_prioridade--;
        if (difftime(time(NULL),tempo_inicio) >= 90){
            sem_post(&aeroporto->sem_torres);
            sem_post(&aeroporto->sem_pistas);
            printf("Aviao %d falhou.\n", aviao->id_aviao);
            aviao->status_aviao = FALHOU;
            pthread_exit(NULL);        }
    } else {                                        // Internacional: Portão → Torre
        alocar_recursos_internacional(aviao, 1);
        while (fila_prioridade > 0) {
            sleep(1);
        }
        while (lock_both(&aeroporto->sem_torres, &aeroporto->sem_portoes) != 0) {
            usleep(250000);
        }
    }
    log_aviao(aviao->id_aviao, tipo_str, "ocupando portao e torre.", aeroporto);
    sleep(1);
    aviao->status_aviao = DESEMBARCANDO;
    simular_operacao("desembarque", aviao->id_aviao, tipo_str, aeroporto, TEMPO_DESEMBARQUE);
    sleep(1);
    sem_post(&aeroporto->sem_torres);
    log_aviao(aviao->id_aviao, tipo_str, "liberando torre.", aeroporto);
    sleep(5); // Tempo de espera para liberar o portão.
    sem_post(&aeroporto->sem_portoes);
    log_aviao(aviao->id_aviao, tipo_str, "liberando portao", aeroporto);
     if (aviao->tipo_aviao == INTERNACIONAL) {
        alocar_recursos_internacional(aviao, 0);
        liberar_recursos_para_domesticos(aviao);
    }
    

    // Decolagem
    sleep(1);
    log_aviao(aviao->id_aviao, tipo_str, "aguardando decolagem.", aeroporto);
    sleep(1);
    aviao->status_aviao = AGUARDANDO_DECOLAGEM;
    if (aviao->tipo_aviao == DOMESTICO) {           // Doméstico: Torre → Portão → Pista
        tempo_inicio = time(NULL);
        aviao->estado_critico = 0;
        alocar_recursos_domestico(aviao, tempo_inicio);
        fila_prioridade++;
        while (lock_three(&aeroporto->sem_torres, &aeroporto->sem_portoes, &aeroporto->sem_pistas) != 0) {
            usleep(250000);
        }
        fila_prioridade--;
        if (difftime(time(NULL),tempo_inicio) >= 90){
            sem_post(&aeroporto->sem_torres);
            sem_post(&aeroporto->sem_portoes);
            sem_post(&aeroporto->sem_pistas);
            printf("Aviao %d falhou.\n", aviao->id_aviao);
            aviao->status_aviao = FALHOU;
            pthread_exit(NULL);
        }
    } else {                                        // Internacional: Portão → Pista → Torre
        alocar_recursos_internacional(aviao, 1);
        while (fila_prioridade > 0) {
            sleep(1);
        }
        while (lock_three(&aeroporto->sem_torres, &aeroporto->sem_portoes, &aeroporto->sem_pistas) != 0) {
            usleep(250000);
        }
    }
    log_aviao(aviao->id_aviao, tipo_str, "ocupando torre, portao e pista.", aeroporto);
    sleep(1);
    aviao->status_aviao = DECOLANDO;
    simular_operacao("decolagem", aviao->id_aviao, tipo_str, aeroporto, TEMPO_DECOLAGEM);
    sleep(1);
    sem_post(&aeroporto->sem_torres);
    sem_post(&aeroporto->sem_pistas);
    sem_post(&aeroporto->sem_portoes);
    log_aviao(aviao->id_aviao, tipo_str, "liberando torre, portao e pista.", aeroporto);
    if (aviao->tipo_aviao == INTERNACIONAL) {
        alocar_recursos_internacional(aviao, 0);
        liberar_recursos_para_domesticos(aviao);
    }

    aviao->status_aviao = FINALIZADO;
    sleep(1);
    log_aviao(aviao->id_aviao, tipo_str, "finalizou todas as operacoes e deixou o aeroporto.", aeroporto);

    return NULL;
}

void log_aviao(int id, const char* tipo, const char* acao, Aeroporto *aeroporto) {
    sem_getvalue(&aeroporto->sem_pistas, &aeroporto->pistas_disponiveis);
    sem_getvalue(&aeroporto->sem_torres, &aeroporto->torres_disponiveis);
    sem_getvalue(&aeroporto->sem_portoes, &aeroporto->portoes_disponiveis);
      
    struct tm *data_hora_atual;     
    time_t segundos;
    time(&segundos);   
    data_hora_atual = localtime(&segundos); 

    printf("Avião %-2d %-15s %-55s Hora: %-2d:%-2d:%-2d | RESURSOS | PISTAS: %d | PORTOES: %d | TORRES: %d\n", 
        id, tipo, acao, data_hora_atual->tm_hour, data_hora_atual->tm_min, data_hora_atual->tm_sec, 
        aeroporto->pistas_disponiveis, aeroporto->portoes_disponiveis, aeroporto->torres_disponiveis);
}