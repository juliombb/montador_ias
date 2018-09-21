#include "montador.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

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

/*
    ---- Você Deve implementar esta função para a parte 1.  ----
    Entrada da função: arquivo de texto lido e seu tamanho
    Retorna:
        * 1 caso haja erro na montagem;
        * 0 caso não haja erro.
*/
int processarEntrada(char* entrada, unsigned tamanho)
{
    char *sLida = (char*) malloc((tamanho+1)* sizeof(char)); // precaução o nome
    char *sAtual = entrada;
    int offset = 0;

    while (sscanf(sAtual, "%2000[^\n] %n", sLida, &offset) != EOF) {
        char *tokenAtual = (char*) malloc(100* sizeof(char));
        int innerOffset = 0;
        char *innerSLida = sLida;
        while (sscanf(innerSLida, "%s%n", tokenAtual, &innerOffset) != EOF) {
            innerSLida += innerOffset;
            // tokenAtual é o token atual. TODO: Tratar ele aqui.
            if (eDiretiva(tokenAtual)) {

                char *maiuscula = paraMaiuscula(tokenAtual);

                if (strcmp(maiuscula, ".SET") == 0) { return 1; }
                if (strcmp(maiuscula, ".ORG") == 0) { return 1; }
                if (strcmp(maiuscula, ".ALIGN") == 0) { return 1; }
                if (strcmp(maiuscula, ".WFILL") == 0) { return 1; }
                if (strcmp(maiuscula, ".WORD") == 0) { return 1; }

            } else if (eRotulo(tokenAtual)) {

            } else if (eInstrucao(tokenAtual)) {

            } else {
                return 0;
            }


        } // aqui eh o fim da linha
        free(tokenAtual);
        sAtual += offset;
    }

    free(sLida);
    free(sAtual);
    /* printf("Você deve implementar esta função para a Parte 1.\n"); */
    return 0;
}
