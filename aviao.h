#ifndef AVIAO.H
#define AVIAO.H

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
    AGUARDANDO_EMBARQUE,
    EMBARCANDO,
    AGUARDANDO_DESEMBARQUE,
    DESEMBARCANDO,
    FINALIZADO,
} Status;

typedef struct {
    char* id_aviao;
    Tipo tipo_aviao;
    Status status_aviao;
    pthread_t thread_aviao;
} Aviao;

#endif AVIAO.H