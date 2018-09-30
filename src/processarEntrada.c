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
    char *linhaLida = (char*) malloc((tamanho+1)* sizeof(char)); // precaução o nome
    char *entradaPosicionada = entrada;
    unsigned offset = 0;
    unsigned linhaAtual = 1;

    while (sscanf(entradaPosicionada, "%2000[^\n] %n", linhaLida, &offset) != EOF) {

        char *tokenAtual = (char*) malloc(100* sizeof(char));
        char *innerLinhaLida = linhaLida;
        unsigned innerOffset = 0;

        while (sscanf(innerLinhaLida, "%s%n", tokenAtual, &innerOffset) != EOF) {
            innerLinhaLida += innerOffset;
            if (tokenAtual[0] == '#') {
                // comentario, acabou a linha
                break;
            }

            char *maiuscula = paraMaiuscula(tokenAtual);

            Token t;
            t.linha = linhaAtual;
            t.palavra = strcat(maiuscula, " ");

            if (eDiretiva(tokenAtual)) { t.tipo = Diretiva; }
            else if (eRotulo(tokenAtual)) { t.tipo = DefRotulo; }
            else if (eInstrucao(tokenAtual)) { t.tipo = Instrucao; }
            else {
                if (recuperaToken(getNumberOfTokens() - 1).linha != linhaAtual
                    || recuperaToken(getNumberOfTokens() - 1).tipo == DefRotulo) {
                    // Nesse caso, teriamos algo que nao eh instrucao nem diretiva nem rotulo no inicio ou após um rótulo.

                    // Parametro onde nao deveria (talvez isso não deva ser feito pra números)
                    fprintf(stderr, "ERRO LEXICO: palavra inválida na linha %d!", linhaAtual);
                    return 0;
                }
                else if (eHexadecimal(tokenAtual)) { t.tipo = Hexadecimal; }
                else if (eDecimal(tokenAtual)) { t.tipo = Decimal; }
                else if (eNome(tokenAtual)) { t.tipo = Nome; }
                else {
                    fprintf(stderr, "ERRO LEXICO: palavra inválida na linha %d!", linhaAtual);
                    return 0;
                }
            }

            adicionarToken(t);

        } // aqui eh o fim da linha
        free(tokenAtual);
        entradaPosicionada += offset;
        linhaAtual++;
    }

    free(linhaLida);
    free(entradaPosicionada);
    /* printf("Você deve implementar esta função para a Parte 1.\n"); */
    return 0;
}
