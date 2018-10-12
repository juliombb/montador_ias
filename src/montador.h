// Júlio Moreira Blás de Barros RA 200491

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
void logErroGramatical(unsigned linha); // para ajudar na verificação de erros
void logErroMontagem();
int emitirMapaDeMemoria();

#endif
