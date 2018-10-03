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

char* paraMaiuscula(char* palavra);
int eHexadecimal(char* palavra);
int eDecimal(char* palavra);
int eNome(char* palavra);
int eDiretiva(char* palavra);
int eRotulo(char* palavra);
int eInstrucao(char* palavre);

int processarEntrada(char*, unsigned);
int emitirMapaDeMemoria();

#endif
