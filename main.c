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

int main (int argc, char *argv[]){ 

    int conta_avioes = 0;
    Aeroporto aeroporto;
    Aviao avioes[MAX_AVIOES];

    inicializar_aeroporto(&aeroporto, NUM_PISTAS, NUM_PORTOES, NUM_TORRES);
    printf("Aguardando aviões...\n\n");
    sleep(1);

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
