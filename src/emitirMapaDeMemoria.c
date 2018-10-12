#include "montador.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define ESQ 0
#define DIR 2

#define PADRAO_NOME "n%02x"
#define PADRAO_WORD "w%02x"

typedef enum TipoSimbolo {Simbolo, Rotulo} TipoSimbolo;
typedef struct DescNome {
    char* nome;
    char definido; // 0 se foi definido ou 1 se não
    unsigned char align;
    TipoSimbolo tipo;
    unsigned valor;
} DescNome;

void adicionaInstrucao(char **linhaAtual, unsigned char align, const char *maiuscula);
unsigned adicionaOuEncontra(DescNome** nomes, unsigned* qtosNomes, char* simbolo);
int define(DescNome** nomes, unsigned* qtosNomes,
           char* simbolo, unsigned valor, unsigned char align, TipoSimbolo tipo);

int preencheSeNecessario(char **linhaAtual, unsigned char align, DescNome** nomes, unsigned qtosNomes);

int trataInstrucao(
        DescNome **nomes, unsigned *qtosNomes,
        unsigned char align, char **linhaAtual,
        unsigned *pos, Token *atual);

int trataDiretiva(
        DescNome **nomes, unsigned *qtosNomes,
        unsigned char *align, char **linhaAtual,
        unsigned *pos, Token *atual, unsigned *numLinhaAtual,
        char ***saida);


char **alocaLinha();

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
    char*** saida = (char***) malloc(1024* sizeof(char**));
    // vetor de vetor de strings. Cada linha do vetor é um vetor de 4 strings, se preenchida
    for (int i = 0; i < 1024; ++i) {
        saida[i] = NULL;
    }

    DescNome** nomes = (DescNome**) malloc(1024* sizeof(DescNome*));
    unsigned qtosNomes = 0;

    // tabelinha

    unsigned char align = ESQ;

    char** linhaAtual = NULL; // minha IDE me enche o saco se eu não fizer isso
    unsigned numLinhaAtual = 0;

    for (unsigned pos = 0; pos < getNumberOfTokens(); pos++) {
        if (align == ESQ) {
            linhaAtual = alocaLinha();
        }

        Token atual = recuperaToken(pos);
        switch (atual.tipo) {
            case Instrucao: {
                if (trataInstrucao(nomes, &qtosNomes, align, linhaAtual, &pos, &atual)) {
                    return 1;
                }
                break;
            }
            case Diretiva: {
                if (trataDiretiva(nomes, &qtosNomes, &align, linhaAtual,
                                  &pos, &atual, &numLinhaAtual, saida)) {
                    return 1;
                }
                continue;
            }
            case DefRotulo: {
                if (define(nomes, &qtosNomes, atual.palavra, numLinhaAtual, align, Rotulo)) {
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

    // segunda sondagem
    // aqui substituimos os rotulos e simbolos e as respectivas funcoes

    for (numLinhaAtual = 0; numLinhaAtual < 1024; ++numLinhaAtual) {
        linhaAtual = saida[numLinhaAtual];
        if (linhaAtual == NULL) {
            continue;
        }

        if (linhaAtual[0][0] == 'w') { // é uma word
            unsigned pos;
            sscanf(linhaAtual[3], PADRAO_WORD, &pos);

            if (pos >= qtosNomes) {
                logErroMontagem();
                return 1;
            }

            DescNome *desc = nomes[pos];
            if (desc->definido == 0) {
                fprintf(stderr, "ERRO: Usado mas não definido: %s", desc->nome);
                return 1;
            }

            // preenchendo a palavra (no caso de ser word)
            char *palavra = (char*) malloc(11* sizeof(char));
            sprintf(palavra, "%010x", desc->valor);

            for (size_t charAt = 9; charAt > 6; --charAt) {
                linhaAtual[3][charAt - 7] = palavra[charAt];
            }

            for (size_t charAt = 6; charAt > 4; --charAt) {
                linhaAtual[2][charAt - 5] = palavra[charAt];
            }

            for (size_t charAt = 4; charAt > 1; --charAt) {
                linhaAtual[1][charAt - 2] = palavra[charAt];
            }

            for (size_t charAt = 1; charAt >= 0; --charAt) {
                linhaAtual[0][charAt] = palavra[charAt];
            }

            continue;
        }

        if (preencheSeNecessario(linhaAtual, ESQ, nomes, qtosNomes)) {
            return 1;
        }

        if (preencheSeNecessario(linhaAtual, DIR, nomes, qtosNomes)) {
            return 1;
        }
    }

    for (numLinhaAtual = 0; numLinhaAtual < 1024; ++numLinhaAtual) {
        linhaAtual = saida[numLinhaAtual];

        if (linhaAtual == NULL) {
            continue;
        }

        printf("%03x %s %s %s %s",
                numLinhaAtual, linhaAtual[0], linhaAtual[1], linhaAtual[2], linhaAtual[3]);
    }

    return 0;
}

char **alocaLinha() {
    char **linhaAtual = (char**) malloc(4 * sizeof(char*)); // 4 strings

    linhaAtual[0] = (char*) malloc(2* sizeof(char)); // inst 1
    strcpy(linhaAtual[0], "00");
    linhaAtual[1] = (char*) malloc(3* sizeof(char)); // end 1
    strcpy(linhaAtual[1], "000");
    linhaAtual[2] = (char*) malloc(2* sizeof(char)); // inst 2
    strcpy(linhaAtual[2], "00");
    linhaAtual[3] = (char*) malloc(3* sizeof(char)); // end 2
    strcpy(linhaAtual[3], "000");

    return linhaAtual;
}

int trataInstrucao(
    DescNome **nomes,
    unsigned int *qtosNomes,
    unsigned char align,
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
                     return 1;
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
                 unsigned numsym = adicionaOuEncontra(nomes, qtosNomes, prox.palavra);
                 sprintf(linhaAtual[align + 1], PADRAO_NOME, numsym);
             }
             // não deve haver outro caso
         }
     }
     return 0;
}

