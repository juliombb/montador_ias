//
// Created by Julio Moreira B. Barros on 9/11/18.
//

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

char * paraMaiuscula(const char *palavra) {
    size_t len = strlen(palavra);
    char *maiuscula = (char *) malloc(len* sizeof(char));
    char decremento = 'a' - 'A';

    for (int i = 0; i < len; ++i) {
        if (palavra[0] >= 65 && palavra[0] <= 90) {
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

    if (palavra[0] == '0' && palavra[1] == 'x') {

        long numeroParseado;
        char* ptrErro;

        numeroParseado = strtol(palavra, &ptrErro, 16);

        if (ptrErro != (palavra + strlen(palavra))) {
            return 0;
        }

        return 1;
    }

    return 0;
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

int eInstrucao(char* palavra) {
    char *maiuscula = paraMaiuscula(palavra);

    if (strcmp(maiuscula, "LOAD") == 0) { return 1; }
    if (strcmp(maiuscula, "STOR") == 0) { return 1; }
    if (strcmp(maiuscula, "JUMP") == 0) { return 1; }
    if (strcmp(maiuscula, "ADD") == 0) { return 1; }
    if (strcmp(maiuscula, "SUB") == 0) { return 1; }
    if (strcmp(maiuscula, "MUL") == 0) { return 1; }
    if (strcmp(maiuscula, "DIV") == 0) { return 1; }
    if (strcmp(maiuscula, "LSH") == 0) { return 1; }
    if (strcmp(maiuscula, "RSH") == 0) { return 1; }

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

    if (palavra[0] >= 48 && palavra[0] <= 57) {
        // numero no inicio
        return 0;
    }

    for (int i = 0; i < len - 1; ++i) {

        if (palavra[0] >= 48 && palavra[0] <= 57) { continue; }
        if (palavra[0] >= 65 && palavra[0] <= 90) { continue; }
        if (palavra[0] >= 97 && palavra[0] <= 122) { continue; }

        // caracter invalido
        return 0;
    }

    return 1;
}