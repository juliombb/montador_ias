#include "montador.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define ESQ 0
#define DIR 2

#define PADRAO_NOME "n%02X"
#define PADRAO_WORD "w%02X"

typedef enum TipoNome {Simbolo, Rotulo} TipoNome;
typedef struct DescNome {
    char* nome;
    char definido; // 0 se foi definido ou 1 se não
    unsigned char align;
    TipoNome tipo;
    unsigned valor;
} DescNome;

int linhaNaoZerada(char **linhaAtual);
void adicionaInstrucao(char **linhaAtual, unsigned char align, const char *maiuscula);
unsigned adicionaOuEncontra(DescNome** nomes, unsigned* qtosNomes, char* simbolo);
int define(DescNome** nomes, unsigned* qtosNomes,
           char* nome, unsigned valor, unsigned char align, TipoNome tipo);

int preencheSeNecessario(char **linhaAtual, unsigned char align, DescNome** nomes, unsigned qtosNomes);

int trataInstrucao(
        DescNome **nomes, unsigned *qtosNomes,
        unsigned char align, char **linhaAtual,
        unsigned *pos, Token *atual);

int trataDiretiva(
        DescNome **nomes, unsigned *qtosNomes,
        unsigned char *align, char ***linhaAtual,
        unsigned *pos, Token *atual, unsigned *numLinhaAtual,
        char ***saida, unsigned* ordemDasLinhas, unsigned* proximaLinhaDaOrdem);


char **alocaLinha();

/*
    Retorna:
        *  1 caso haja erro na montagem;
        *  0 caso não haja erro.
 */