void adicionaInstrucao(char **linhaAtual, unsigned char align, const char *maiuscula) {
    if (strcmp(maiuscula, "LDMQ") == 0) {
        strcpy(linhaAtual[align], "0A");
    }
    if (strcmp(maiuscula, "LSH") == 0) {
        strcpy(linhaAtual[align], "14");
    }
    if (strcmp(maiuscula, "RSH") == 0) {
        strcpy(linhaAtual[align], "15");
    }
    if (strcmp(maiuscula, "LD") == 0) {
        strcpy(linhaAtual[align], "01");
    }
    if (strcmp(maiuscula, "LDINV") == 0) {
        strcpy(linhaAtual[align], "02");
    }
    if (strcmp(maiuscula, "LDABS") == 0) {
        strcpy(linhaAtual[align], "03");
    }
    if (strcmp(maiuscula, "LDMQMX") == 0) {
        strcpy(linhaAtual[align], "09");
    }
    if (strcmp(maiuscula, "STORE") == 0) {
        strcpy(linhaAtual[align], "21");
    }
    if (strcmp(maiuscula, "JUMP") == 0) {
        strcpy(linhaAtual[align], "JM"); // alias p/ mudar dps
    }
    if (strcmp(maiuscula, "JUMPL") == 0) {
        strcpy(linhaAtual[align], "J+"); // alias p/ mudar dps
    }
    if (strcmp(maiuscula, "JUMPR") == 0) {
        strcpy(linhaAtual[align], "J+");
    }
    if (strcmp(maiuscula, "ADD") == 0) {
        strcpy(linhaAtual[align], "05");
    }
    if (strcmp(maiuscula, "ADDABS") == 0) {
        strcpy(linhaAtual[align], "07");
    }
    if (strcmp(maiuscula, "SUB") == 0) {
        strcpy(linhaAtual[align], "06");
    }
    if (strcmp(maiuscula, "SUBABS") == 0) {
        strcpy(linhaAtual[align], "08");
    }
    if (strcmp(maiuscula, "MULT") == 0) {
        strcpy(linhaAtual[align], "0B");
    }
    if (strcmp(maiuscula, "DIV") == 0) {
        strcpy(linhaAtual[align], "0C");
    }
    if (strcmp(maiuscula, "STORAL") == 0) {
        strcpy(linhaAtual[align], "ST");
    }
    if (strcmp(maiuscula, "STORAR") == 0) {
        strcpy(linhaAtual[align], "ST");
    }
}

