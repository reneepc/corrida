#include "ep2.h"
#include "aleatorio.h"
#include <math.h>
#include <time.h>

#define LOG_FD stderr

int d;
int args;
_Atomic int n;
int n_total;
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

int* elimina_id;
int* quebra_id;
_Atomic int* pos_volta;
pthread_mutex_t* mutex_volta;
int** ranking;

rank_final* ranking_final;
int pos_final;
pthread_mutex_t mutex_final;

void uso() {
    fprintf(stderr, "Uso:\n");
    fprintf(stderr, "\t./ep2 <tamanho-da-pista> <numero-de-ciclistas>\n");
    exit(EXIT_FAILURE);
}

void cria_ranking(int max_voltas, int n_total) {
    // Armazena a posição do ultimo corredor da volta;
    pos_volta = calloc(max_voltas + 1, sizeof(int));
    mutex_volta = calloc(max_voltas + 1, sizeof(pthread_mutex_t));
    // Armazena a colocação no id de cada corredor;
    ranking = calloc(max_voltas + 1, sizeof(int*));
    for(int i = 0; i < max_voltas + 1; i++) {
        pthread_mutex_init(&mutex_volta[i], NULL);
        ranking[i] = calloc(n_total, sizeof(int));
    }
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
        pthread_mutex_init(&pista[i].mutex_linha, NULL);
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

void atualiza_numero(int* pista_atual, int* prox_pista) {
    pista[*prox_pista].n_ciclistas++;
    pista[*pista_atual].n_ciclistas--;
}

int verifica_quebra() {
    int quebrou; 
    pthread_mutex_lock(&mutex_n);
    quebrou = decide_quebrou(n);
    pthread_mutex_unlock(&mutex_n);
    return quebrou;
}

int atualiza_velocidade(int* vel, int ultimas_voltas) {
    *vel = decide_velocidade(*vel, ultimas_voltas);
    if(*vel == 8) return 1.0;
    else if(*vel == 16) return 2.0;
    else {
        return 0.0;
    }
}

void atualiza_posicao(int* pista_atual, int* prox_pista, int* faixa_atual, int id, int* metro_atual, int* pos_relativa) {
    // Mutex do número de ciclistas
    pthread_mutex_lock(&pista[*pista_atual].mutex_linha);
    pthread_mutex_lock(&pista[*prox_pista].mutex_linha);
    // Mutex das posições
    pthread_mutex_lock(&pista[*prox_pista].mutex_pos[*faixa_atual]);
    pthread_mutex_lock(&pista[*pista_atual].mutex_pos[*faixa_atual]);
    // Atualização de posição para a posição da frente
    if(pista[*prox_pista].pos[*faixa_atual] == -1 && pista[*prox_pista].n_ciclistas < 5) {
        pista[*prox_pista].pos[*faixa_atual] = id;
        pista[*pista_atual].pos[*faixa_atual] = -1;

        atualiza_numero(pista_atual, prox_pista);

        pthread_mutex_unlock(&pista[*pista_atual].mutex_pos[*faixa_atual]);
        pthread_mutex_unlock(&pista[*prox_pista].mutex_pos[*faixa_atual]);
        pthread_mutex_unlock(&pista[*pista_atual].mutex_linha);
        pthread_mutex_unlock(&pista[*prox_pista].mutex_linha);

        *pista_atual = *prox_pista;
        *prox_pista = *pista_atual - 1;
        if(*prox_pista == -1) *prox_pista = d - 1;
        *metro_atual += 1;
        *pos_relativa = 0;
    // Ultrapassagem
    } else if(*faixa_atual < NUM_FAIXAS-1 && pista[*prox_pista].n_ciclistas < 5) {

        pthread_mutex_lock(&pista[*prox_pista].mutex_pos[*faixa_atual+1]);
        if(pista[*prox_pista].pos[*(faixa_atual)+1] == -1) {
            pista[*prox_pista].pos[*(faixa_atual)+1] = id;
            pista[*pista_atual].pos[*faixa_atual] = -1;

            atualiza_numero(pista_atual, prox_pista);

            pthread_mutex_unlock(&pista[*prox_pista].mutex_pos[*(faixa_atual)+1]);
            pthread_mutex_unlock(&pista[*pista_atual].mutex_pos[*faixa_atual]);
            pthread_mutex_unlock(&pista[*prox_pista].mutex_pos[*faixa_atual]);
            pthread_mutex_unlock(&pista[*pista_atual].mutex_linha);
            pthread_mutex_unlock(&pista[*prox_pista].mutex_linha);

            *pista_atual = *prox_pista;
            *prox_pista = *(pista_atual) - 1;
            *faixa_atual = *(faixa_atual) + 1;
            if(*prox_pista == -1) *prox_pista = d - 1;
            *metro_atual += 1;
            *pos_relativa = 0;
        } else {
            pthread_mutex_unlock(&pista[*prox_pista].mutex_pos[*(faixa_atual)+1]);
            pthread_mutex_unlock(&pista[*pista_atual].mutex_pos[*faixa_atual]);
            pthread_mutex_unlock(&pista[*prox_pista].mutex_pos[*faixa_atual]);
            pthread_mutex_unlock(&pista[*pista_atual].mutex_linha);
            pthread_mutex_unlock(&pista[*prox_pista].mutex_linha);
        }

    } else {
        pthread_mutex_unlock(&pista[*pista_atual].mutex_pos[*faixa_atual]);
        pthread_mutex_unlock(&pista[*prox_pista].mutex_pos[*faixa_atual]);
        pthread_mutex_unlock(&pista[*pista_atual].mutex_linha);
        pthread_mutex_unlock(&pista[*prox_pista].mutex_linha);
    }
}

void adiciona_colocacao(int id, int* volta) {
    pthread_mutex_lock(&mutex_volta[*volta]);
    pos_volta[*volta]++;
    ranking[*volta][id] = pos_volta[*volta];
    pthread_mutex_unlock(&mutex_volta[*volta]);
}

void remove_corredor(int* pista_atual, int*faixa_atual) {
    pthread_mutex_lock(&pista[*pista_atual].mutex_linha);
    pthread_mutex_lock(&pista[*pista_atual].mutex_pos[*faixa_atual]);
        pista[*pista_atual].n_ciclistas--;
        pista[*pista_atual].pos[*faixa_atual] = -1;
    pthread_mutex_unlock(&pista[*pista_atual].mutex_pos[*faixa_atual]);
    pthread_mutex_unlock(&pista[*pista_atual].mutex_linha);
}

void add_ranking_final_eliminado(int id, int tempo_total, int volta_final) {
    pthread_mutex_lock(&mutex_final);
        ranking_final[pos_final].id = id;
        ranking_final[pos_final].tempo_final = tempo_total;
        ranking_final[pos_final].volta_final = volta_final;

        pthread_mutex_lock(&mutex_n);
            ranking_final[pos_final].colocacao = n+1;
        pthread_mutex_unlock(&mutex_n);

        pos_final = pos_final + 1;
    pthread_mutex_unlock(&mutex_final);
}

void add_ranking_final_quebrado(int id, int tempo_total, int volta_final) {
    pthread_mutex_lock(&mutex_final);
        ranking_final[pos_final].id = id;
        ranking_final[pos_final].tempo_final = tempo_total;
        ranking_final[pos_final].volta_final = volta_final;
        ranking_final[pos_final].colocacao = -1;
        pos_final = pos_final + 1;
    pthread_mutex_unlock(&mutex_final);
}

void print_ranking_final(int n_total) {
    for(int i = 0; i < n_total; i++) {
        fprintf(stderr, "Colocação %d: Ciclista %d", ranking_final[i].colocacao, ranking_final[i].id);
        fprintf(stderr, " - Tempo Final: %ds", ranking_final[i].tempo_final);
        fprintf(stderr, " - Volta Final %d\n", ranking_final[i].volta_final);
    }
}

void* ciclista(int* id) {
    int eliminado = 0;
    int quebrou = 0;
    int vel = 8;
    int volta = 1;
    double tempo_total = 0.0;
    int metro_atual = 0;
    int pos_relativa = 0;
    double resto = 1.0;
    int faixa_atual = 2*(*id)%NUM_FAIXAS;
    int pista_atual = 2*(*id)/NUM_FAIXAS;
    int prox_pista = pista_atual - 1;
    if(prox_pista == -1) prox_pista = d - 1;

    pthread_barrier_wait(&largada);

    while(1) {

        if(turno == 0) {
            if(elimina_id[*id]) {
                fprintf(stderr, "O ciclista %d foi eliminado\n", *id);
                remove_corredor(&pista_atual, &faixa_atual);
                add_ranking_final_eliminado(*id, tempo_total, volta);
                //print_ranking_final(n_total);
                break;
            }
             if (n == 1){
                add_ranking_final_eliminado(*id, tempo_total, volta);
                return NULL;

            } 

            if(metro_atual == 0 && volta % 6 == 0) {
                //quebrou = verifica_quebra();
            }

            pos_relativa += vel*intervalo + ceil((resto*intervalo)/3);
            if(pos_relativa >= 1000.0) {
                atualiza_posicao(&pista_atual, &prox_pista, &faixa_atual, *id, &metro_atual, &pos_relativa);
            }
            if(metro_atual == d) {
                if(args == 5)
                    fprintf(LOG_FD, "Ciclista: %3d Volta %d Velocidade: %d\n", *id, volta, vel);

                if(volta <= max_voltas) {
                    adiciona_colocacao(*id, &volta);
                }
                volta += 1;
                metro_atual = 0;
            }

            if(volta != 1 && metro_atual == 0) {
                resto = atualiza_velocidade(&vel, max_voltas - 2 <= volta);
            }
            if(resto == 0 && ultimas == 0) {
                pthread_mutex_lock(&mutex_ultimas);
                ultimas = 1;
                pthread_mutex_unlock(&mutex_ultimas);
            }
            if(volta <= max_voltas) tempo_total += 0.001*intervalo;
            pthread_barrier_wait(&barr[0]);
            pthread_barrier_wait(&barr[0]);
//////////////////////////////////////////TURNO ÍMPAR//////////////////////////////////////////////////////////
        } else {
            if(elimina_id[*id]) {
                fprintf(stderr, "O ciclista %d foi eliminado\n", *id);
                remove_corredor(&pista_atual, &faixa_atual);
                add_ranking_final_eliminado(*id, tempo_total, volta);
                //print_ranking_final(n_total);
                break;
            }
             if (n == 1){
                add_ranking_final_eliminado(*id, tempo_total, volta);
                return NULL;

            } 

            if(metro_atual == 0 && volta % 6 == 0) {
                //quebrou = verifica_quebra();
            }
            pos_relativa += vel*intervalo + ceil((resto*intervalo)/3);
            if(pos_relativa >= 1000.0) {
                atualiza_posicao(&pista_atual, &prox_pista, &faixa_atual, *id, &metro_atual, &pos_relativa);
            }
            if(metro_atual == d) {
                if(args == 5)
                    fprintf(LOG_FD, "Thread: %3d Volta %d Velocidade: %d\n", *id, volta, vel);
                if(volta <= max_voltas) {
                    adiciona_colocacao(*id, &volta);
                }
                volta += 1;
                metro_atual = 0;
            }

            if(volta != 1 && metro_atual == 0) {
                resto = atualiza_velocidade(&vel, max_voltas - 2 <= volta);
            }
            if(resto == 0 && ultimas == 0) {
                pthread_mutex_lock(&mutex_ultimas);
                ultimas = 1;
                pthread_mutex_unlock(&mutex_ultimas);
            }
            if(volta <= max_voltas) tempo_total += 0.001*intervalo;
            pthread_barrier_wait(&barr[1]);
            pthread_barrier_wait(&barr[1]);
        }
    }
    return NULL;
}

void ajusta_ranking(int id, int colocacao, int volta_atual) {
    for(int i = volta_atual + 1; i <= max_voltas; i++) {
        ranking[i][id] = colocacao;
    }
}

void print_volta (int volta_atual) {
    for(int i = 0; i < n_total; i++) {
        fprintf(LOG_FD, "Ciclista %d: - Colocação: %d\n", i, ranking[volta_atual][i]);
    }
}


int main(int argc, char** argv) {
    if(argc < 3)
        uso();
        
    d = atoi(argv[1]);
    n = atoi(argv[2]);
    max_voltas = 2*(n-1) + 1;
    int turno_main;
    int volta_atual = 1;
    struct timespec tempo;
    args = argc;
    n_total = n;
    int eliminado = 0;
    ranking_final = calloc(n_total, sizeof(rank_final));
    pos_final = 0;

    elimina_id = calloc(n, sizeof(int));
    quebra_id = calloc(n, sizeof(int));

    pthread_mutex_init(&mutex_n, NULL);
    pthread_mutex_init(&mutex_ultimas, NULL);
    pthread_mutex_init(&mutex_final, NULL);

    cria_barreiras();
    pthread_t id_threads[n];
    cria_pista(id_threads);
    cria_ranking(max_voltas, n_total);

    pthread_barrier_wait(&largada);
    while(1) {
        pthread_barrier_wait(&barr[turno]);

        if(ultimas && intervalo == 60) {
            intervalo = 20;
            fprintf(stderr, "Alterando intervalo\n");
        }

        if(pos_volta[volta_atual] == n && n != 1) {
            fprintf(stderr, "Volta %d Completada\n", volta_atual);
            if(volta_atual == 175) exit(EXIT_SUCCESS);
            if(volta_atual % 2 == 0 && volta_atual != 1) {
                for(int i = 0; i < n_total; i++) {
                    fprintf(LOG_FD, "Ciclista %d: - Colocação: %d\n", i, ranking[volta_atual][i]);
                    if(ranking[volta_atual][i] == pos_volta[volta_atual]) {
                        ajusta_ranking(i, n, volta_atual);
                        elimina_id[i] = 1;
                        n = n - 1;
                    }
                }
            } else {
                print_volta(volta_atual);
            }
            volta_atual = volta_atual + 1;
        }

        if(n == 1) {
            printf("Acabou");
            print_ranking_final(n_total);
            break;
        }

        if(argc == 4) {
            tempo.tv_sec = 0;
            // Multiplica o intervalo por 1 000 000 para obter
            // o tempo em milisegundos.
            tempo.tv_nsec = intervalo * 1000000;
            nanosleep(&tempo, NULL);
            print_pista(pista);
        }

        pthread_barrier_destroy(&barr[!turno]);
        pthread_barrier_init(&barr[!turno], NULL, n+1);
        turno = !turno;
        pthread_barrier_wait(&barr[!turno]);
    }

    return(0);
}
