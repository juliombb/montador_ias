//
// Created by Julio Moreira B. Barros on 9/11/18.
//

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#define MAXINT 2147483647 // 2^31 -1
#define MININT -2147483648 // -2^31

int verificarPalavraValida(const char *palavra, size_t len) {
    if (palavra[0] >= 48 && palavra[0] <= 57) {
        // numero no inicio
        return 0;
    }

    for (int i = 0; i < len; ++i) {

        if (palavra[i] >= 48 && palavra[i] <= 57) { continue; }
        if (palavra[i] >= 65 && palavra[i] <= 90) { continue; }
        if (palavra[i] >= 97 && palavra[i] <= 122) { continue; }
        if (palavra[i] == 95) { continue; }

        // caracter invalido
        return 0;
    }

    return 1;
}

char * paraMaiuscula(const char *palavra) {
    size_t len = strlen(palavra);
    char *maiuscula = (char *) malloc((len+1)* sizeof(char));
    char decremento = 'a' - 'A';

    for (int i = 0; i <= len; ++i) {
        if (palavra[i] >= 'a' && palavra[i] <= 'z') {
            maiuscula[i] = palavra[i] - decremento;
        } else {
            maiuscula[i] = palavra[i];
        }
    }

    return maiuscula;
}

int eHexadecimal(char* palavra) {
    size_t len = strlen(palavra);
    if (len <= 2 || len > 12) {
        return 0;
    }

    if (palavra[0] == '0' && (palavra[1] == 'x' || palavra[1] == 'X')) {

        char* ptrErro;
        strtol(palavra, &ptrErro, 16);

        if (ptrErro != (palavra + len)) {
            return 0;
        }

        return 1;
    }

    return 0;
}

int eDecimal(char* palavra) {
    char* ptrErro;

    long numeroParseado = strtol(palavra, &ptrErro, 10);

    if (numeroParseado > MAXINT || numeroParseado < MININT) {
        fprintf(stderr, "Overflow decimal.");
        return 0;
    }

    if (ptrErro != (palavra + strlen(palavra))) {
        return 0;
    }

    return 1;
}

int eDiretiva(char* palavra) {
    size_t len = strlen(palavra);
    if (len < 2) {
        // diretiva mt curta
        return 0;
    }

    if (palavra[0] != '.') {
        // sem ponto no inicio
        return 0;
    }

    char *maiuscula = paraMaiuscula(palavra);

    if (strcmp(maiuscula, ".SET") == 0) { return 1; }
    if (strcmp(maiuscula, ".ORG") == 0) { return 1; }
    if (strcmp(maiuscula, ".ALIGN") == 0) { return 1; }
    if (strcmp(maiuscula, ".WFILL") == 0) { return 1; }
    if (strcmp(maiuscula, ".WORD") == 0) { return 1; }

    // desconhecida
    return 0;
}

int eNome(char* palavra) {
    return verificarPalavraValida(palavra, strlen(palavra));
}

int eInstrucao(char* palavra) {
    char *maiuscula = paraMaiuscula(palavra);

    if (strcmp(maiuscula, "LD") == 0) { return 1; }
    if (strcmp(maiuscula, "LDINV") == 0) { return 1; }
    if (strcmp(maiuscula, "LDABS") == 0) { return 1; }
    if (strcmp(maiuscula, "LDMQ") == 0) { return 1; }
    if (strcmp(maiuscula, "LDMQMX") == 0) { return 1; }
    if (strcmp(maiuscula, "STORE") == 0) { return 1; }
    if (strcmp(maiuscula, "JUMP") == 0) { return 1; }
    if (strcmp(maiuscula, "JUMPL") == 0) { return 1; }
    if (strcmp(maiuscula, "JUMPR") == 0) { return 1; }
    if (strcmp(maiuscula, "ADD") == 0) { return 1; }
    if (strcmp(maiuscula, "ADDABS") == 0) { return 1; }
    if (strcmp(maiuscula, "SUB") == 0) { return 1; }
    if (strcmp(maiuscula, "SUBABS") == 0) { return 1; }
    if (strcmp(maiuscula, "MULT") == 0) { return 1; }
    if (strcmp(maiuscula, "DIV") == 0) { return 1; }
    if (strcmp(maiuscula, "LSH") == 0) { return 1; }
    if (strcmp(maiuscula, "RSH") == 0) { return 1; }
    if (strcmp(maiuscula, "STORAL") == 0) { return 1; }
    if (strcmp(maiuscula, "STORAR") == 0) { return 1; }

    // desconhecida
    return 0;
}

int eRotulo(char* palavra) {
    size_t len = strlen(palavra);
    if (len < 2) {
        // rotulo mt curto
        return 0;
    }

    if (palavra[len - 1] != ':') {
        // rotulo nao terminado em :
        return 0;
    }

    return verificarPalavraValida(palavra, len - 1);
}