int emitirMapaDeMemoria()
{
    unsigned* ordemDasLinhas = (unsigned*) malloc(1024* sizeof(unsigned));
    unsigned proximaLinhaDaOrdem = 0;
    // vetor que armazena cada linha em uma posicao
    // para printar ordenado e o teste passar :P

    char*** saida = (char***) malloc(1024* sizeof(char**));
    // matriz cujo indice vertical eh o end de
    // e as 4 colunas compoem a linha

    DescNome** nomes = (DescNome**) malloc(1024* sizeof(DescNome*));
    unsigned qtosNomes = 0;
    // vetor que armazena os nomes

    unsigned char align = ESQ;
    // representacao do alinhamento

    char** linhaAtual = NULL; // minha IDE me enche o saco se eu nao fizer isso
    unsigned numLinhaAtual = 0;
    // linhaAtual eh a linha processada

    for (unsigned pos = 0; pos < getNumberOfTokens(); pos++) {
        if (!linhaAtual) {
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
                if (trataDiretiva(nomes, &qtosNomes, &align, &linhaAtual,
                                  &pos, &atual, &numLinhaAtual, saida,
                                  ordemDasLinhas, &proximaLinhaDaOrdem)) {
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
            // Os outros casos ja foram tratados
        }

        if (align == ESQ) {
            align = DIR;
        } else {
            align = ESQ;
            ordemDasLinhas[proximaLinhaDaOrdem++] = numLinhaAtual;
            saida[numLinhaAtual] = linhaAtual;
            linhaAtual = alocaLinha();
            numLinhaAtual++;
        }
    }

    if (linhaNaoZerada(linhaAtual)) {
        saida[numLinhaAtual] = linhaAtual;
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
            sprintf(palavra, "%010X", desc->valor);

            for (size_t charAt = 9; charAt > 6; --charAt) {
                linhaAtual[3][charAt - 7] = palavra[charAt];
            }

            for (size_t charAt = 6; charAt > 4; --charAt) {
                linhaAtual[2][charAt - 5] = palavra[charAt];
            }

            for (size_t charAt = 4; charAt > 1; --charAt) {
                linhaAtual[1][charAt - 2] = palavra[charAt];
            }

            for (int charAt = 1; charAt >= 0; --charAt) {
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

    // ultima sondagem
    // imprime todos
    for (numLinhaAtual = 0; numLinhaAtual < proximaLinhaDaOrdem; ++numLinhaAtual) {
        linhaAtual = saida[ordemDasLinhas[numLinhaAtual]];

        if (!linhaAtual) {
            continue;
        }

        printf("%03X %s %s %s %s\n",
               ordemDasLinhas[numLinhaAtual], linhaAtual[0], linhaAtual[1], linhaAtual[2], linhaAtual[3]);
    }

    return 0;
}

char **alocaLinha() {
    char **linhaAtual = (char**) malloc(4 * sizeof(char*)); // 4 strings

    linhaAtual[0] = (char*) malloc(3* sizeof(char)); // inst 1
    strcpy(linhaAtual[0], "00");
    linhaAtual[1] = (char*) malloc(4* sizeof(char)); // end 1
    strcpy(linhaAtual[1], "000");
    linhaAtual[2] = (char*) malloc(3* sizeof(char)); // inst 2
    strcpy(linhaAtual[2], "00");
    linhaAtual[3] = (char*) malloc(4* sizeof(char)); // end 2
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

                 sprintf(linhaAtual[align + 1], "%03lX", strtol(prox.palavra, NULL, 16));
                 break;
             }
             case Decimal: {
                 long num = strtol(prox.palavra, NULL, 10);
                 if (num > 65535) {
                     logErroGramatical(prox.linha);
                     return 1;
                 }

                 sprintf(linhaAtual[align + 1], "%03lX", num);
                 break;
             }
             case Nome: {
                 unsigned numsym = adicionaOuEncontra(nomes, qtosNomes, prox.palavra);
                 sprintf(linhaAtual[align + 1], PADRAO_NOME, numsym);
                 break;
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
        strcpy(linhaAtual[align], "0F"); // alias p/ mudar dps
    }
    if (strcmp(maiuscula, "JUMPR") == 0) {
        strcpy(linhaAtual[align], "10");
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
        strcpy(linhaAtual[align], "12");
    }
    if (strcmp(maiuscula, "STORAR") == 0) {
        strcpy(linhaAtual[align], "13");
    }
}

int linhaNaoZerada(char **linhaAtual) {
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
        char ***linhaAtual,
        unsigned int *pos,
        Token *atual,
        unsigned *numLinhaAtual,
        char ***saida,
        unsigned* ordemDasLinhas,
        unsigned* proximaLinhaDaOrdem
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

        if (linhaNaoZerada(*linhaAtual)) {
            ordemDasLinhas[*proximaLinhaDaOrdem] = *numLinhaAtual;
            (*proximaLinhaDaOrdem) += 1;
            saida[*numLinhaAtual] = *linhaAtual;
        } else {
            free(*linhaAtual);
        }

        *linhaAtual = alocaLinha();
        (*numLinhaAtual) = num;
        (*align) = ESQ;
        return (*numLinhaAtual > 1023);
    }

    if (strcmp(maiuscula, ".ALIGN") == 0) {
        // ja verificamos que eh decimal
        long num = strtol(prox.palavra, NULL, 10);

        if (*align == ESQ && (*numLinhaAtual % num == 0)) {
            return 0;
        }

        if (linhaNaoZerada(*linhaAtual)) {
            ordemDasLinhas[*proximaLinhaDaOrdem] = *numLinhaAtual;
            (*proximaLinhaDaOrdem) += 1;
            saida[*numLinhaAtual] = *linhaAtual;
        } else {
            free(*linhaAtual);
        }

        do {
            (*numLinhaAtual) = (*numLinhaAtual) + 1;
        } while ((*numLinhaAtual) % num != 0);

        *linhaAtual = alocaLinha();
        (*align) = ESQ;
        return (*numLinhaAtual > 1023);
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
                    sprintf(palavra, "0x%010lX", strtol(valor.palavra, NULL, 16));
                }

                // prox.palavra eh o numero de linhas
                for (int i = 0; i < strtol(prox.palavra, NULL, 10); ++i) {
                    // preenchendo a linha
                    for (size_t charAt = 11; charAt > 8; --charAt) {
                        (*linhaAtual)[3][charAt-9] = palavra[charAt];
                    }

                    for (size_t charAt = 8; charAt > 6; --charAt) {
                        (*linhaAtual)[2][charAt-7] = palavra[charAt];
                    }

                    for (size_t charAt = 6; charAt > 3; --charAt) {
                        (*linhaAtual)[1][charAt-4] = palavra[charAt];
                    }

                    for (size_t charAt = 3; charAt > 1; --charAt) {
                        (*linhaAtual)[0][charAt-2] = palavra[charAt];
                    }

                    ordemDasLinhas[*proximaLinhaDaOrdem] = *numLinhaAtual;
                    (*proximaLinhaDaOrdem) += 1;
                    saida[*numLinhaAtual] = *linhaAtual;
                    (*numLinhaAtual) += 1;
                    *linhaAtual = alocaLinha();

                    if ((*numLinhaAtual) > 1023) {
                        return 1;
                    }
                }

                return (*numLinhaAtual) > 1023;
            }
            case Decimal: {
                long argVal = strtol(valor.palavra, NULL, 10);
                char* palavra = (char*) malloc(11* sizeof(char));
                sprintf(palavra, "%010lX", argVal);

                for (int i = 0; i < strtol(prox.palavra, NULL, 10); ++i) {
                    // preenchendo a linha
                    for (size_t charAt = 9; charAt > 6; --charAt) {
                        (*linhaAtual)[3][charAt - 7] = palavra[charAt];
                    }

                    for (size_t charAt = 6; charAt > 4; --charAt) {
                        (*linhaAtual)[2][charAt - 5] = palavra[charAt];
                    }

                    for (size_t charAt = 4; charAt > 1; --charAt) {
                        (*linhaAtual)[1][charAt - 2] = palavra[charAt];
                    }

                    for (int charAt = 1; charAt >= 0; --charAt) {
                        (*linhaAtual)[0][charAt] = palavra[charAt];
                    }

                    ordemDasLinhas[*proximaLinhaDaOrdem] = *numLinhaAtual;
                    (*proximaLinhaDaOrdem) += 1;
                    saida[*numLinhaAtual] = *linhaAtual;
                    (*numLinhaAtual) += 1;
                    *linhaAtual = alocaLinha();

                    if ((*numLinhaAtual) > 1023) {
                        return 1;
                    }
                }

                return (*numLinhaAtual) > 1023;
            }
            case Nome: {
                unsigned numsym = adicionaOuEncontra(nomes, qtosNomes, valor.palavra);
                for (int i = 0; i < strtol(prox.palavra, NULL, 10); ++i) {
                    (*linhaAtual)[0][0] = 'w';
                    sprintf((*linhaAtual)[3], PADRAO_WORD, numsym);
                    // w000000w12 é uma word que deve ser preenchida com o nome 12

                    ordemDasLinhas[*proximaLinhaDaOrdem] = *numLinhaAtual;
                    (*proximaLinhaDaOrdem) += 1;
                    saida[*numLinhaAtual] = *linhaAtual;
                    (*numLinhaAtual) += 1;

                    if ((*numLinhaAtual) > 1023) {
                        return 1;
                    }

                    *linhaAtual = alocaLinha();
                }

                return (*numLinhaAtual) > 1023;
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
                    sprintf(palavra, "0x%010lX", strtol(prox.palavra, NULL, 16));
                }

                // preenchendo a linha
                for (size_t charAt = 11; charAt > 8; --charAt) {
                    (*linhaAtual)[3][charAt-9] = palavra[charAt];
                }

                for (size_t charAt = 8; charAt > 6; --charAt) {
                    (*linhaAtual)[2][charAt-7] = palavra[charAt];
                }

                for (size_t charAt = 6; charAt > 3; --charAt) {
                    (*linhaAtual)[1][charAt-4] = palavra[charAt];
                }

                for (size_t charAt = 3; charAt > 1; --charAt) {
                    (*linhaAtual)[0][charAt-2] = palavra[charAt];
                }
                break;
            }
            case Decimal: {
                long argVal = strtol(prox.palavra, NULL, 10);
                char* palavra = (char*) malloc(11* sizeof(char));
                sprintf(palavra, "%010lX", argVal);

                // preenchendo a linha
                for (size_t charAt = 9; charAt > 6; --charAt) {
                    (*linhaAtual)[3][charAt - 7] = palavra[charAt];
                }

                for (size_t charAt = 6; charAt > 4; --charAt) {
                    (*linhaAtual)[2][charAt - 5] = palavra[charAt];
                }

                for (size_t charAt = 4; charAt > 1; --charAt) {
                    (*linhaAtual)[1][charAt - 2] = palavra[charAt];
                }

                for (int charAt = 1; charAt >= 0; --charAt) {
                    (*linhaAtual)[0][charAt] = palavra[charAt];
                }
                break;
            }
            case Nome: {
                (*linhaAtual)[0][0] = 'w';
                unsigned numsym = adicionaOuEncontra(nomes, qtosNomes, prox.palavra);
                sprintf((*linhaAtual)[3], PADRAO_WORD, numsym);
                break;
            }
        }

        ordemDasLinhas[*proximaLinhaDaOrdem] = *numLinhaAtual;
        (*proximaLinhaDaOrdem) += 1;
        saida[*numLinhaAtual] = *linhaAtual;
        *linhaAtual = alocaLinha();
        (*numLinhaAtual) += 1;
        return (*numLinhaAtual) > 1023;
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
           char* nome, unsigned valor, unsigned char align, TipoNome tipo) {
    unsigned pos;

    switch (tipo) {

        case Simbolo: {
            pos = adicionaOuEncontra(nomes, qtosNomes, nome);
            break;
        }
        case Rotulo: {
            char* nomeReal = (char*) malloc(strlen(nome) * sizeof(char));
            for (unsigned char i = 0; i < strlen(nome)-1; ++i) {
                nomeReal[i] = nome[i];
            }
            nomeReal[strlen(nome) - 1] = '\0';
            pos = adicionaOuEncontra(nomes, qtosNomes, nomeReal);
            break;
        }
    }

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
    if (strcmp(linhaAtual[align], "JM") == 0) {

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

        sprintf(linhaAtual[align+1], "%03X", desc->valor);

        if (desc->align == ESQ) {
            strcpy(linhaAtual[align], "0D");
        } else {
            // desc->align = DIR
            strcpy(linhaAtual[align], "0E");
        }

        /*else if (strcmp(linhaAtual[align], "J+") == 0) {
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
        } */
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

        sprintf(linhaAtual[align+1], "%03X", desc->valor);
    }

    return 0;
}