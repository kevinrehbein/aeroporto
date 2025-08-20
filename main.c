#include "aeroporto.h"
#include "aviao.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define MAX_AVIOES 100  // Apenas para simplificar, evitando lista encadeada
#define NUM_PISTAS 3
#define NUM_PORTOES 5
#define NUM_TORRES 1    // Atendem duas operações simultâneamente
const int TEMPO_POUSO = 10;
const int TEMPO_DESEMBARQUE = 10;
const int TEMPO_DECOLAGEM = 10;

void* monitora_deadlock(void* arg);

int main (int argc, char *argv[]){ 

    int conta_avioes = 0;
    Aeroporto aeroporto;
    Aviao avioes[MAX_AVIOES];

    inicializar_aeroporto(&aeroporto, NUM_PISTAS, NUM_PORTOES, NUM_TORRES);

    time_t inicio = time(NULL);
    while (difftime(time(NULL), inicio) < 30.0){   // Roda por 30s. 
        
        srand(time(NULL));

        avioes[conta_avioes].id_aviao = conta_avioes;
        avioes[conta_avioes].tipo_aviao = (rand() % 2 == 0) ?  DOMESTICO : INTERNACIONAL;
        avioes[conta_avioes].aeroporto = &aeroporto;   

        if (pthread_create(&avioes[conta_avioes].thread_aviao, NULL, rotina_aviao, &avioes[conta_avioes]) != 0){
            perror("Erro ao criar threads!\n");
            exit(EXIT_FAILURE);
        }

        sleep((rand() % 11) + 5);   // Cria threads (aviões) com intervalos randomicos entre 5s e 15s.

        conta_avioes++;
    }

    printf("TEMPO ACABOU\n");

    for (int i = 0; i < conta_avioes; i++) {
        if (pthread_join(avioes[i].thread_aviao, NULL) != 0) {
            perror("Erro ao aguardar thread do avião\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("\nTodos os aviões concluíram seus voos.\n");
    destruir_aeroporto(&aeroporto);
}

void* monitora_deadlock(void* arg) {
    Aeroporto *aeroporto = (Aeroporto*) arg;
    int inicio_pistas, inicio_torres, inicio_portoes;
    int sem_mudanca = 0;

    while (1) {
        inicio_pistas = aeroporto->pistas_disponiveis;
        inicio_torres = aeroporto->torres_disponiveis;
        inicio_portoes = aeroporto->portoes_disponiveis;

        sleep(5);                   // verifica a 5 cada segundos

        if (aeroporto->pistas_disponiveis == inicio_pistas && 
            aeroporto->torres_disponiveis == inicio_torres &&
            aeroporto->portoes_disponiveis == inicio_portoes) {
            sem_mudanca++;
        } else {
            sem_mudanca = 0;        // houve mundança
        }

        if (sem_mudanca >= 6) {     // 30s sem mudança.
            printf("\nALERTA: DEADLOCK detectado! Recursos parados.\n");
            printf("Todos os avioes (threads) serão derrubados.");
        }
    }
    return NULL;
}
