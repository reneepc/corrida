#include "pista.h"
#include "aleatorio.h"
#include <math.h>

int d;
int n;
int n_inicial;
int tempo; // Cronometro de quanto tempo se passou
int count; // Contagem de quantas threads ja foram processadas, usado para definir quem sera eliminado.
int total_quebras = 0; // Variavel para debug

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


// Armazena 1 caso o ciclista tenha quebrado, e 0 em caso constrario.

void atualiza_ranking (int n, int volta, int quebrou, long thread) {
    ranking[n].volta_final = volta;
    ranking[n].tempo_final = tempo;
    ranking[n].quebrou = quebrou;
    ranking[n].ciclista = thread;

}

// Imprime o ranqueamento dos ciclistas em ordem decrescente. 
// A funcao esta com um bug: imprime x saidas adicionais que estao
// completamente zeradas, fazendo com que o ranking va de 
// [-x, (n - quebrados - x)]. As tentativas de nao imprimir as linhas zeradas 
// fizeram com que os valores reais de alguns ciclistas fossem perdidos, entao optamos
// pela impressao com linhas adicionais zeradas.

void print_ranking() {
    fprintf(stderr, "\nRANKING\n");
    int rank = n_inicial - total_quebras;
    for (int i = n_inicial + 1; i > 0; i--) {
        if (!ranking[i].quebrou ) {
            
            fprintf(stderr, "#%d Ciclista %ld, cruzou a linha de chegada em %d.\n", rank, ranking[i].ciclista, ranking[i].tempo_final);
            rank--;
            
        }
    }


    fprintf(stderr, "\nQUEBRAS\n");
    for (int i = n_inicial; i > 0; i--) {
        if (ranking[i].quebrou == 1)
            fprintf(stderr, "Ciclista %ld quebrou na volta %d.\n", ranking[i].ciclista, ranking[i].volta_final);
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
        // as vezes o sistema de eliminacao pula algumas das rodadas pares. Corrigir isso.
        if (metro_atual == 0 && volta % 2 == 0 && volta > 1) {
            pthread_mutex_lock(&mutex_n);
            count += 1;
            if (count >= n) {
                printf("O ciclista %ld foi eliminado. \n", pthread_self());
                atualiza_ranking(n, volta, 0, pthread_self());
                n -= 1;

                count = 0;
                eliminado = 1;
            }

            else if (volta % 6 == 0) {
                quebrou = decide_quebrou(n);
                if(quebrou) {
                    n -= 1;
                    atualiza_ranking(n, volta, 1, pthread_self());
                    fprintf(stderr, "O ciclista %ld quebrou. \n", pthread_self());
                    total_quebras += 1;
                }
            }

            pthread_mutex_unlock(&mutex_n);

            if (n == 1) {
                pthread_mutex_lock(&mutex_n);
                atualiza_ranking(n, volta, 0, pthread_self());
                pthread_mutex_unlock(&mutex_n);
            }
        }


            pos_relativa += vel*intervalo + ceil((resto*intervalo)/3);
            if(pos_relativa >= 1000.0) {
                metro_atual += 1;
                pos_relativa = 0;
            }

            if(metro_atual == d) {
                volta += 1;
                metro_atual = 0;
                fprintf(stderr, "Thread: %ld Volta %d Velocidade: %d, %d, %d\n", pthread_self(), volta, vel, n, count);
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
            
            if(quebrou || eliminado) break;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////TURNO ÍMPAR//////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        } else {


            if (n == 1)
                atualiza_ranking(n, volta, 0, pthread_self());


            if(volta != 1) {
                vel = decide_velocidade(vel, ultimas);
                if(vel == 8) resto = 1.0;
                else if(vel == 16) resto = 2.0;
                else resto = 0;
            }


            pos_relativa += vel*intervalo + ceil((resto*intervalo)/3);
            if(pos_relativa >= 1000.0) {
                metro_atual += 1;
                pos_relativa = 0;
            }
            if(metro_atual == d) {
                volta += 1;
                metro_atual = 0;
                fprintf(stderr, "Thread: %ld Volta %d Velocidade: %d, %d, %d\n", pthread_self(), volta, vel, n, count);
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
            if(quebrou || eliminado) break;
        }
    }
    return NULL;
}

int main(int argc, char** argv) {

    if(argc != 3)
        uso();
        
    d = atoi(argv[1]);
    n = atoi(argv[2]);
    n_inicial = n;
    max_voltas = 2*(n-1);
    tempo = 0;

    int turno_main;

    pthread_mutex_init(&mutex_n, NULL);
    pthread_mutex_init(&mutex_ultimas, NULL);

    // Inicia pista
    pista = calloc(d, sizeof(linha));
    for(int i = 0; i < d; i++) {
        for(int j = 0; j < NUM_FAIXAS; j++)
            pthread_mutex_init(&pista[i].mutex_pos[j], NULL);
    }

    // Inicia ranking
    ranking = calloc (n, sizeof(rank));

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

    count = 0;
    pthread_barrier_wait(&largada);
    while(1) {
        pthread_barrier_wait(&barr[turno]);
        pthread_barrier_destroy(&barr[!turno]);
        if(n == 0) exit(EXIT_SUCCESS);
        if(ultimas) intervalo = 20;
        tempo += intervalo;
        // Aqui será feito todo o processamento da main.
        pthread_barrier_init(&barr[!turno], NULL, n+1);
        turno = !turno;
        pthread_barrier_wait(&barr[!turno]);
        if (n == 0)
            print_ranking();
    }

    fprintf(stderr, "%d", tempo);
    return(0);
}