#include <stdio.h>
#include <stdlib.h>
#include "TARVBM_bin.h"

typedef int OFFSET;

#define MAX_TAM_CHAVE 100
#define MAX_TAM_NOME 32

void ler_folha(const char *nome_arquivo, int t){
    FILE *fp = fopen(nome_arquivo, "rb");

    if(!fp){
        printf("Erro ao abrir %s\n", nome_arquivo);
        return;
    }

    int nchaves;
    int max_chaves = 2 * t - 1;
    char prox_folha[MAX_TAM_NOME];

    OFFSET *offset_chaves = malloc(sizeof(OFFSET) * max_chaves);
    char chave[MAX_TAM_CHAVE];

    fread(&nchaves, sizeof(int), 1, fp);

    printf("======================\n");
    printf("Arquivo: %s\n", nome_arquivo);
    printf("nchaves: %d\n\n", nchaves);

    fread(offset_chaves, sizeof(OFFSET), max_chaves, fp);
    fread(prox_folha, sizeof(char), MAX_TAM_NOME, fp);

    printf("offset_chaves:\n");
    for(int i = 0; i < max_chaves; i++){
        printf("[%d] %d\n", i, offset_chaves[i]);
    }

    printf("\nProxima folha: %s\n", prox_folha);

    printf("\nchaves:\n");
    for(int i = 0; i < nchaves; i++){
        fread(chave, sizeof(char), MAX_TAM_CHAVE, fp);
        printf("[%d] %s | offset no arq pessoa: %d\n", i, chave, offset_chaves[i]);
    }
    printf("======================");

    free(offset_chaves);
    fclose(fp);
}

int main(){
    int t = 2;
    char nome[MAX_TAM_NOME];

    printf("Digite a folha que deseja averiguar: ");
    scanf("%s", nome);
    ler_folha(nome, t);

    return 0;
}