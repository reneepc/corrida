#include "pista.h"

#define N 5
#define TMAX 10000000000 /* 10s no máximo para comer e 10s no
                            máximo para pensar */
pthread_mutex_t forkV[N];

void * filosofoF (void * i) {
    struct timespec ts;
    int * eu = (int *) i;

    fprintf(stderr,"Sou o Filosofo %d\n",*eu);
   
    if (*eu != N-1)
        while (1) {
            pthread_mutex_lock(&forkV[*eu]);
            fprintf(stderr,"Pegou garfo %d: Filosofo %d\n", *eu, *eu);
            pthread_mutex_lock(&forkV[(*eu+1)%N]);
            fprintf(stderr,"Pegou garfo %d: Filosofo %d\n", (*eu+1)%N, *eu);

            /* Come */
            fprintf(stderr,"Comendo: Filosofo %d\n", *eu);
            ts.tv_sec=0;
            ts.tv_nsec=TMAX*random()/RAND_MAX;
            nanosleep(&ts,NULL);

            fprintf(stderr,"Devolveu garfo %d: Filosofo %d\n", *eu, *eu);
            pthread_mutex_unlock(&forkV[*eu]);
            fprintf(stderr,"Devolveu garfo %d: Filosofo %d\n", (*eu+1)%N, *eu);
            pthread_mutex_unlock(&forkV[(*eu+1)%N]);

            /* Pensando  */
            fprintf(stderr,"Pensando: Filosofo %d\n", *eu);
            ts.tv_sec=0;
            ts.tv_nsec=TMAX*random()/RAND_MAX;
            nanosleep(&ts,NULL);
        }
    else
        /* Mudando a ordem para o último filósofo */
        while (1) {
            pthread_mutex_lock(&forkV[(*eu+1)%N]);
            fprintf(stderr,"Pegou garfo %d: Filosofo %d\n", (*eu+1)%N, *eu);
            pthread_mutex_lock(&forkV[*eu]);
            fprintf(stderr,"Pegou garfo %d: Filosofo %d\n", *eu, *eu);

            /* Come */
            fprintf(stderr,"Comendo: Filosofo %d\n", *eu);
            ts.tv_sec=0;
            ts.tv_nsec=TMAX*random()/RAND_MAX;
            nanosleep(&ts,NULL);

            fprintf(stderr,"Devolveu garfo %d: Filosofo %d\n", (*eu+1)%N, *eu);
            pthread_mutex_unlock(&forkV[(*eu+1)%N]);
            fprintf(stderr,"Devolveu garfo %d: Filosofo %d\n", *eu, *eu);
            pthread_mutex_unlock(&forkV[*eu]);

            /* Pensando  */
            fprintf(stderr,"Pensando: Filosofo %d\n", *eu);
            ts.tv_sec=0;
            ts.tv_nsec=TMAX*random()/RAND_MAX;
            nanosleep(&ts,NULL);
        }

    return NULL;
}


//velodromo new_velodromo (int d, int n) {
  //  velodromo velod;
    //velod.tamanho = d;
    //velod.n_ciclistas = n;

    //linha* pista;
    // rank ranking;
    //return velod;
//}

ciclista *new_ciclista (int i) {
    ciclista *c = malloc(sizeof(ciclista));
    c->id = i;
    c->velocidade = 30;
    return c;
}

ciclista ***cria_pistas (int d, int n) {
    ciclista ***pistas = (ciclista ***) malloc (NUM_FAIXAS * sizeof(ciclista**));
    ciclista *cicli;

    for (int i = 0; i < NUM_FAIXAS; i ++) {
        pistas[i] = (ciclista **) malloc (d * sizeof(ciclista*));
    }

    for (int i = 0; i < NUM_FAIXAS; i++) {
        for (int j = 0; j < d; j++) {
            if (i * NUM_FAIXAS + j < n)
                cicli = new_ciclista(i * NUM_FAIXAS + j);
            else
                cicli = NULL;
            pistas[i][j] = cicli;
        }
    }

    return pistas;
}

int main() {
    int d = 250, n = 20;
    int i, ids[N];
    pthread_t filosofoT[N];
    pthread_t ciclistas[5];
    velodromo v;
    ciclista *cicli;
    ciclista ***pistas = cria_pistas(d, n);

    v.tamanho = d;
    v.n_ciclistas = n;



    return(0);
}
