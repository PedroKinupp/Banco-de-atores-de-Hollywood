#include "TARVBM_bin.h"

int main(){
    TARVBM_BIN * arvore = TARVBM_BIN_criar("ARVORE", 2);

    TARVBM_BIN_insere(arvore, "Keanu Reeves", 0);
    TARVBM_BIN_insere(arvore, "Carrie-Anne Moss", 16);
    TARVBM_BIN_insere(arvore, "Laurence Fishburne", 32);
    TARVBM_BIN_insere(arvore, "Hugo Weaving", 48);
    TARVBM_BIN_insere(arvore, "Lilly Wachowski", 64);
    TARVBM_BIN_insere(arvore, "Lana Wachowski", 0);
    TARVBM_BIN_insere(arvore, "Joel Silver", 16);
    TARVBM_BIN_insere(arvore, "Emil Eifrem", 32);
    TARVBM_BIN_insere(arvore, "Charlize Theron", 48);
    TARVBM_BIN_insere(arvore, "Al Pacino", 64);
    TARVBM_BIN_insere(arvore, "Taylor Hackford", 0);
    TARVBM_BIN_insere(arvore, "Tom Cruise", 16);
    TARVBM_BIN_insere(arvore, "Cuba Gooding Jr.", 32);
    TARVBM_BIN_insere(arvore, "Noah Wyle", 48);
    TARVBM_BIN_insere(arvore, "Anthony Edwards", 64);

    TARVBM_BIN_imprime(arvore);

    printf("===========================\n");

    printf("sizeof(int): %zu\n", sizeof(int));
    printf("sizeof(OFFSET): %zu\n", sizeof(OFFSET));
    printf("MAX_TAM_NOME: %d\n", MAX_TAM_NOME);
    printf("MAX_TAM_CHAVE: %d\n", MAX_TAM_CHAVE);

    printf("tamanho_no: %zu\n",
        2 * sizeof(int)
        + 2 * MAX_TAM_NOME
        + (2 * arvore->t - 1) * MAX_TAM_CHAVE
        + (2 * arvore->t - 1) * sizeof(OFFSET)   // offset_chaves
        + (2 * arvore->t) * sizeof(OFFSET)       // offset_filhos
    );

    printf("===========================");
    TARVBM_BIN_imprime_todos(arvore);
    TARVBM_BIN_fechar(arvore);
    return 0;
}
