#ifndef _VELODROMO_H
#define _VELODROMO_H

#define NUM_FAIXAS 10

typedef struct ciclista_t {
	int id;
	int velocidade;	

} ciclista;

typedef struct linha_t {
    ciclista ciclistas[NUM_FAIXAS];
} linha;

typedef struct rank_t {
    int posicao_final; // No caso de quebra armazena a volta.
    int tempo_final;
    int quebrou;
} rank;

typedef struct velodromo_t {
    int tamanho;
    int n_ciclistas;
    linha* pista;
    //rank ranking;
    
} velodromo;


#endif

