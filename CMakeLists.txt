cmake_minimum_required(VERSION 3.12)
project(montador_ias C)

set(CMAKE_C_STANDARD 11)

include_directories(src)
include_directories(aux)

add_executable(montador_ias
        src/auxMontador.c
        src/emitirMapaDeMemoria.c
        src/teste.c
        aux/main1.c
        aux/main2.c
        src/montador.h
        src/processarEntrada.c
        src/token.c
        src/token.h)
