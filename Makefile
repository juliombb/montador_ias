#!/bin/bash
VERBOSE=0
SRC = $(PWD)/src
OUTDIR = $(PWD)/bin
TESTDIR = $(PWD)/testes
TESTOUTSDIR = $(PWD)/saidas
RESULTSDIR = $(PWD)/resultados

CC = gcc
RM = rm -rf
CFLAGS = -std=c99 -Wall -O2
LDFLAGS = -lm

TESTES = 00 01 02 03 04 05 06 07 08


#.PHONY: montador.x testar testar-tudo clean

montador.x: rule_mkpath $(OBJS)
	$(CC) $(CFLAGS) -o $(OUTDIR)/montador.x $(SRC)/*.c $(LDFLAGS)

rule_mkpath:
	mkdir -p $(OUTDIR)
	mkdir -p $(TESTOUTSDIR)

executar:
	$(OUTDIR)/montador.x $(TESTDIR)/arq$(teste).in > $(TESTOUTSDIR)/arq$(teste).res 2>&1;

executar-tudo:
	for teste in $(TESTES); do \
		$(OUTDIR)/montador.x $(TESTDIR)/arq$$teste.in > $(TESTOUTSDIR)/arq$$teste.res 2>&1 || continue; \
	done;

testar-parte1:
	diff $(TESTOUTSDIR)/arq$(teste).res $(RESULTSDIR)/arq$(teste).parte1.res || echo "ERROR!"

clean:
	$(RM) $(OUTDIR)
	$(RM) $(SRC)/*.o