int deveArmazenarAntesDeTrocar(char **linhaAtual, unsigned int pos) {
    // essa funcao nos diz se devemos ter uma
    // linha armazenada quando uma diretiva for trocá-la

    if (strcmp(linhaAtual[0], "00") == 0
        && strcmp(linhaAtual[1], "000") == 0
        && strcmp(linhaAtual[2], "00") == 0
        && strcmp(linhaAtual[3], "000") == 0) {

        return 0;
    }

    return 1;
}

int trataDiretiva(
        DescNome **nomes,
        unsigned int *qtosNomes,
        unsigned char *align,
        char **linhaAtual,
        unsigned int *pos,
        Token *atual,
        unsigned *numLinhaAtual,
        char ***saida
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

        if (deveArmazenarAntesDeTrocar(linhaAtual, (*pos - 1))) {
            saida[*numLinhaAtual] = linhaAtual;
        } else {
            free(linhaAtual);
        }

        (*numLinhaAtual) = num;
        (*align) = ESQ;
        return (*numLinhaAtual <= 1023);
    }

    if (strcmp(maiuscula, ".ALIGN") == 0) {
        // ja verificamos que eh decimal
        long num = strtol(prox.palavra, NULL, 10);

        if (*align == ESQ && (*numLinhaAtual % num == 0)) {
            return 0;
        }

        if (deveArmazenarAntesDeTrocar(linhaAtual, (*pos - 1))) {
            saida[*numLinhaAtual] = linhaAtual;
        } else {
            free(linhaAtual);
        }

        do {
            (*numLinhaAtual) = (*numLinhaAtual) + 1;
        } while ((*numLinhaAtual) % num != 0);

        (*align) = ESQ;
        return (*numLinhaAtual <= 1023);
    }

    if (strcmp(maiuscula, ".WFILL") == 0) {
        if ((*align) == DIR) {
            logErroMontagem();
            return 1;
        }

        (*pos) += 1;
        Token valor = recuperaToken(*pos);

        switch (valor.tipo) {
            case Hexadecimal: {
                char * palavra = valor.palavra;
                unsigned ultimo = strlen(palavra)-1;

                // se o hexa nao esta preenchido, preenchemos
                if (ultimo < 11) {
                    palavra = (char*) malloc(13* sizeof(char));
                    sprintf(palavra, "0x%010x", strtol(valor.palavra, NULL, 16));
                }

                // prox.palavra eh o numero de linhas
                for (int i = 0; i < strtol(prox.palavra, NULL, 10); ++i) {
                    // preenchendo a linha
                    for (size_t charAt = 11; charAt > 8; --charAt) {
                        linhaAtual[3][charAt-9] = palavra[charAt];
                    }

                    for (size_t charAt = 8; charAt > 6; --charAt) {
                        linhaAtual[2][charAt-7] = palavra[charAt];
                    }

                    for (size_t charAt = 6; charAt > 3; --charAt) {
                        linhaAtual[1][charAt-4] = palavra[charAt];
                    }

                    for (size_t charAt = 3; charAt > 1; --charAt) {
                        linhaAtual[0][charAt-2] = palavra[charAt];
                    }

                    saida[*numLinhaAtual] = linhaAtual;
                    (*numLinhaAtual) += 1;

                    if ((*numLinhaAtual) > 1023) {
                        return 1;
                    }

                    linhaAtual = alocaLinha();
                }
                break;
            }
            case Decimal: {
                long argVal = strtol(valor.palavra, NULL, 10);
                char* palavra = (char*) malloc(10* sizeof(char));
                sprintf(palavra, "%010x", argVal);

                for (int i = 0; i < strtol(prox.palavra, NULL, 10); ++i) {
                    // preenchendo a linha
                    for (size_t charAt = 9; charAt > 6; --charAt) {
                        linhaAtual[3][charAt - 7] = palavra[charAt];
                    }

                    for (size_t charAt = 6; charAt > 4; --charAt) {
                        linhaAtual[2][charAt - 5] = palavra[charAt];
                    }

                    for (size_t charAt = 4; charAt > 1; --charAt) {
                        linhaAtual[1][charAt - 2] = palavra[charAt];
                    }

                    for (size_t charAt = 1; charAt >= 0; --charAt) {
                        linhaAtual[0][charAt] = palavra[charAt];
                    }
                    saida[*numLinhaAtual] = linhaAtual;
                    (*numLinhaAtual) += 1;

                    if ((*numLinhaAtual) > 1023) {
                        return 1;
                    }

                    linhaAtual = alocaLinha();
                }
                break;
            }
            case Nome: {
                unsigned numsym = adicionaOuEncontra(nomes, qtosNomes, prox.palavra);
                for (int i = 0; i < strtol(prox.palavra, NULL, 10); ++i) {
                    linhaAtual[0][0] = 'w';
                    sprintf(linhaAtual[3], PADRAO_WORD, numsym);
                    // w000000w12 é uma word que deve ser preenchida com o nome 12

                    saida[*numLinhaAtual] = linhaAtual;
                    (*numLinhaAtual) += 1;

                    if ((*numLinhaAtual) > 1023) {
                        return 1;
                    }

                    linhaAtual = alocaLinha();
                }
            }
        }

    }

    if (strcmp(maiuscula, ".WORD") == 0) {
        if ((*align) == DIR) {
            logErroMontagem();
            return 1;
        }

        switch (prox.tipo) {
            case Hexadecimal: {
                char * palavra = prox.palavra;
                unsigned ultimo = strlen(palavra)-1;

                // se o hexa nao esta preenchido, preenchemos
                if (ultimo < 11) {
                    palavra = (char*) malloc(13* sizeof(char));
                    sprintf(palavra, "0x%010x", strtol(prox.palavra, NULL, 16));
                }

                // preenchendo a linha
                for (size_t charAt = 11; charAt > 8; --charAt) {
                    linhaAtual[3][charAt-9] = palavra[charAt];
                }

                for (size_t charAt = 8; charAt > 6; --charAt) {
                    linhaAtual[2][charAt-7] = palavra[charAt];
                }

                for (size_t charAt = 6; charAt > 3; --charAt) {
                    linhaAtual[1][charAt-4] = palavra[charAt];
                }

                for (size_t charAt = 3; charAt > 1; --charAt) {
                    linhaAtual[0][charAt-2] = palavra[charAt];
                }
                break;
            }
            case Decimal: {
                long argVal = strtol(prox.palavra, NULL, 10);
                char* palavra = (char*) malloc(10* sizeof(char));
                sprintf(palavra, "%010x", argVal);

                // preenchendo a linha
                for (size_t charAt = 9; charAt > 6; --charAt) {
                    linhaAtual[3][charAt - 7] = palavra[charAt];
                }

                for (size_t charAt = 6; charAt > 4; --charAt) {
                    linhaAtual[2][charAt - 5] = palavra[charAt];
                }

                for (size_t charAt = 4; charAt > 1; --charAt) {
                    linhaAtual[1][charAt - 2] = palavra[charAt];
                }

                for (size_t charAt = 1; charAt >= 0; --charAt) {
                    linhaAtual[0][charAt] = palavra[charAt];
                }
                break;
            }
            case Nome: {
                linhaAtual[0][0] = 'w';
                unsigned numsym = adicionaOuEncontra(nomes, qtosNomes, prox.palavra);
                sprintf(linhaAtual[3], PADRAO_WORD, numsym);
            }
        }

        saida[*numLinhaAtual] = linhaAtual;
        (*numLinhaAtual) += 1;
        return (*numLinhaAtual) <= 1023;
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

        return define(nomes, qtosNomes, prox.palavra, argVal, ESQ, Simbolo);
    }

    return 0;
}

