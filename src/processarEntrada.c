// Júlio Moreira Blás de Barros RA 200491

#include "montador.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

/*
 * Funções auxiliares
 */

// Erros em tokens
int temErroNaInstrucao(Token token, unsigned pos);
int temErroNaDiretiva(Token token, unsigned pos);

// Funcoes para evitar erro no match dos testes
void logErroLexico(unsigned linha);
void logErroGramatical(unsigned linha);

// Erros na entrada
int existeErroLexico(char *entrada, unsigned int tamanho);
int existeErroGramatical();

/*
    ---- Você Deve implementar esta função para a parte 1.  ----
    Entrada da função: arquivo de texto lido e seu tamanho
    Retorna:
        * 1 caso haja erro na montagem;
        * 0 caso não haja erro.
*/
int processarEntrada(char* entrada, unsigned tamanho)
{
    if (existeErroLexico(entrada, tamanho)) {
        return 1;
    }

    if (existeErroGramatical()) {
        return 1;
    }

    return 0;
}

int existeErroGramatical() {

    for (unsigned pos = 0; pos < getNumberOfTokens(); ++pos) {
        Token atual = recuperaToken(pos);

        switch (atual.tipo) {
            case Instrucao: {
                if (temErroNaInstrucao(atual, pos)) {
                    return 1;
                }
                break;
            }

            case Diretiva: {
                if (temErroNaDiretiva(atual, pos)) {
                    return 1;
                }
                break;
            }

            case DefRotulo: {
                if (pos > 0 && recuperaToken(pos - 1).linha == atual.linha) {
                    // defRotulo deve ser a primeira coisa da linha
                    logErroGramatical(atual.linha);
                    return 1;
                }
                break;
            }

            case Hexadecimal:
            case Decimal:
            case Nome: {
                Token tokenAnterior = recuperaToken(pos - 1);
                if (pos == 0 || tokenAnterior.linha != atual.linha) {
                    // parametro nao deve estar no inicio da linha
                    logErroGramatical(atual.linha);
                    return 1;
                }

                if (tokenAnterior.tipo == DefRotulo) {
                    // parametro nao deve estar logo depois de rotulo
                    logErroGramatical(atual.linha);
                    return 1;
                }
                break;
            }
        }
    }

    return 0;
}

int existeErroLexico(char *entrada, unsigned int tamanho) {
    char *linhaLida = (char*) malloc((tamanho + 1) * sizeof(char));
    char *entradaPosicionada = entrada;
    unsigned offset = 0;
    unsigned linhaAtual = 0;

    while (1) {
        unsigned i = 0;
        while (1) {
            /*
             * Leitura feita caracter a caracter ate que seja encontrado \n
             * se nada mais puder ser lido e erros nao foram retornados,
             * a funcao terminou com sucesso
             */
            if (sscanf(entradaPosicionada, "%c%n", &linhaLida[i], &offset) == EOF) {
                free(linhaLida);
                return 0;
            }

            entradaPosicionada += offset;

            if (linhaLida[i] == '\n') { // terminou a linha
                linhaLida[i] = '\0';
                linhaAtual++;
                break;
            }

            i++;
        }

        char *palavraAtual = (char*) malloc(100 * sizeof(char));
        char *innerLinhaLida = linhaLida;
        unsigned innerOffset = 0;

        while (sscanf(innerLinhaLida, "%s%n", palavraAtual, &innerOffset) != EOF) {
            innerLinhaLida += innerOffset;

            if (strlen(palavraAtual) <= 0 || palavraAtual[0] == '\n' || palavraAtual[0] == '#') {
                // comentario, line break ou palavra vazia: acabou a linha
                break;
            }

            Token token;

            if (eDiretiva(palavraAtual)) { token.tipo = Diretiva; }
            else if (eRotulo(palavraAtual)) { token.tipo = DefRotulo; }
            else if (eInstrucao(palavraAtual)) { token.tipo = Instrucao; }
            else if (eHexadecimal(palavraAtual)) { token.tipo = Hexadecimal; }
            else if (eDecimal(palavraAtual)) { token.tipo = Decimal; }
            else if (eNome(palavraAtual)) { token.tipo = Nome; }
            else {
                logErroLexico(linhaAtual);
                return 1;
            }

            token.linha = linhaAtual;
            char *novaPalavra = (char *) malloc((strlen(palavraAtual)+1) * sizeof(char));
            strcpy(novaPalavra, palavraAtual);
            token.palavra = novaPalavra;
            adicionarToken(token);
        }

        free(palavraAtual);
    }
}

