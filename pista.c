#include "pista.h"
#include "aleatorio.h"
#include <math.h>

#define LOG_FD stdout

int d;
int n;
pthread_mutex_t mutex_n;
int max_voltas;
pthread_barrier_t largada;
pthread_barrier_t barr[2];
_Atomic int turno = 0; // Alterna as barreiras para permitir a criação e destruição das mesmas.

int intervalo = 60; // Multiplicado pela velocidade, obtem-se a distância percorrida.
int ultimas = 0; // Variável booleana que indica se está nas últimas duas voltas
pthread_mutex_t mutex_ultimas;

int* ids;
linha* pista;
rank* ranking;
pthread_barrier_t* barreiras;

void uso() {
    fprintf(stderr, "Uso:\n");
    fprintf(stderr, "\t./ep2 <tamanho-da-pista> <numero-de-ciclistas>\n");
    exit(EXIT_FAILURE);
}

void cria_barreiras() {
    pthread_barrier_init(&largada, NULL, n+1);

    // São criadas duas barreiras, as quais serão alternadas a cada 
    // turno da simulação. De modo que, enquanto as n threads ciclistas
    // restantes esperam uma barreira, a do outro turno é criada.
    pthread_barrier_init(&barr[0], NULL, n+1);
    pthread_barrier_init(&barr[1], NULL, n+1);
}

void cria_pista(pthread_t id_threads[]) {
    // Inicia pista
    pista = calloc(d, sizeof(linha));
    for(int i = 0; i < d; i++) {
        for(int j = 0; j < NUM_FAIXAS; j++) {
            pthread_mutex_init(&pista[i].mutex_pos[j], NULL);
            pista[i].pos[j] = -1;
        }
    }

    // Distribui os ciclistas nas faixas 0, 2, 4, 6 e 8.
    ids = malloc(n*sizeof(int));
    for(int z = 0; z < n; z++) {
        ids[z] = z;
        pthread_create(&id_threads[z], NULL, (void*)&ciclista, &ids[z]);
        pista[2*z/NUM_FAIXAS].pos[2*z%NUM_FAIXAS] = z;
        pista[2*z/NUM_FAIXAS].n_ciclistas += 1;
    }
}


void print_pista(linha* pista) {
    for(int i = 0; i < d; i++) {
        printf("Metro %4d: ", i);
        for(int j = 0; j < NUM_FAIXAS; j++)
            printf("%4d", pista[i].pos[j]);
        printf("  -> %d", pista[i].n_ciclistas);
        printf("\n");
    }
}

void atualiza_numero(int pista_atual, int prox_pista) {
    pthread_mutex_lock(&pista[prox_pista].mutex_linha);
    pthread_mutex_lock(&pista[pista_atual].mutex_linha);
    pista[prox_pista].n_ciclistas++;
    pista[pista_atual].n_ciclistas--;
    pthread_mutex_unlock(&pista[prox_pista].mutex_linha);
    pthread_mutex_unlock(&pista[pista_atual].mutex_linha);
}

int verifica_quebra() {
    int quebrou;
    pthread_mutex_lock(&mutex_n);
    quebrou = decide_quebrou(n);
    if(quebrou) {
        fprintf(LOG_FD, "O ciclista quebrou\n");
        //n -= 1;
        fprintf(LOG_FD, "%d restantes\n", n);
    }
    pthread_mutex_unlock(&mutex_n);
    return quebrou;
}

int atualiza_velocidade(int* vel) {
    *vel = decide_velocidade(*vel, ultimas);
    if(*vel == 8) return 1.0;
    else if(*vel == 16) return 2.0;
    else {
        pthread_mutex_lock(&mutex_ultimas);
        ultimas = 1;
        pthread_mutex_unlock(&mutex_ultimas);
        return 0.0;
    }
}

