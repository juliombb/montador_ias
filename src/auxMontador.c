//
// Created by Julio Moreira B. Barros on 9/11/18.
//

#include <memory.h>

int eHexadecimal(char* palavra) {
    if (strlen(palavra) <= 2) {
        return 0;
    }

    if (palavra[0] == '0' && palavra[1] == 'x') {
        return 1;
    }

    return 0;
}

int eDiretiva(char* palavra) {

}

int eRotulo(char* palavre) {

}