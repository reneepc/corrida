#ifndef _VELODROMO_H
#define _VELODROMO_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define NUM_FAIXAS 10

void cria_pista(pthread_t id_threads[]);

void atualiza_numero(int pista_atual, int prox_pista);

int verifica_quebra();

int atualiza_velocidade(int* vel);

void atualiza_posicao(int* pista_atual, int* prox_pista, int* faixa_atual, int id, int* metro_atual, int* pos_relativa);

void* ciclista(int* id);

typedef struct linha_t {
    int pos[NUM_FAIXAS];        // Em cada linha há NUM_FAIXAS posições possíveis
    pthread_mutex_t mutex_pos[NUM_FAIXAS];
    int n_ciclistas;
    pthread_mutex_t mutex_linha;
} linha;

typedef struct rank_t {
    int volta_final;                // Armazena a volta em que perderam ou quebraram
    int tempo_final;                // Instante de tempo que cruzou a linha de chegada
    int quebrou;                    // Indica se o ciclista quebrou
} rank;

#endif

