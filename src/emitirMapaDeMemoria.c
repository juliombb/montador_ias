#include "montador.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define ESQ 0
#define DIR 2

typedef struct DescSimbolo {
    char* nome;
    char definido; // 0 se foi definido ou 1 se não
    unsigned valor;
} DescSimbolo;

void adicionaInstrucao(char **linhaAtual, char pos, const char *maiuscula);
unsigned adicionaOuEncontra(DescSimbolo** simbolos, unsigned* tamSimbolos, char* simbolo);
unsigned define(DescSimbolo** simbolos, unsigned* tamSimbolos, char* simbolo, unsigned valor);

int trataInstrucao(
        DescSimbolo **simbolos, unsigned *tamSimbolos,
        char align, char **linhaAtual,
        unsigned *pos, Token *atual);

int trataDiretiva(
        DescSimbolo **simbolos, unsigned *tamSimbolos,
        char *align, char **linhaAtual,
        unsigned *pos, Token *atual, unsigned *numLinhaAtual);


/*
    ---- Voce deve implementar essa função para a Parte 2! ----
    Utilize os tokens da estrutura de tokens
    Retorna:
        *  1 caso haja erro na montagem;
        *  0 caso não haja erro.
 */
int emitirMapaDeMemoria()
{
    unsigned palavraMemoriaAtual = 0;
    char*** saida = (char***) malloc(1023* sizeof(char**));
    // vetor de vetor de strings. Cada linha do vetor é um vetor de 4 strings

    DescSimbolo** simbolos = (DescSimbolo**) malloc(1023* sizeof(DescSimbolo*));
    unsigned tamSimbolos = 0;

    // tabelinha

    char align = ESQ;

    char** linhaAtual = NULL; // minha IDE me enche o saco se eu não fizer isso
    unsigned numLinhaAtual = 0;

    for (unsigned pos = 0; pos < getNumberOfTokens(); pos++) {
        if (align == ESQ) {
            linhaAtual = (char**) malloc(4 * sizeof(char*)); // 4 strings
            linhaAtual[0] = (char*) malloc(2* sizeof(char)); // inst 1
            strcpy(linhaAtual[0], "00");
            linhaAtual[1] = (char*) malloc(3* sizeof(char)); // end 1
            strcpy(linhaAtual[1], "000");
            linhaAtual[2] = (char*) malloc(2* sizeof(char)); // inst 2
            strcpy(linhaAtual[2], "00");
            linhaAtual[3] = (char*) malloc(3* sizeof(char)); // end 2
            strcpy(linhaAtual[3], "000");
        }

        Token atual = recuperaToken(pos);
        switch (atual.tipo) {
            case Instrucao: {
                if (trataInstrucao(simbolos, &tamSimbolos, align, linhaAtual, &pos, &atual)) {
                    return 1;
                }
                break;
            }
            case Diretiva: {
                if (trataDiretiva(simbolos, &tamSimbolos, &align, linhaAtual, &pos, &atual, &numLinhaAtual)) {
                    return 1;
                }
                continue;
            }
            case DefRotulo: {
                if (define(simbolos, &tamSimbolos, atual.palavra, numLinhaAtual)) {
                    return 1;
                }
                continue;
            }
            // Os outros casos devem ter sido tratados ja
        }

        if (align == ESQ) {
            align = DIR;
        } else {
            align = ESQ;
            saida[numLinhaAtual] = linhaAtual;
            numLinhaAtual++;
        }
    }

    return 0;
}

int trataInstrucao(
    DescSimbolo **simbolos,
    unsigned int *tamSimbolos,
    char align,
    char **linhaAtual,
    unsigned int *pos,
    Token *atual
) {

    char* maiuscula = paraMaiuscula((*atual).palavra);

    adicionaInstrucao(linhaAtual, align, maiuscula);

    if (strcmp(maiuscula, "LDMQ") == 0
     || strcmp(maiuscula, "LSH") == 0
     || strcmp(maiuscula, "RSH") == 0) {

        strcpy(linhaAtual[align + 1], "000");
     } else {
         (*pos) = (*pos) + 1;

         Token prox = recuperaToken(*pos);
         switch(prox.tipo) {
             case Hexadecimal: {
                 if (strlen(prox.palavra) > 5) {
                     logErroGramatical(prox.linha);
                     // return 1;
                 }

                 sprintf(linhaAtual[align + 1], "%03x", strtol(prox.palavra, NULL, 16));
             }
             case Decimal: {
                 long num = strtol(prox.palavra, NULL, 10);
                 if (num > 65535) {
                     logErroGramatical(prox.linha);
                     return 1;
                 }

                 sprintf(linhaAtual[align + 1], "%03x", num);
             }
             case Nome: {
                 unsigned numsym = adicionaOuEncontra(simbolos, tamSimbolos, prox.palavra);
                 sprintf(linhaAtual[align + 1], "%03d", numsym);
             }
             // não deve haver outro caso
         }
     }
     return 0;
}

