#ifndef MONTADOR_H
#define MONTADOR_H

#include "token.h"

/*
Exemplos de erros e definições...
#define ERROR_OFFSET -1000

enum errors
{
    ERR_HEXADECIMAL_NAO_VALIDO = ERROR_OFFSET,
    ERRO_ROTULO_JA_DEFINIDO,
    ERR_DIRETIVA_NAO_VALIDA,
    ERR_NUMERO_INVALIDO,
    ERR_NOME_INVALIDO,
};
*/

/* Exemplos de funções uteis

    //Retorna 1 se a palavra é um numero hexa ou 0 caso contrário
    int eHexadecimal(char* palavra)

    //Retorna 1 se a palavra é um numero Diretiva ou 0 caso contrário
    int eDiretiva(char* palavra)
*/

int processarEntrada(char*, unsigned);
int emitirMapaDeMemoria();

#endif
