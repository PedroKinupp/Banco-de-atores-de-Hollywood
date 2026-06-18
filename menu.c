#include "TARVBM_bin.h"
#include "FilmeHash.h"
#include <string.h>
#include <ctype.h>

/* Arquivos de persistência */
#define ARQUIVO_HASH  "filmes.hash"
#define ARQUIVO_DADOS "filmes.dat"
#define TAM_TABELA    101     

void imprimir_tam_no(int t){
    printf("---------------\n");
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
    printf("---------------\n");
}

void ler_linha(char *str, int tamanho) {// Descarta caracteres restantes no buffer (inclusive '\n')
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (fgets(str, tamanho, stdin) != NULL) {// Lê a linha
        str[strcspn(str, "\n")] = '\0';// Remove o '\n' final, se existir
    }
}
void remover_quebra(char *s) {
    s[strcspn(s, "\r\n")] = '\0';
}
void trim(char *s) {
    int i = 0;

    while (isspace((unsigned char)s[i])) i++;

    if (i > 0) memmove(s, s + i, strlen(s + i) + 1);

    int fim = strlen(s) - 1;
    while (fim >= 0 && isspace((unsigned char)s[fim])) {
        s[fim] = '\0';
        fim--;
    }
}
int ano_valido(char *s) {
    trim(s);

    if (strcmp(s, "(no birth year)") == 0) return 0;
    if (strcmp(s, "(no tagline)") == 0) return 0;

    return atoi(s);
}
void ler_arquivo_inserir(TARVBM_BIN *arvore, const char *nome_arquivo) {
    FILE *f = fopen(nome_arquivo, "r");

    if (!f) {
        printf("Erro ao abrir arquivo %s\n", nome_arquivo);
        return;
    }

    char linha[1000];

    while (fgets(linha, sizeof(linha), f)) {
        remover_quebra(linha);

        char *tipo = strtok(linha, "|");
        if (!tipo) continue;
        trim(tipo);

        if (strcmp(tipo, "Person") == 0) {
            char *nome = strtok(NULL, "|");
            char *ano = strtok(NULL, "|");

            if (!nome || !ano) continue;

            trim(nome);
            trim(ano);

            PESSOA p;

            strncpy(p.nome, nome, MAX_TAM_NOME - 1);
            p.nome[MAX_TAM_NOME - 1] = '\0';

            p.data_nascimento = ano_valido(ano);

            TARVBM_BIN_insere_pessoa(arvore, &p);
        }

        // else if (strcmp(tipo, "Movie") == 0) {
        //     char *nome = strtok(NULL, "|");
        //     char *ano = strtok(NULL, "|");
        //     char *tagline = strtok(NULL, "");

        //     if (!nome || !ano) continue;

        //     trim(nome);
        //     trim(ano);

        //     FILME filme;

        //     strncpy(filme.nome, nome, MAX_TAM_NOME - 1);
        //     filme.nome[MAX_TAM_NOME - 1] = '\0';

        //     filme.data_lancamento = ano_valido(ano);

        //     if (tagline) {
        //         trim(tagline);
        //         strncpy(filme.tagline, tagline, MAX_TAM_TAGLINE - 1);
        //     } else {
        //         strcpy(filme.tagline, "");
        //     }

        //     filme.tagline[MAX_TAM_TAGLINE - 1] = '\0';

        //     TARVBM_BIN_insere_filme(arvore, &filme);
        // }
    }
    fclose(f);
}

int manipular_hash(void) {

    //1. Inicializa a tabela zerada
    FH_inicializa(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA);

    //2. Carrega o Relationships.txt
    //(O dataset não possui ano → 0 como padrão)
    printf("Carregando arquivo...\n");
    int n = FH_carregar_arquivo(ARQUIVO_HASH, ARQUIVO_DADOS,
                                TAM_TABELA, "Relationships.txt", 0);
    printf("Registros inseridos: %d\n\n", n);

    //3. Demonstração: CalculoDeHash
    //Pessoas do mesmo filme → mesmo bucket
    printf("=== CalculoDeHash ===\n");
    int h1 = CalculoDeHash("The Matrix", 0, TAM_TABELA);
    int h2 = CalculoDeHash("The Matrix", 0, TAM_TABELA);
    int h3 = CalculoDeHash("A Few Good Men", 0, TAM_TABELA);
    printf("Hash(\"The Matrix\", 0)     = %d\n", h1);
    printf("Hash(\"The Matrix\", 0)     = %d  \n", h2);
    printf("Hash(\"A Few Good Men\", 0) = %d  \n\n", h3);

    //4. Listar cast completo de alguns filmes
    FH_listar_filme(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA, "The Matrix", 0);
    putchar('\n');
    FH_listar_filme(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA, "A Few Good Men", 0);
    putchar('\n');

    // 5. Busca individual
    printf("=== Buscas individuais ===\n");
    TF resultado;
    const char *filmes[] = {"The Matrix", "A Few Good Men", "Top Gun"};
    const char *pessoas[] = {"Keanu Reeves", "Tom Cruise", "Tom Hanks"};
    for (int i = 0; i < 3; i++) {
        int achou = FH_busca_pessoa(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
                                    filmes[i], 0, pessoas[i], &resultado);
        if (achou)
            printf("ENCONTRADO: %-20s em \"%s\"  [%s]  papel: %s\n",
                   pessoas[i], filmes[i], resultado.relacao, resultado.papel);
        else
            printf("NAO ENCONTRADO: %s em \"%s\"\n", pessoas[i], filmes[i]);
    }

    /* Busca de alguém que não está no filme */
    int achou = FH_busca_pessoa(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
                                "The Matrix", 0, "Tom Hanks", &resultado);
    printf("Tom Hanks em \"The Matrix\": %s\n\n",
           achou ? "ENCONTRADO" : "NAO ENCONTRADO");

    //6. Remoção
    printf("=== Remoção ===\n");
    int ret = FH_retira_pessoa(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
                               "The Matrix", 0, "Hugo Weaving");
    printf("Retira Hugo Weaving de \"The Matrix\": %s\n", ret ? "OK" : "nao encontrado");

    /* Verifica que sumiu da listagem */
    FH_listar_filme(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA, "The Matrix", 0);
    putchar('\n');

    //7. Re-inserção (reaproveitamento de slot)
    printf("=== Re-inserção de Hugo Weaving ===\n");
    FH_insere(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
              "The Matrix", 0,
              "Hugo Weaving", "ACTED_IN", "Agent Smith");
    FH_listar_filme(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA, "The Matrix", 0);
    putchar('\n');

    //8. Inserção de um registro novo (não estava no .txt)
    printf("=== Inserção manual de novo registro ===\n");
    FH_insere(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
              "The Matrix", 0,
              "Andy Wachowski", "DIRECTED", "");
    FH_listar_filme(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA, "The Matrix", 0);

    return 0;
}

