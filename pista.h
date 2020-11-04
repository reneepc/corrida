#ifndef _VELODROMO_H
#define _VELODROMO_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define NUM_FAIXAS 10

void cria_pista(pthread_t id_threads[]);

void atualiza_numero(int* pista_atual, int* prox_pista);

int verifica_quebra();

int atualiza_velocidade(int* vel, int ultimas_voltas);

void atualiza_posicao(int* pista_atual, int* prox_pista, int* faixa_atual, int id, int* metro_atual, int* pos_relativa);

void cria_ranking(int max_voltas, int n_total);

void adiciona_colocacao(int id, int* volta);

void* ciclista(int* id);

typedef struct linha_t {
    int pos[NUM_FAIXAS];        // Em cada linha há NUM_FAIXAS posições possíveis
    pthread_mutex_t mutex_pos[NUM_FAIXAS];
    int n_ciclistas;
    pthread_mutex_t mutex_linha;
} linha;


typedef struct rank_final_t {
    int colocacao;
    int tempo_final;
    int volta_final;
    int id;
} rank_final;

#endif

