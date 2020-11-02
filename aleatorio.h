#ifndef _ALEATORIO_H
#define _ALEATORIO_H

#include <stdlib.h>

double get_prob();

int decide_velocidade(int vel_ant, int ultimas_voltas);

int decide_quebrou(int volta, int restantes);

#endif

