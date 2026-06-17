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
    int opcao;
    int t;
    char nome[MAX_TAM_NOME];
    char arq[MAX_TAM_NOME];
    TARVBM_BIN * arvore = NULL;

    printf("O que deseja fazer?\n");
    while (1) {
        printf("1 - Criar Arvore\n");
        printf("2 - Abrir Arvore\n");
        printf("3 - Inserir\n");
        printf("4 - Remover\n");
        printf("5 - Imprimir arvore\n");
        printf("6 - Imprimir folhas\n");
        printf("7 - Verificar tamanho do no\n");
        printf("-1 - Sair\n");
        printf("Opcao: ");

        scanf("%d", &opcao);

        if (opcao == -1)
            break;

        switch (opcao) {
            case 1: {
                printf("Nome da arvore: ");
                scanf("%31s", arq);
                FILE *f = fopen(arq, "rb");
                if (f) { // Verifica se o arquivo já existe
                    fclose(f);
                    printf("Erro: ja existe uma arvore com esse nome.\n");
                    break;
                }
                printf("Digite o t: ");
                scanf("%d", &t);
                if (t < 2) t = 2;
                if (arvore) { // Fecha a árvore atual, se houver
                    TARVBM_BIN_fechar(arvore);
                    arvore = NULL;
                }
                arvore = TARVBM_BIN_criar(arq, t);
                if (!arvore) {
                    printf("Nao foi possivel criar a arvore (talvez ela ja exista).\n");
                    break;
                }
                printf("Arvore criada com sucesso.\n");
                break;
            }
            case 2: {
                printf("Escreva o nome da árvore que deseja abrir: ");
                scanf("%31s", arq);
                FILE *f = fopen(arq, "rb");
                if (!f) { // Verifica se o arquivo existe
                    printf("Erro: arvore não encontrada.\n");
                    break;
                }
                fclose(f);
                if (arvore) { // Fecha a árvore atual, se houver
                    TARVBM_BIN_fechar(arvore);
                    arvore = NULL;
                }
                arvore = TARVBM_BIN_abrir(arq);
                if (!arvore)
                    printf("Erro ao abrir a arvore.\n");
                else
                    printf("Arvore aberta com sucesso.\n");
                break;
            }
            case 3: {
                if (!arvore) {
                    printf("Erro: nenhuma árvore aberta.\n");
                    break;
                }
                printf("Digite alguem para inserir: ");
                scanf("%31s", nome);
                PESSOA *p = malloc(sizeof(PESSOA));
                if (!p) {
                    printf("Erro de memoria.\n");
                    break;
                }
                strcpy(p->nome,nome);
                p->data_nascimento = 2006;
                TARVBM_BIN_insere_pessoa(arvore, p);
                free(p);
                break;
            }
            case 4: {
                if (!arvore) {
                    printf("Erro: nenhuma arvore aberta.\n");
                    break;
                }
                printf("Digite alguem para remover: ");
                scanf("%31s", nome);
                PESSOA *p = malloc(sizeof(PESSOA));
                if (!p) {
                    printf("Erro de memoria.\n");
                    break;
                }
                strcpy(p->nome,nome);
                p->data_nascimento = 2006;
                TARVBM_BIN_remove_pessoa(arvore, p);
                free(p);
                break;
            }
            case 5:
                if (!arvore) {
                    printf("Erro: nenhuma arvore aberta.\n");
                    break;
                }
                printf("Imprimindo arvore...\n");
                TARVBM_BIN_imprime(arvore);
                break;

            case 6:
                printf("Imprimindo todos...\n");
                if (!arvore) {
                    printf("Erro: nenhuma arvore aberta.\n");
                    break;
                }
                TARVBM_BIN_imprime_todos(arvore);
                break;
            
            case 7:
                printf("imprimindo tamanho do no...\n");
                if (!arvore) {
                    printf("Erro: nenhuma arvore aberta.\n");
                    break;
                }
                imprimir_tam_no(arvore->t);
                break;

            default:
                printf("Opcao invalida!\n");
        }
    }
    if(arvore) TARVBM_BIN_fechar(arvore);
    return 0;
}