void adicionaInstrucao(char **linhaAtual, char pos, const char *maiuscula) {
    if (strcmp(maiuscula, "LDMQ") == 0) {
        strcpy(linhaAtual[pos], "0A");
    }
    if (strcmp(maiuscula, "LSH") == 0) {
        strcpy(linhaAtual[pos], "14");
    }
    if (strcmp(maiuscula, "RSH") == 0) {
        strcpy(linhaAtual[pos], "15");
    }
    if (strcmp(maiuscula, "LD") == 0) {
        strcpy(linhaAtual[pos], "01");
    }
    if (strcmp(maiuscula, "LDINV") == 0) {
        strcpy(linhaAtual[pos], "02");
    }
    if (strcmp(maiuscula, "LDABS") == 0) {
        strcpy(linhaAtual[pos], "03");
    }
    if (strcmp(maiuscula, "LDMQMX") == 0) {
        strcpy(linhaAtual[pos], "09");
    }
    if (strcmp(maiuscula, "STORE") == 0) {
        strcpy(linhaAtual[pos], "21");
    }
    if (strcmp(maiuscula, "JUMP") == 0) {
        strcpy(linhaAtual[pos], "JM"); // alias p/ mudar dps
    }
    if (strcmp(maiuscula, "JUMPL") == 0) {
        strcpy(linhaAtual[pos], "J+"); // alias p/ mudar dps
    }
    if (strcmp(maiuscula, "JUMPR") == 0) {
        strcpy(linhaAtual[pos], "J+");
    }
    if (strcmp(maiuscula, "ADD") == 0) {
        strcpy(linhaAtual[pos], "05");
    }
    if (strcmp(maiuscula, "ADDABS") == 0) {
        strcpy(linhaAtual[pos], "07");
    }
    if (strcmp(maiuscula, "SUB") == 0) {
        strcpy(linhaAtual[pos], "06");
    }
    if (strcmp(maiuscula, "SUBABS") == 0) {
        strcpy(linhaAtual[pos], "08");
    }
    if (strcmp(maiuscula, "MULT") == 0) {
        strcpy(linhaAtual[pos], "0B");
    }
    if (strcmp(maiuscula, "DIV") == 0) {
        strcpy(linhaAtual[pos], "0C");
    }
    if (strcmp(maiuscula, "STORAL") == 0) {
        strcpy(linhaAtual[pos], "ST");
    }
    if (strcmp(maiuscula, "STORAR") == 0) {
        strcpy(linhaAtual[pos], "ST");
    }
}


int trataDiretiva(
        DescSimbolo **simbolos,
        unsigned int *tamSimbolos,
        char *align,
        char **linhaAtual,
        unsigned int *pos,
        Token *atual,
        unsigned *numLinhaAtual
) {

    char* maiuscula = paraMaiuscula((*atual).palavra);

    (*pos) = (*pos) + 1;
    Token prox = recuperaToken(*pos);

    if (strcmp(maiuscula, ".ORG") == 0) {
        long num = 0;
        switch (prox.tipo) {
            case Decimal: {
                num = strtol(prox.palavra, NULL, 10);
                break;
            }
            case Hexadecimal: {
                num = strtol(prox.palavra, NULL, 16);
                break;
            }
            // sabemos que não será outro
        }

        (*numLinhaAtual) = num;
        (*align) = ESQ;
        return (*numLinhaAtual <= 1023);
    }

    if (strcmp(maiuscula, ".ALIGN") == 0) {
        // ja verificamos que eh decimal
        long num = strtol(prox.palavra, NULL, 10);

        do {
            (*numLinhaAtual) = (*numLinhaAtual) + 1;
        } while ((*numLinhaAtual) % num != 0);

        (*align) = ESQ;
        return (*numLinhaAtual <= 1023);
    }

    if (strcmp(maiuscula, ".WFILL") == 0) {
        strcpy(linhaAtual[*align], "0A");
    }

    if (strcmp(maiuscula, ".WORD") == 0) {
        if ((*align) == DIR) {
            logErroGramatical((*atual).linha);
            return 1;
        }

        switch (prox.tipo) {
            case Hexadecimal: {
                char * palavra = prox.palavra;
                unsigned ultimo = strlen(palavra)-1;

                // se o hexa nao esta preenchido, preenchemos
                if (ultimo < 11) {
                    palavra = (char*) malloc(12* sizeof(char));
                    for (unsigned i = 11; i > 11 - (ultimo-2); --i) {
                        palavra[i] = prox.palavra[ultimo + i - 11];
                    }
                    for (unsigned j = 11 - (ultimo-2); j >= 0; --j) {
                        palavra[j] = '0';
                    }

                    palavra[12] = '\0';
                }

                // preenchendo a linha
                for (size_t charAt = 11; charAt > 8; --charAt) {
                    linhaAtual[3][charAt] = palavra[charAt];
                }

                for (size_t charAt = 8; charAt > 6; --charAt) {
                    linhaAtual[3][charAt] = palavra[charAt];
                }

                for (size_t charAt = 6; charAt > 3; --charAt) {
                    linhaAtual[3][charAt] = palavra[charAt];
                }

                for (size_t charAt = 3; charAt > 1; --charAt) {
                    linhaAtual[3][charAt] = palavra[charAt];
                }

                (*numLinhaAtual) += 1;
                return (*numLinhaAtual) < 1023;
            }
            case Decimal:break;
            case Nome:break;
        }
    }

    if (strcmp(maiuscula, ".SET") == 0) {
        (*pos) = (*pos) + 1;
        Token arg = recuperaToken(*pos);
        long argVal;
        switch (arg.tipo) {
            case Decimal: {
                argVal = strtol(arg.palavra, NULL, 10);
                break;
            }
            case Hexadecimal: {
                argVal = strtol(arg.palavra, NULL, 16);
                break;
            }
            // sabemos que não será outro
        }

        (*numLinhaAtual) = (*numLinhaAtual) + 1;

        return define(simbolos, tamSimbolos, prox.palavra, argVal);
    }

    return 0;
}