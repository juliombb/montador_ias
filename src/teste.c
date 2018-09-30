#include "montador.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

/* Retorna:
 *  1 caso haja erro na montagem;
 *  0 caso não haja erro.
 */
int main(int args, char** argv)
{
    int ret = 0;

    if (args < 2) {
        printf("Você deve passar o nome do arquivo como parâmetro!\n");
        return 1;
    }

    return 0;
}