void atualiza_posicao(int* pista_atual, int* prox_pista, int* faixa_atual, int id, int* metro_atual, int* pos_relativa) {
    pthread_mutex_lock(&pista[*prox_pista].mutex_pos[*faixa_atual]);
    pthread_mutex_lock(&pista[*pista_atual].mutex_pos[*faixa_atual]);
    if(pista[*prox_pista].pos[*faixa_atual] == -1) {
        pista[*prox_pista].pos[*faixa_atual] = id;
        pista[*pista_atual].pos[*faixa_atual] = -1;

        atualiza_numero(*pista_atual, *prox_pista);

        pthread_mutex_unlock(&pista[*pista_atual].mutex_pos[*faixa_atual]);
        pthread_mutex_unlock(&pista[*prox_pista].mutex_pos[*faixa_atual]);

        *pista_atual = *prox_pista;
        *prox_pista = *pista_atual - 1;
        if(*prox_pista == -1) *prox_pista = d - 1;
        *metro_atual += 1;
        *pos_relativa = 0;
    } else {
        pthread_mutex_unlock(&pista[*pista_atual].mutex_pos[*faixa_atual]);
        pthread_mutex_unlock(&pista[*prox_pista].mutex_pos[*faixa_atual]);
    }
}

void* ciclista(int* id) {
    int eliminado = 0;
    int quebrou = 0;
    int vel = 8;
    int volta = 1;
    int metro_atual = 0;
    int pos_relativa = 0;
    double resto = 1.0;
    int faixa_atual = 2*(*id)%NUM_FAIXAS;
    int pista_atual = 2*(*id)/NUM_FAIXAS;
    int prox_pista = pista_atual - 1;
    if(prox_pista == -1) prox_pista = d - 1;

    pthread_barrier_wait(&largada);
    // Cada ciclista eliminado deverá decrementar a variável
    // global n, utilizando o mutex_n. O qual também será
    // utilizado para controlar o acesso ao ranking antes 
    // de se alcançar a barreira.
    while(1) {
        if(turno == 0) {

            if(volta != 1 && metro_atual == 0) {
                resto = atualiza_velocidade(&vel);
            }
            if(metro_atual == 0 && volta % 6 == 0) {
                quebrou = verifica_quebra();
            }

            pos_relativa += vel*intervalo + ceil((resto*intervalo)/3);
            if(pos_relativa >= 1000.0) {
                atualiza_posicao(&pista_atual, &prox_pista, &faixa_atual, *id, &metro_atual, &pos_relativa);
            }
            if(metro_atual == d) {
                volta += 1;
                metro_atual = 0;
                fprintf(LOG_FD, "Thread: %3d Volta %d Velocidade: %d\n", *id, volta, vel);
            }

            //// Atualiza velocidade e estado, utilizando as funções aleatórias.
            //if(eliminado || quebrou) {
            //    // Encontrar alguma forma de controlar a aleatoriedade da destruição
            //    // das threads
            //    pthread_mutex_lock(&mutex_n);
            //    n -= 1;
            //    pthread_mutex_unlock(&mutex_n);
            //}
            pthread_barrier_wait(&barr[0]);
            pthread_barrier_wait(&barr[0]);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////TURNO ÍMPAR//////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        } else {

            if(volta != 1 && metro_atual == 0) {
                resto = atualiza_velocidade(&vel);
            }
            if(metro_atual == 0 && volta % 6 == 0) {
                quebrou = verifica_quebra();
            }

            pos_relativa += vel*intervalo + ceil((resto*intervalo)/3);

            if(pos_relativa >= 1000.0) {
                atualiza_posicao(&pista_atual, &prox_pista, &faixa_atual, *id, &metro_atual, &pos_relativa);
            }

            if(metro_atual == d) {
                volta += 1;
                metro_atual = 0;
                fprintf(LOG_FD, "Thread: %3d Volta %d Velocidade: %d\n", *id, volta, vel);
            }

            pthread_barrier_wait(&barr[1]);
            pthread_barrier_wait(&barr[1]);
        }
    }
    return NULL;
}
int main(int argc, char** argv) {
    if(argc != 3)
        uso();
        
    d = atoi(argv[1]);
    n = atoi(argv[2]);
    max_voltas = 2*(n-1);
    int turno_main;

    pthread_mutex_init(&mutex_n, NULL);
    pthread_mutex_init(&mutex_ultimas, NULL);

    cria_barreiras();
    pthread_t id_threads[n];
    cria_pista(id_threads);



    pthread_barrier_wait(&largada);
    while(1) {
        pthread_barrier_wait(&barr[turno]);
        print_pista(pista);
        if(n == 0) exit(EXIT_SUCCESS);
        if(ultimas) intervalo = 20;

        pthread_barrier_destroy(&barr[!turno]);
        pthread_barrier_init(&barr[!turno], NULL, n+1);
        turno = !turno;
        pthread_barrier_wait(&barr[!turno]);
    }

    
    return(0);
}
