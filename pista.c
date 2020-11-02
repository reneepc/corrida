#include "pista.h"
#include "aleatorio.h"
#include <math.h>

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

linha* pista;
rank* ranking;
pthread_barrier_t* barreiras;

void uso() {
    fprintf(stderr, "Uso:\n");
    fprintf(stderr, "\t./ep2 <tamanho-da-pista> <numero-de-ciclistas>\n");
    exit(EXIT_FAILURE);
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

void* ciclista() {
    int eliminado = 0;
    int quebrou = 0;
    int vel = 8;
    int volta = 1;
    int metro_atual = 0;
    int pos_relativa = 0;
    double resto = 1.0;

    pthread_barrier_wait(&largada);
    // Cada ciclista eliminado deverá decrementar a variável
    // global n, utilizando o mutex_n. O qual também será
    // utilizado para controlar o acesso ao ranking antes 
    // de se alcançar a barreira.
    while(1) {
        if(turno == 0) {

            if(volta != 1 && metro_atual == 0) {
                vel = decide_velocidade(vel, ultimas);
                if(vel == 8) resto = 1.0;
                else if(vel == 16) resto = 2.0;
                else {
                    pthread_mutex_lock(&mutex_ultimas);
                    ultimas = 1;
                    pthread_mutex_unlock(&mutex_ultimas);
                }
            }
            if(metro_atual == 0 && volta % 6 == 0) {
                pthread_mutex_lock(&mutex_n);
                quebrou = decide_quebrou(n);
                if(quebrou) {
                    if(n != 0) {
                        n -= 1;
                    }
                    fprintf(stderr, "O ciclista quebrou");
                }
                pthread_mutex_unlock(&mutex_n);
            }

            pos_relativa += vel*intervalo + ceil((resto*intervalo)/3);
            if(pos_relativa >= 1000.0) {
                metro_atual += 1;
                pos_relativa = 0;
            }
            if(metro_atual == d) {
                volta += 1;
                metro_atual = 0;
                fprintf(stderr, "Thread: %ld Volta %d Velocidade: %d\n", pthread_self(), volta, vel);
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
            if(quebrou) break;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////TURNO ÍMPAR//////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        } else {
            if(volta != 1) {
                vel = decide_velocidade(vel, ultimas);
                if(vel == 8) resto = 1.0;
                else if(vel == 16) resto = 2.0;
                else resto = 0;
            }

            if(metro_atual == 0 && volta % 6 == 0) {
                pthread_mutex_lock(&mutex_n);
                quebrou = decide_quebrou(n);
                if(quebrou) {
                    if(n != 0) {
                        n -= 1;
                    }
                    fprintf(stderr, "O ciclista quebrou");
                }
                pthread_mutex_unlock(&mutex_n);
            }

            pos_relativa += vel*intervalo + ceil((resto*intervalo)/3);
            if(pos_relativa >= 1000.0) {
                metro_atual += 1;
                pos_relativa = 0;
            }
            if(metro_atual == d) {
                volta += 1;
                metro_atual = 0;
                fprintf(stderr, "Thread: %ld Volta %d Velocidade: %d\n", pthread_self(), volta, vel);
            }


            // Atualiza velocidade e estado, utilizando as funções aleatórias.
            //if(eliminado || quebrou) {
            //    // Encontrar alguma forma de controlar a aleatoriedade da destruição
            //    // das threads e de atualizar o ranking.
            //    pthread_mutex_lock(&mutex_n);
            //    n -= 1;
            //    pthread_mutex_unlock(&mutex_n);
            //}
            pthread_barrier_wait(&barr[1]);
            pthread_barrier_wait(&barr[1]);
            if(quebrou) break;
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

    // Inicia pista
    pista = calloc(d, sizeof(linha));
    for(int i = 0; i < d; i++) {
        for(int j = 0; j < NUM_FAIXAS; j++)
            pthread_mutex_init(&pista[i].mutex_pos[j], NULL);
    }

    pthread_barrier_init(&largada, NULL, n+1);

    // São criadas duas barreiras, as quais serão alternadas a cada 
    // turno da simulação. De modo que, enquanto as n threads ciclistas
    // restantes esperam uma barreira, a do outro turno é criada.
    pthread_barrier_init(&barr[0], NULL, n+1);
    pthread_barrier_init(&barr[1], NULL, n+1);

    // Distribui os ciclistas nas faixas 0, 2, 4, 6 e 8.
    pthread_t id_threads[n];
    int ids[n];
    for(int z = 0; z < n; z++) {
        ids[z] = z + 1;
        pthread_create(&id_threads[z], NULL, (void*)&ciclista, NULL);
        pista[2*z/NUM_FAIXAS].pos[2*z%NUM_FAIXAS] = z + 1;
        pista[2*z/NUM_FAIXAS].n_ciclistas += 1;
    }
    print_pista(pista);

    pthread_barrier_wait(&largada);
    while(1) {
        pthread_barrier_wait(&barr[turno]);
        pthread_barrier_destroy(&barr[!turno]);
        if(n == 0) exit(EXIT_SUCCESS);
        if(ultimas) intervalo = 20;
        // Aqui será feito todo o processamento da main.
        pthread_barrier_init(&barr[!turno], NULL, n+1);
        turno = !turno;
        pthread_barrier_wait(&barr[!turno]);
    }

    
    return(0);
}
