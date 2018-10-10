#!/bin/bash
VERBOSE=0
SRC = $(PWD)/src
OUTDIR = $(PWD)/bin
AUXDIR = $(PWD)/aux
TESTDIR = $(PWD)/testes
TESTOUTSDIR = $(PWD)/saidas
RESULTSDIR = $(PWD)/resultados

CC = gcc
RM = rm -rf
CFLAGS = -std=c99 -Wall -O2
LDFLAGS = -lm

EXECUTES := $(wildcard $(TESTDIR)/*.in)
TESTES = 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23

montador-p1.x: rule_mkpath
	cat $(AUXDIR)/main1.c > $(SRC)/main.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/montador.x $(SRC)/*.c $(LDFLAGS)

montador-p2.x: rule_mkpath
	cat $(AUXDIR)/main2.c > $(SRC)/main.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/montador.x $(SRC)/*.c $(LDFLAGS)

rule_mkpath:
	@mkdir -p $(OUTDIR)
	@mkdir -p $(TESTOUTSDIR)

executar-tudo: $(EXECUTES)
	@$(foreach test, $(TESTES), $(OUTDIR)/montador.x $(TESTDIR)/arq$(test).in > $(TESTOUTSDIR)/arq$(test).res 2>&1; echo " ------ Resultado de $(TESTOUTSDIR)/arq$(test).res -------"; cat $(TESTOUTSDIR)/arq$(test).res; echo "----------------------------\n\n" || true; )

testar-parte1: set-parte1 $(TESTES)
	@echo "Finalizado testes da parte 1"

testar-parte2: set-parte2 $(TESTES)
	@echo "Finalizado testes da parte 2"

set-parte1:
	$(eval parte=parte1)

set-parte2:
	$(eval parte=parte2)

$(TESTES): montador-p1.x executar-tudo
	$(eval teste=$@)
	@echo "------------------- TESTE:" $(teste) " -------------------------"

	@echo "Comparando saida" $(teste) "com" arq$(teste).$(parte).res". Abaixo as diferenças: "

	@if [ ! -f $(RESULTSDIR)/arq$(teste).$(parte).res ] || [ ! -f $(TESTOUTSDIR)/arq$(teste).res ]; then \
		echo "*** TESTE "$(teste)": ARQUIVO NÃO EXISTE ***"; \
	elif [ "`head -n 1 $(RESULTSDIR)/arq$(teste).$(parte).res | grep 'ERRO' | wc -l`" = "0" ]; then \
		if [ `diff --side-by-side --suppress-common-lines --ignore-case $(TESTOUTSDIR)/arq$(teste).res $(RESULTSDIR)/arq$(teste).$(parte).res |  wc -l` != "0" ]; then \
			diff --side-by-side --suppress-common-lines --ignore-case $(TESTOUTSDIR)/arq$(teste).res $(RESULTSDIR)/arq$(teste).$(parte).res | cat -n; \
			echo "*** TESTE "$(teste)": ERRO ***"; \
		else \
			echo "NENHUMA DIFERENÇA! PARABÉNS!"; \
			echo "*** TESTE "$(teste)": SUCESSO ***"; \
		fi \
	else \
		if [ "`head -n 1 $(TESTOUTSDIR)/arq$(teste).res`" != "`head -n 1 $(RESULTSDIR)/arq$(teste).$(parte).res`" ]; then \
			echo " 1 `head -n 1 $(TESTOUTSDIR)/arq$(teste).res` |  `head -n 1 $(RESULTSDIR)/arq$(teste).$(parte).res`"; \
			echo "*** TESTE "$(teste)": ERRO ***"; \
		else \
			echo "NENHUMA DIFERENÇA! PARABÉNS!"; \
			echo "*** TESTE "$(teste)": SUCESSO ***"; \
		fi \
	fi

	@echo

clean:
	$(RM) $(OUTDIR)/*.x
	$(RM) $(SRC)/*.o
