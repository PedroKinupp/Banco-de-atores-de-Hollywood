#ifndef TARVBM_BIN_H
#define TARVBM_BIN_H
#include <stdio.h>
#include <stdlib.h>

typedef int OFFSET;

#define MAX_TAM_NOME 32
#define MAX_NOME_ARQ 32
#define MAX_TAM_CHAVE 100
#define MAX_TAM_TAGLINE 200

typedef struct no_bin {
    OFFSET offset;
    int nchaves;
    int folha;

    char **chave;
    OFFSET *offset_chaves;
    OFFSET *offset_filhos;

    char *folha_nome;
    char *prox_folha_nome;
} NO_BIN;

typedef struct {
    FILE *arquivo;
    char *nome_arquivo;
    int t;
    OFFSET raiz_offset;
} TARVBM_BIN;
typedef struct {
    char nome[MAX_TAM_NOME];
    int data_nascimento;
} PESSOA;
typedef struct {
    char nome[MAX_TAM_NOME];
    int data_lancamento;
    char tagline[MAX_TAM_TAGLINE];
} FILME;

// Operações básicas
TARVBM_BIN* TARVBM_BIN_criar(const char *nome_arquivo, int t);
TARVBM_BIN* TARVBM_BIN_abrir(const char *nome_arquivo);
void TARVBM_BIN_fechar(TARVBM_BIN *arvore);

// Operações de árvore
OFFSET TARVBM_BIN_busca(TARVBM_BIN *arvore, const char *chave);
void TARVBM_BIN_insere(TARVBM_BIN *arvore, const char *chave, OFFSET offset_pessoa);
OFFSET TARVBM_BIN_remove(TARVBM_BIN *arvore, char *chave);

//operações com pessoas
void TARVBM_BIN_insere_pessoa(TARVBM_BIN *arvore, PESSOA *p);
void TARVBM_BIN_remove_pessoa(TARVBM_BIN *arvore, PESSOA *p);

// Utilitários
void TARVBM_BIN_imprime(TARVBM_BIN *arvore);
void TARVBM_BIN_imprime_todos(TARVBM_BIN* avr);
int TARVBM_num_folhas(TARVBM_BIN* arv);

#endif