int temErroNaInstrucao(Token token, unsigned pos) {
    char *maiuscula = paraMaiuscula(token.palavra);

    if (strcmp(maiuscula, "LDMQ") == 0
        || strcmp(maiuscula, "LSH") == 0
        || strcmp(maiuscula, "RSH") == 0) {

        // Instrucoes sem parametro

        if (pos == (getNumberOfTokens() - 1)) { // esta na ultima posicao
            return 0;
        }

        Token proxToken = recuperaToken(pos + 1);
        if (proxToken.linha == token.linha) { // parametro na instrucao sem parametro
            logErroGramatical(token.linha);
            return 1;
        }

        return 0;
    }

    // Instrucoes com parametro

    if (pos == (getNumberOfTokens() - 1)) { // esta na ultima posicao
        logErroGramatical(token.linha);
        return 1;
    }

    Token proxToken = recuperaToken(pos + 1);
    if (proxToken.linha != token.linha) { // instrucao sem parametro (mas ela precisa ter!)
        logErroGramatical(token.linha);
        return 1;
    }

    if (proxToken.tipo == Instrucao
        || proxToken.tipo == Diretiva
        || proxToken.tipo == DefRotulo) {

        logErroGramatical(token.linha);
        return 1;
    }

    if (pos == (getNumberOfTokens() - 2)) { // esta na penultima posicao e deu td certo
        return 0;
    }

    if (recuperaToken(pos + 2).linha == token.linha) {
        // nao esta na penultima e tem token na linha ainda
        logErroGramatical(token.linha);
        return 1;
    }

    return 0;
}

int temErroNaDiretiva(Token token, unsigned pos) {
    char *maiuscula = paraMaiuscula(token.palavra);

    if (pos == (getNumberOfTokens() - 1)) { // esta na ultima posicao
        logErroGramatical(token.linha);
        return 1;
    }

    Token arg1 = recuperaToken(pos + 1);

    if (arg1.linha != token.linha) { // diretiva deve ter ao menos um param
        logErroGramatical(token.linha);
        return 1;
    }

    if (pos < (getNumberOfTokens() - 3)) { // esta antes da antepenultima posicao
        Token arg3 = recuperaToken(pos + 3);
        if (arg3.linha == token.linha) { // 3 parametros
            logErroGramatical(token.linha);
            return 1;
        }
    }

    if (strcmp(maiuscula, ".SET") == 0) {
        if (pos == (getNumberOfTokens() - 2)) { // esta na penultima posicao
            logErroGramatical(token.linha);
            return 1;
        }

        Token arg2 = recuperaToken(pos + 2);
        if (arg2.linha != token.linha) { // set deve ter só um param
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg2.tipo != Hexadecimal && arg2.tipo != Decimal) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg2.tipo == Decimal) { // limite decimal
            long valorInteiro = strtol(arg1.palavra, NULL, 10);
            if (valorInteiro < 0) {
                logErroGramatical(token.linha);
                return 1;
            }
        }

        return 0;
    }

    if (strcmp(maiuscula, ".ORG") == 0) {
        if (arg1.tipo != Decimal && arg1.tipo != Hexadecimal) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg1.tipo == Decimal) { // limite decimal
            long valorInteiro = strtol(arg1.palavra, NULL, 10);
            if (valorInteiro > 1023 || valorInteiro < 0) {
                logErroGramatical(token.linha);
                return 1;
            }
        }

        if (pos < (getNumberOfTokens() - 2)) { // esta antes da penultima posicao
            Token arg2 = recuperaToken(pos + 2);
            if (arg2.linha == token.linha) { // 2 parametros
                logErroGramatical(token.linha);
                return 1;
            }
        }

        return 0;
    }

    if (strcmp(maiuscula, ".ALIGN") == 0) {
        if (arg1.tipo != Decimal) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg1.tipo == Decimal) { // limite decimal
            long valorInteiro = strtol(arg1.palavra, NULL, 10);
            if (valorInteiro > 1023 || valorInteiro < 1) {
                logErroGramatical(token.linha);
                return 1;
            }
        }

        if (pos < (getNumberOfTokens() - 2)) { // esta antes da penultima posicao
            Token arg2 = recuperaToken(pos + 2);
            if (arg2.linha == token.linha) { // 2 parametros
                logErroGramatical(token.linha);
                return 1;
            }
        }

        return 0;
    }

    if (strcmp(maiuscula, ".WFILL") == 0) {
        if (arg1.tipo != Decimal) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg1.tipo == Decimal) { // limite decimal
            long valorInteiro = strtol(arg1.palavra, NULL, 10);
            if (valorInteiro > 1023 || valorInteiro < 1) {
                logErroGramatical(token.linha);
                return 1;
            }
        }

        if (pos == (getNumberOfTokens() - 2)) { // esta na penultima posicao
            logErroGramatical(token.linha);
            return 1;
        }

        Token arg2 = recuperaToken(pos + 2);
        if (arg2.linha != token.linha) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg2.tipo != Hexadecimal && arg2.tipo != Decimal && arg2.tipo != Nome) {
            logErroGramatical(token.linha);
            return 1;
        }

        return 0;
    }

    if (strcmp(maiuscula, ".WORD") == 0) {
        if (arg1.tipo != Hexadecimal && arg1.tipo != Decimal && arg1.tipo != Nome) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (pos < (getNumberOfTokens() - 2)) { // esta antes da penultima posicao
            Token arg2 = recuperaToken(pos + 2);
            if (arg2.linha == token.linha) { // 2 parametros
                logErroGramatical(token.linha);
                return 1;
            }
        }

        return 0;
    }

    logErroGramatical(token.linha);
    return 1;
}

void logErroGramatical(unsigned linha) {
    fprintf(stderr, "ERRO GRAMATICAL: palavra na linha %d!\n", linha);
}

void logErroLexico(unsigned linha) {
    fprintf(stderr, "ERRO LEXICO: palavra inválida na linha %d!\n", linha);
}