int main(){
    int opcao;
    int t;
    char nome[MAX_TAM_NOME];
    char arq[MAX_TAM_NOME];
    int data;
    TARVBM_BIN * arvore = NULL;

    printf("O que deseja fazer?\n");
    while (1) {
        printf("=================================\n");
        printf("1 - Criar Arvore\n");
        printf("2 - Abrir Arvore\n");
        printf("3 - Inserir\n");
        printf("4 - Remover\n");
        printf("5 - Imprimir arvore\n");
        printf("6 - Imprimir folhas\n");
        printf("7 - Verificar tamanho do no\n");
        printf("8 - Buscar Ator por nome\n");
        printf("9 - Preencher arvore\n");
        printf("0 - Checar Hash\n");
        printf("-1 - Sair\n");
        printf("Opcao: ");

        scanf("%d", &opcao);

        if (opcao == -1)
            break;

        switch (opcao) {
            case 1: {
                printf("Nome da arvore: ");
                scanf("%31s", arq);

                printf("Digite o t: ");
                scanf("%d", &t);

                if (t < 2) t = 2;

                if (arvore) {
                    TARVBM_BIN_fechar(arvore);
                    arvore = NULL;
                }

                arvore = TARVBM_BIN_criar(arq, t);

                if (!arvore)
                    printf("Nao foi possivel criar a arvore.\n");
                else
                    printf("Arvore criada com sucesso.\n");

                break;
            }
            case 2: {
                printf("Escreva o nome da arvore que deseja abrir: ");
                scanf("%31s", arq);

                if (arvore) {
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
                    printf("Erro: nenhuma arvore aberta.\n");
                    break;
                }
                printf("Digite alguem para inserir: ");
                ler_linha(nome, MAX_TAM_NOME);
                printf("Digite a data de nascimento: ");
                scanf("%d", &data);
                PESSOA *p = malloc(sizeof(PESSOA));
                if (!p) {
                    printf("Erro de memoria.\n");
                    break;
                }
                strcpy(p->nome,nome);
                p->data_nascimento = data;
                p->data_nascimento = data;
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
                ler_linha(nome, MAX_TAM_NOME);
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
            
            case 8:
                printf("Digite alguem para buscar: ");
                ler_linha(nome, MAX_TAM_NOME);
                OFFSET offset = TARVBM_BIN_busca(arvore, nome);
                if (offset == -1) {
                    printf("Pessoa nao encontrada.\n");
                    break;
                }
                printf("%d\n", offset);
                FILE * fp = fopen("PESSOAS", "rb");
                if (!fp) {
                    printf("Erro ao abrir arquivo PESSOAS.\n");
                    break;
                }
                fseek(fp, offset, SEEK_SET);
                PESSOA p;
                fread(&p, sizeof(PESSOA), 1, fp);
                printf("--------\n");
                printf("Nome: %s\n", p.nome);
                printf("Data de nascimento: %d\n", p.data_nascimento);
                printf("--------\n");
                fclose(fp);
                break;

            case 9:
                if (!arvore) {
                    printf("Digite a arvore que deseja preencher: ");
                    scanf("%31s", arq);

                    arvore = TARVBM_BIN_abrir(arq);

                    if (!arvore) {
                        printf("Erro ao abrir a arvore.\n");
                        break;
                    }
                }

                ler_arquivo_inserir(arvore, "../nodes.txt");
                printf("Arquivo carregado com sucesso!\n");
                break;
            
            case 0:
                if (arvore) {
                    TARVBM_BIN_fechar(arvore);
                    arvore = NULL;
                }
                manipular_hash();
                break;

            default:
                printf("Opcao invalida!\n");
        }
    }
    if(arvore) TARVBM_BIN_fechar(arvore);
    return 0;
}
