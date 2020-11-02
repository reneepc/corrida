#ifndef _VELODROMO_H
#define _VELODROMO_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define NUM_FAIXAS 10

typedef struct linha_t {
    int pos[NUM_FAIXAS];        // Em cada linha há NUM_FAIXAS posições possíveis
    pthread_mutex_t mutex_pos[NUM_FAIXAS];
    int n_ciclistas;
    pthread_mutex_t mutex_linha;    // Experimento
} linha;

typedef struct rank_t {
    int volta_final;                // Armazena a volta em que perderam ou quebraram
    int tempo_final;                // Instante de tempo que cruzou a linha de chegada
    int quebrou;                    // Indica se o ciclista quebrou
} rank;

#endif

