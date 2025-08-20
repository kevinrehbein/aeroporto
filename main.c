#include "aeroporto.h"
#include "aviao.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define TEMPO_EXECUCAO 300 // 5 minutos
#define MAX_AVIOES 100  // Apenas para simplificar, evitando lista encadeada
#define NUM_PISTAS 3
#define NUM_PORTOES 5
#define NUM_TORRES 1    // Atendem duas operações simultâneamente
const int TEMPO_POUSO = 5;
const int TEMPO_DESEMBARQUE = 10;
const int TEMPO_DECOLAGEM = 5;

int conta_avioes = 0;
int conta_deadlocks = 0;
int conta_starvation = 0;

Aeroporto aeroporto;
Aviao avioes[MAX_AVIOES];

void* monitora_deadlock(void* arg);
void gera_avioes();
void liberar_aviao(Aviao *aviao);
void mostra_relatorio();

int main (int argc, char *argv[]){ 

    inicializar_aeroporto(&aeroporto, NUM_PISTAS, NUM_PORTOES, NUM_TORRES);

    if (pthread_create(&aeroporto.monitor_deadlock, NULL, monitora_deadlock, NULL) != 0) {
        perror("Erro ao criar thread monitor deadlock!");
        exit(EXIT_FAILURE);
    };
    gera_avioes();

    for (int i = 0; i < conta_avioes; i++) {
        if (pthread_join(avioes[i].thread_aviao, NULL) != 0) {
            perror("Erro ao aguardar thread do avião\n");
            exit(EXIT_FAILURE);
        }
    }

    mostra_relatorio();
    pthread_cancel(aeroporto.monitor_deadlock);
    destruir_aeroporto(&aeroporto);
}

void* monitora_deadlock(void* arg) {

    int inicio_pistas, inicio_torres, inicio_portoes;
    int sem_mudanca = 0;

    while (1) {
        inicio_pistas = aeroporto.pistas_disponiveis;
        inicio_torres = aeroporto.torres_disponiveis;
        inicio_portoes = aeroporto.portoes_disponiveis;

        for (int i = 0; i <= 60; i++){
            sleep(1);                   // verifica a cada 1 segundo 60 vezes
            if (aeroporto.pistas_disponiveis == inicio_pistas && 
                aeroporto.torres_disponiveis == inicio_torres &&
                aeroporto.portoes_disponiveis == inicio_portoes) {
                sem_mudanca++;
            } else {
                sem_mudanca = 0;        // houve mundança
            }

        if (sem_mudanca == 30)          // 30s sem mudança
            printf("Risco de DEADLOCK!\n");
        }

        if (sem_mudanca >= 60) {         // 60s sem mudança.
            printf("ALERTA: DEADLOCK detectado! Recursos parados.\n");
            printf("Todos os avioes (threads) serão encerrados.\n");
            conta_deadlocks++;

            for (int i = 0; i < conta_avioes; i++){ 
                liberar_aviao(&avioes[i]);
            }
        }
    }
}

void gera_avioes() {

    time_t inicio = time(NULL);
    while (difftime(time(NULL), inicio) < TEMPO_EXECUCAO){   // Roda por 30s. 
        srand(time(NULL));

        avioes[conta_avioes].id_aviao = conta_avioes;
        avioes[conta_avioes].tipo_aviao = (rand() % 2 == 0) ?  DOMESTICO : INTERNACIONAL;
        avioes[conta_avioes].aeroporto = &aeroporto;   
        avioes[conta_avioes].starvation = 0;

        if (pthread_create(&avioes[conta_avioes].thread_aviao, NULL, rotina_aviao, &avioes[conta_avioes]) != 0){
            perror("Erro ao criar threads!\n");
            exit(EXIT_FAILURE);
        }
        sleep((rand() % 15) + 5);   // Cria threads (aviões) com intervalos randomicos entre 5 e 20s.
        conta_avioes++;
    }
    printf("TEMPO ACABOU\n");
}

void liberar_aviao(Aviao *aviao){
    if (aviao->status_aviao != FINALIZADO){
        if (aviao->status_aviao == AGUARDANDO_POUSO) {
            ("Aviao %d caiu.\n", aviao->id_aviao);
            aviao->status_aviao = CAIU;
        } else {
            printf("Aviao %d liberado.", aviao->id_aviao);
            aviao->status_aviao = FALHOU;
        }
        pthread_cancel(aviao->thread_aviao);
    }
}

void mostra_relatorio() {
    int sucessos = 0;
    int falhas = 0;
    int quedas = 0;

    for (int i = 0; i < conta_avioes; i++) {
        if (avioes[i].status_aviao == FINALIZADO) {
            sucessos++;
        } else if (avioes[i].status_aviao == FALHOU) {
            falhas++;
        } else if (avioes[i].status_aviao == CAIU) {
            quedas++;
        }
    }

    printf("\n===== RELATÓRIO FINAL DO AEROPORTO =====\n");
    printf("Total de aviões simulados: %d\n", conta_avioes);
    printf("Sucessos (finalizados): %d\n", sucessos);
    for (int i = 0; i < conta_avioes; i++) {
        if (avioes[i].status_aviao == FINALIZADO)
            printf ("Aviao %d ", avioes[i].id_aviao);
    }
    
    printf("\nFalhas: %d\n", falhas);
    for (int i = 0; i < conta_avioes; i++) {
        if (avioes[i].status_aviao == FALHOU)
            printf ("Aviao %d ", avioes[i].id_aviao);
    }
    printf("\nQuedas: %d\n", quedas);
    for (int i = 0; i < conta_avioes; i++) {
        if (avioes[i].status_aviao == CAIU)
            printf ("Aviao %d ", avioes[i].id_aviao);
    }
    printf("\nCasos de starvation detectados (avioes que esperaram mais de 60s/falharam/cairam): %d\n", conta_starvation);
    for (int i = 0; i < conta_avioes; i++) {
        if (avioes[i].starvation == 1)
            printf ("Aviao %d ", avioes[i].id_aviao);
    }
    printf("\nCasos de deadlock detectados: %d\n", conta_deadlocks);
    printf("========================================\n");
}