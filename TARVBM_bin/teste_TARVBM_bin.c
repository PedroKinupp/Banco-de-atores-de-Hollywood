#include "TARVBM_bin.h"
#include <string.h>

void imprimir_tam_no(int t){
    printf("===========================\n");
    printf("sizeof(int): %zu\n", sizeof(int));
    printf("sizeof(OFFSET): %zu\n", sizeof(OFFSET));
    printf("MAX_TAM_NOME: %d\n", MAX_TAM_NOME);
    printf("MAX_TAM_CHAVE: %d\n", MAX_TAM_CHAVE);
    printf("tamanho_no: %zu\n",
        2 * sizeof(int)
        + 2 * MAX_TAM_NOME
        + (2 * t - 1) * MAX_TAM_CHAVE
        + (2 * t - 1) * sizeof(OFFSET)   // offset_chaves
        + (2 * t) * sizeof(OFFSET)       // offset_filhos
    );
    printf("===========================\n");
}

int main(){
    TARVBM_BIN * arvore = TARVBM_BIN_abrir("ARVORE");
    int opcao;
    int tam = 64;
    char nome[MAX_TAM_NOME];

    printf("O que deseja fazer?\n");
    while (1) {
        printf("\n");
        printf("1 - Inserir\n");
        printf("2 - Remover\n");
        printf("3 - Imprimir arvore\n");
        printf("4 - Imprimir folhas\n");
        printf("5 - Verificar tamanho do no\n");
        printf("-1 - Sair\n");
        printf("Opcao: ");

        scanf("%d", &opcao);

        if (opcao == -1)
            break;

        switch (opcao) {
            case 1:
                printf("Digite alguem para inserir\n");
                scanf("%s", nome);
                tam = tam + 12;
                PESSOA *p = malloc(sizeof(PESSOA));
                strcpy(p->nome,nome);
                p->data_nascimento = 2006;
                TARVBM_BIN_insere_pessoa(arvore, p);
                free(p);
                break;

            case 2:
                printf("Digite alguem para remover\n");
                scanf("%s", nome);
                TARVBM_BIN_remove(arvore, nome);
                break;

            case 3:
                printf("Imprimindo arvore...\n");
                TARVBM_BIN_imprime(arvore);
                break;

            case 4:
                printf("Imprimindo todos...\n");
                TARVBM_BIN_imprime_todos(arvore);
                break;
            
            case 5:
                printf("imprimindo tamanho do no...\n");
                imprimir_tam_no(arvore->t);
                break;

            default:
                printf("Opcao invalida!\n");
        }
    }
    TARVBM_BIN_fechar(arvore);
    return 0;
}
