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

//Descarta tudo que sobrou no stdin até o próximo
static void _limpa_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}
//Esta função remove o '\n'
static void _remove_quebra_linha(char *s) {
    int len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[len-1] = '\0';
        len--;
    }
}

static void solicitarNomeFilme(char *out) {
    printf("Digite o nome do filme: ");
    fgets(out, FH_NOME_MAX, stdin);
    _remove_quebra_linha(out);
}

static int solicitarAnoFilme(void) {
    int ano;
    printf("Digite o ano de lançamento do filme: ");
    scanf("%d", &ano);
    _limpa_buffer();
    return ano;
}

static void solicitarNomePessoa(char *out) {
    printf("Digite o nome da pessoa: ");
    fgets(out, FH_NOME_MAX, stdin);
    _remove_quebra_linha(out);
}

static void solicitarFuncaoEPapel(char *funcao, char *papel) {
    int tempOp;
    printf("Selecione a função:\n");
    printf("1 - ACTED_IN\n");
    printf("2 - DIRECTED\n");
    printf("3 - PRODUCED\n");
    printf("4 - WROTE\n");
    printf("Opcao: ");
    scanf("%d", &tempOp);
    _limpa_buffer();

    papel[0] = '\0';   // papel vazio por padrão 

    switch (tempOp) {
        case 1:
            strcpy(funcao, "ACTED_IN");
            printf("Digite o nome do personagem: ");
            fgets(papel, FH_ROLE_MAX, stdin);
            _remove_quebra_linha(papel);
            break;
        case 2:
            strcpy(funcao, "DIRECTED");
            break;
        case 3:
            strcpy(funcao, "PRODUCED");
            break;
        case 4:
            strcpy(funcao, "WROTE");
            break;
        default:
            funcao[0] = '\0';
            printf("Opcao invalida\n");
            break;
    }
}

void hash(void) {
    int opcao;

    printf("O que deseja fazer?\n");
    while (1) {
        printf("=================================\n");
        printf("1 - Iniciar hash de Relações\n");
        printf("2 - Cálculo de Hash\n");
        printf("3 - Inserir relação\n");
        printf("4 - Remover relação\n");
        printf("5 - Listar cast de um filme\n");
        printf("6 - Busca Individual\n");
        printf("-1 - Sair das opções de Hash\n");
        printf("Opcao: ");

        if (scanf("%d", &opcao) != 1) {
            _limpa_buffer();
            continue;
        }
        _limpa_buffer();

        if (opcao == -1)
            break;
        
        char nomeFilme[FH_NOME_MAX];
        char nomePessoa[FH_NOME_MAX];
        char funcao[FH_REL_MAX];
        char papel[FH_ROLE_MAX];
        int ano;
        TF resultado;
        int h1, ret, achou, n;

        switch (opcao) {
            case 1:
                //Inicializa a tabela zerada
                FH_inicializa(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA);

                //Carrega o Relationships.txt
                printf("Carregando arquivo...\n");
                n = FH_carregar_arquivo(ARQUIVO_HASH, ARQUIVO_DADOS,
                                        TAM_TABELA, "Relationships.txt", 0);
                printf("Registros inseridos: %d\n\n", n);
                break;

            case 2:
                printf("=== CalculoDeHash ===\n");
                solicitarNomeFilme(nomeFilme);
                ano = solicitarAnoFilme();

                h1 = CalculoDeHash(nomeFilme, ano, TAM_TABELA);
                printf("Hash(\"%s\", %d) = %d\n", nomeFilme, ano, h1);
                break;

            case 3:
                printf("=== Inserção de novo registro ===\n");
                solicitarNomeFilme(nomeFilme);
                ano = solicitarAnoFilme();
                solicitarNomePessoa(nomePessoa);
                solicitarFuncaoEPapel(funcao, papel);

                FH_insere(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
                         nomeFilme, ano, nomePessoa, funcao, papel);
                FH_listar_filme(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
                               nomeFilme, ano);
                break;

            case 4:
                printf("=== Remoção ===\n");
                solicitarNomeFilme(nomeFilme);
                ano = solicitarAnoFilme();
                solicitarNomePessoa(nomePessoa);

                ret = FH_retira_pessoa(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
                                       nomeFilme, ano, nomePessoa);
                printf("Retira %s de \"%s\": %s\n",
                       nomePessoa, nomeFilme, ret ? "OK" : "nao encontrado");

                //Verifica que sumiu da lista
                FH_listar_filme(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
                               nomeFilme, ano);
                putchar('\n');
                break;

            case 5:
                solicitarNomeFilme(nomeFilme);
                ano = solicitarAnoFilme();
                FH_listar_filme(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
                               nomeFilme, ano);
                putchar('\n');
                break;

            case 6:
                printf("=== Buscas individuais ===\n");
                solicitarNomeFilme(nomeFilme);
                ano = solicitarAnoFilme();
                solicitarNomePessoa(nomePessoa);

                achou = FH_busca_pessoa(ARQUIVO_HASH, ARQUIVO_DADOS, TAM_TABELA,
                                        nomeFilme, ano, nomePessoa, &resultado);
                if (achou)
                    printf("ENCONTRADO: %-20s em \"%s\"  [%s]  papel: %s\n",
                           nomePessoa, nomeFilme, resultado.relacao, resultado.papel);
                else
                    printf("NAO ENCONTRADO: %s em \"%s\"\n", nomePessoa, nomeFilme);
                break;

            default:
                printf("Opcao invalida!\n");
        }
    }
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
                hash();
                printf("E neccessário abrir uma Arvore novamente\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }
    }
    if(arvore) TARVBM_BIN_fechar(arvore);
    return 0;
}
