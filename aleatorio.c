#include "aleatorio.h"

// Retorna um double entre 0 e 1, o qual será utilizado para
// determinar a probabilidade de ocorrência de um evento.
double get_prob() {
    return (double) rand()/ (double) RAND_MAX;
}

// Recebe a velocidade anterior e um indicador booleano 
// que possui valor 1 se a corrida estiver nas últimas 
// duas voltas.
//
// Retorna a velocidade atual do ciclista baseado na sua
// velocidade na volta anterior, bem como se está nas últimas
// duas voltas ou não.
//
// Se a velocidade anterior for 30 o ciclista terá 80% de chance
// de ficar com 60 Km/h. Se for 60, terá 40% de chance de ficar 
// com 30 Km/h.
//
// Ainda, se estiver nas últimas duas voltas, o ciclista tem 10%
// de ficar com 90 Km/h.
//
// Caso a probabilidade não esteja no intervalo desejado, o ciclista
// continua com a mesma velocidade anterior.
int decide_velocidade(int vel_ant, int ultimas_voltas) {
    double probabilidade = get_prob();
    int vel_final = vel_ant;
    if(vel_ant == 30) {
        if(probabilidade <= 0.8)
            vel_final = 60;
    }
    if(vel_ant == 60) {
        if(probabilidade <= 0.4)
            vel_final = 30;
    }
    if(ultimas_voltas == 1) {
        if(probabilidade <= 0.1)
            vel_final = 90;
    }
    
    return vel_final;
}


// Recebe o número da volta atual, bem como o número
// restante de ciclistas na corrida.
//
// Retorna 1 se o ciclista quebrou, 0 caso contrário.
int quebrou(int volta, int restantes) {
    if(volta % 6 == 0 && restantes > 5) {
        double probabilidade = get_prob();
        return probabilidade <= 0.05;
    }
    return 0;
}
