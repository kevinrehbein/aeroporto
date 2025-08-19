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
} Status;

typedef struct {
    int id_aviao;
    Tipo tipo_aviao;
    Status status_aviao;
    pthread_t thread_aviao;
    Aeroporto *aeroporto;
} Aviao;

void *rotina_aviao(void *arg);
#endif
