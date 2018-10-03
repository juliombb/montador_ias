#include "montador.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

/*
 * Funções auxiliares
 */

int temErroNaInstrucao(Token token, unsigned pos);
int temErroNaDiretiva(Token token, unsigned pos);

/*
Exemplo de erros:
const char* get_error_string (enum errors code) {
    switch (code) {
        case ERR_HEXADECIMAL_NAO_VALIDO:
            return "LEXICO: Numero Hexadecimal Inválido!";
        case ERRO_ROTULO_JA_DEFINIDO:
            return "GRAMATICAL: ROTULO JA FOI DEFINIDO!";
        case ERR_DIRETIVA_NAO_VALIDA:
            return "LEXICO: Diretiva não válida";
*/

void logErroLexico(unsigned linha);
void logErroGramatical(unsigned linha); // funcoes para evitar erro na hora de fazer matching

int processarErrosLexicos(char *entrada, unsigned int tamanho);
int processarErrosGramaticais();

/*
    ---- Você Deve implementar esta função para a parte 1.  ----
    Entrada da função: arquivo de texto lido e seu tamanho
    Retorna:
        * 1 caso haja erro na montagem;
        * 0 caso não haja erro.
*/
int processarEntrada(char* entrada, unsigned tamanho)
{
    if (processarErrosLexicos(entrada, tamanho)) {
        return 1;
    }

    if (processarErrosGramaticais()) {
        return 1;
    }

    return 0;
}

int processarErrosGramaticais() {

    for (unsigned pos = 0; pos < getNumberOfTokens(); ++pos) {
        Token atual = recuperaToken(pos);

        if (atual.tipo == Instrucao) {
            if (temErroNaInstrucao(atual, pos)) {
                return 1;
            }
        }
        else if (atual.tipo == Diretiva) {
            if (temErroNaDiretiva(atual, pos)) {
                return 1;
            }
        }
        else if (atual.tipo == DefRotulo) {
            if (pos > 0 && recuperaToken(pos - 1).linha == atual.linha) {
                logErroGramatical(atual.linha);
                return 1;
            }
        }
        else if (atual.tipo == Hexadecimal
                 || atual.tipo == Decimal
                 || atual.tipo == Nome) {

            Token tokenAnterior = recuperaToken(pos - 1);
            if (pos == 0 || tokenAnterior.linha != atual.linha) {
                logErroGramatical(atual.linha);
                return 1;
            }

            if (tokenAnterior.tipo == DefRotulo) {
                logErroGramatical(atual.linha);
                return 1;
            }
        }
    }

    return 0;
}

int processarErrosLexicos(char *entrada, unsigned int tamanho) {
    char *linhaLida = (char*) malloc((tamanho + 1) * sizeof(char));
    char *entradaPosicionada = entrada;
    unsigned offset = 0;
    unsigned linhaAtual = 0;

    while(1) {
        unsigned i = 0;
        while (1) {
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

        char *tokenAtual = (char*) malloc(100* sizeof(char));
        char *innerLinhaLida = linhaLida;
        unsigned innerOffset = 0;

        while (sscanf(innerLinhaLida, "%s%n", tokenAtual, &innerOffset) != EOF) {
            innerLinhaLida += innerOffset;

            if (strlen(tokenAtual) <= 0 || tokenAtual[0] == '\n' || tokenAtual[0] == '#') {
                // comentario, acabou a linha
                break;
            }

            Token t;

            if (eDiretiva(tokenAtual)) { t.tipo = Diretiva; }
            else if (eRotulo(tokenAtual)) { t.tipo = DefRotulo; }
            else if (eInstrucao(tokenAtual)) { t.tipo = Instrucao; }
            else if (eHexadecimal(tokenAtual)) { t.tipo = Hexadecimal; }
            else if (eDecimal(tokenAtual)) { t.tipo = Decimal; }
            else if (eNome(tokenAtual)) { t.tipo = Nome; }
            else {
                logErroLexico(linhaAtual);
                return 1;
            }

            t.linha = linhaAtual;
            char *newToken = (char *) malloc(strlen(tokenAtual)* sizeof(char));
            strcpy(newToken, tokenAtual);
            t.palavra = strcat(newToken, " ");
            adicionarToken(t);

        }

        free(tokenAtual);
    }
}

int temErroNaInstrucao(Token token, unsigned pos) {
    char *maiuscula = paraMaiuscula(token.palavra);

    if (strcmp(maiuscula, "LDMQ ") == 0
        || strcmp(maiuscula, "LSH ") == 0
        || strcmp(maiuscula, "RSH ") == 0) {

        // Instrucoes sem parametro

        if (pos == (getNumberOfTokens() - 1)) { // esta na ultima posicao
            return 0;
        }

        Token proxToken = recuperaToken(pos + 1);
        if (proxToken.linha == token.linha) {
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
    if (proxToken.linha != token.linha) {
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

    if (arg1.linha != token.linha) {
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

    if (strcmp(maiuscula, ".SET ") == 0) {

        if (pos == (getNumberOfTokens() - 2)) { // esta na penultima posicao
            logErroGramatical(token.linha);
            return 1;
        }

        Token arg2 = recuperaToken(pos + 2);
        if (arg2.linha != token.linha) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg2.tipo != Hexadecimal && arg2.tipo != Decimal) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg2.tipo == Decimal) {
            long valorInteiro = strtol(arg1.palavra, NULL, 10);
            if (valorInteiro < 0) {
                logErroGramatical(token.linha);
                return 1;
            }
        }

        return 0;
    }

    if (strcmp(maiuscula, ".ORG ") == 0) {
        if (arg1.tipo != Decimal && arg1.tipo != Hexadecimal) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg1.tipo == Decimal) {
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

    if (strcmp(maiuscula, ".ALIGN ") == 0) {
        if (arg1.tipo != Decimal) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg1.tipo == Decimal) {
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

    if (strcmp(maiuscula, ".WFILL ") == 0) {
        if (arg1.tipo != Decimal) {
            logErroGramatical(token.linha);
            return 1;
        }

        if (arg1.tipo == Decimal) {
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

    if (strcmp(maiuscula, ".WORD ") == 0) {
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