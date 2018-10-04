#ifndef MONTADOR_H
#define MONTADOR_H

#include "token.h"

/*
 * Aloca uma nova palavra com todos
 * os caracteres do alfabeto maiusculos
 */
char* paraMaiuscula(char* palavra);

int eHexadecimal(char* palavra);
int eDecimal(char* palavra);
int eNome(char* palavra);
int eDiretiva(char* palavra);
int eRotulo(char* palavra);
int eInstrucao(char* palavre);

int processarEntrada(char*, unsigned);
/*int emitirMapaDeMemoria();*/

#endif
