#include "pista.h"

int d;
int n;
pthread_mutex_t mutex_n;
pthread_barrier_t barr[2];
int turno = 0;

int intervalo = 60; // Multiplicado pela velocidade, obtem-se a distância percorrida. Modificar a lógica das velocidades para m/s, 30 possui valor decimal, porém podemos tentar contornar utilizando uma variável que é incrementada até três a cada ciclo e depois dividida, quando a velocidade for igual a 30m/s.

linha* pista;
rank* ranking;
pthread_barrier_t* barreiras;

void uso() {
    fprintf(stderr, "Uso:\n");
    fprintf(stderr, "\t./ep2 <tamanho-da-pista> <numero-de-ciclistas>\n");
    exit(EXIT_FAILURE);
}

void ciclista(int vel_inicial, int voltas) {
    int eliminado = 0;
    int quebrou = 0;

    // Cada ciclista eliminado deverá decrementar a variável
    // global n, utilizando o mutex_n. O qual também será
    // utilizado para controlar o acesso ao ranking antes 
    // de se alcançar a barreira.
    while(1) {
        if(turno == 0) {
            // Atualiza velocidade e estado, utilizando as funções aleatórias.
            if(eliminado || quebrou) {
                // Encontrar alguma forma de controlar a aleatoriedade da destruição
                // das threads
                pthread_mutex_lock(&mutex_n);
                n -= 1;
                pthread_mutex_unlock(&mutex_n);
            }
            pthread_barrier_wait(&barr[0]);
            pthread_barrier_wait(&barr[0]);
        } else {
            // Atualiza velocidade e estado, utilizando as funções aleatórias.
            if(eliminado || quebrou) {
                // Encontrar alguma forma de controlar a aleatoriedade da destruição
                // das threads e de atualizar o ranking.
                pthread_mutex_lock(&mutex_n);
                n -= 1;
                pthread_mutex_unlock(&mutex_n);
            }
            pthread_barrier_wait(&barr[1]);
            pthread_barrier_wait(&barr[1]);
        }
    }
}

int main(int argc, char** argv) {

    if(argc != 3)
        uso();
        
    d = atoi(argv[1]);
    n = atoi(argv[2]);
    int turno_main;

    // Distribui os ciclistas nas faixas 0, 2, 4, 6 e 8.
    /*
    pista = malloc(d * sizeof(linha));
    for(int i = 0; i < 2*n_ciclistas; i += 2) {
        pista[i/NUM_FAIXAS].pos[i%NUM_FAIXAS].id = pthread_create
    }
    */


    // São criadas duas barreiras, as quais serão alternadas a cada 
    // turno da simulação. De modo que, enquanto as n threads ciclistas
    // restantes esperam uma barreira, a do outro turno é criada.
    pthread_barrier_init(&barr[0], NULL, n+1);
    pthread_barrier_init(&barr[1], NULL, n+1);


    while(1) {
        pthread_barrier_wait(&barr[turno]);
        pthread_barrier_destroy(&barr[!turno]);
        // Aqui será feito todo o processamento da main.
        pthread_barrier_init(&barr[!turno], NULL, n+1);
        turno_main = turno;
        turno = !turno;
        pthread_barrier_wait(&barr[turno_main]);
    }
    
    return(0);
}
