#ifndef AVIAO_H
#define AVIAO_H

#include "aeroporto.h"
#include <pthread.h>

typedef enum {
    DOMESTICO,
    INTERNACIONAL
} Tipo;

typedef enum {
    AGUARDANDO_POUSO,
    POUSANDO,
    AGUARDANDO_DECOLAGEM,
    DECOLANDO,
    AGUARDANDO_DESEMBARQUE,
    DESEMBARCANDO,
    FINALIZADO,
    CAIU,
    FALHOU,
} Status;

typedef struct {
    int id_aviao;
    Tipo tipo_aviao;
    Status status_aviao;
    pthread_t thread_aviao;
    Aeroporto *aeroporto;
    int starvation;
    int estado_critico;
} Aviao;

void *rotina_aviao(void *arg);
void simular_operacao(char* operacao, int id_aviao, char *tipo_str, Aeroporto *aeroporto, int tempo_segundos);
void alocar_recursos_domestico(Aviao* aviao, time_t tempo_inicio);
void alocar_recursos_internacional(Aviao* aviao, int esperar);
void liberar_recursos_para_domesticos(Aviao* aviao);
void log_aviao(int id, const char* tipo, const char* acao, Aeroporto *aeroporto);

#endif
