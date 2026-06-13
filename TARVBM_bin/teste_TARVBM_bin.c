#include "TARVBM_bin.h"

int main(){
    TARVBM_BIN * arvore = TARVBM_BIN_abrir("PESSOAS");

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
        + (2 * arvore->t) * sizeof(OFFSET)
    );

    printf("===========================");
    TARVBM_BIN_imprime_todos(arvore);
    TARVBM_BIN_fechar(arvore);
    return 0;
}