unsigned adicionaOuEncontra(DescNome** nomes, unsigned* qtosNomes, char* simbolo) {
    for (unsigned i = 0; i < *qtosNomes; ++i) {
        if (strcmp(nomes[i]->nome, simbolo) == 0) {
            return i;
        }
    }
    DescNome *desc = (DescNome*) malloc(sizeof(DescNome));
    desc->nome = simbolo;
    desc->definido = 0;
    nomes[*qtosNomes] = desc;

    (*qtosNomes) += 1;

    return (*qtosNomes) - 1;
}

int define(DescNome** nomes, unsigned* qtosNomes,
           char* simbolo, unsigned valor, unsigned char align, TipoSimbolo tipo) {
    unsigned pos = adicionaOuEncontra(nomes, qtosNomes, simbolo);
    DescNome* desc = nomes[pos];

    if (desc->definido) {
        logErroMontagem();
        return 1;
    }

    desc->definido = 1;
    desc->tipo = tipo;
    desc->valor = valor;
    desc->align = align;

    return 0;
}

int preencheSeNecessario(char** linhaAtual, unsigned char align, DescNome** nomes, unsigned qtosNomes) {
    if (strcmp(linhaAtual[align], "JM") == 0
        || strcmp(linhaAtual[align], "J+") == 0
        || strcmp(linhaAtual[align], "ST") == 0) {

        unsigned pos;
        sscanf(linhaAtual[align+1], PADRAO_NOME, &pos);

        if (pos >= qtosNomes) {
            logErroMontagem();
            return 1;
        }

        DescNome *desc = nomes[pos];
        if (desc->definido == 0) {
            fprintf(stderr, "ERRO: Usado mas não definido: %s", desc->nome);
            return 1;
        }

        if (desc->tipo == Simbolo && desc->valor > 1023) {
            logErroMontagem();
            return 1;
        }

        sprintf(linhaAtual[align+1], "%03x", desc->valor);

        if (strcmp(linhaAtual[align], "JM") == 0) {
            if (desc->align == ESQ) {
                strcpy(linhaAtual[align], "0D");
            } else {
                // desc->align = DIR
                strcpy(linhaAtual[align], "0E");
            }
        }

        else if (strcmp(linhaAtual[align], "J+") == 0) {
            if (desc->align == ESQ) {
                strcpy(linhaAtual[align], "0F");
            } else {
                // desc->align = DIR
                strcpy(linhaAtual[align], "10");
            }
        }

        else if (strcmp(linhaAtual[align], "ST") == 0) {
            if (desc->align == ESQ) {
                strcpy(linhaAtual[align], "12");
            } else {
                // desc->align = DIR
                strcpy(linhaAtual[align], "13");
            }
        }
    }

    if (linhaAtual[align+1][0] == 'n') {
        // parametro simples eh nome

        unsigned pos;
        sscanf(linhaAtual[align+1], PADRAO_NOME, &pos);

        if (pos >= qtosNomes) {
            logErroMontagem();
            return 1;
        }

        DescNome *desc = nomes[pos];
        if (desc->definido == 0) {
            fprintf(stderr, "ERRO: Usado mas não definido: %s", desc->nome);
            return 1;
        }

        if (desc->tipo == Simbolo && desc->valor > 1023) {
            logErroMontagem();
            return 1;
        }

        sprintf(linhaAtual[align+1], "%03x", desc->valor);
    }

    return 0;
}