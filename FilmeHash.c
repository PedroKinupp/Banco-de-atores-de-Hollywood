#include "FilmeHash.h"

 
//Concatena título e ano em uma única string "titulo|ano"
//Exemplo: "The Matrix|1999"
static void _montar_chave(const char *titulo, int ano, char *chave, int sz) {
    snprintf(chave, sz, "%s|%d", titulo, ano);
}

//djb2
//Retorna um valor sem sinal que deve ser reduzido módulo m.
static unsigned long _djb2(const char *str) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        h = ((h << 5) + h) ^ c;   /* h = h*33 XOR c */
    return h;
}

//CalculoDeHash
int CalculoDeHash(const char *titulo, int ano, int m) {
    char chave[FH_NOME_MAX + 16];
    _montar_chave(titulo, ano, chave, sizeof(chave));
    return (int)(_djb2(chave) % (unsigned long)m);
}


//FH_inicializa
void FH_inicializa(const char *tabHash, const char *dados, int m) {
    //Cria ou limpa o arquivo de dados
    FILE *fp = fopen(dados, "wb");
    if (!fp) { perror("FH_inicializa: dados"); exit(1); }
    fclose(fp);

    // Cria o arquivo-índice com m entradas = -1
    fp = fopen(tabHash, "wb");
    if (!fp) { perror("FH_inicializa: tabHash"); exit(1); }
    int vazio = -1;
    for (int i = 0; i < m; i++)
        fwrite(&vazio, sizeof(int), 1, fp);
    fclose(fp);
}

//FH_insere
void FH_insere(const char *tabHash, const char *dados, int m,
               const char *titulo, int ano,
               const char *pessoa, const char *relacao,
               const char *papel)
{
    int h = CalculoDeHash(titulo, ano, m);

    //lê posição do cabeçalho da hash
    FILE *fph = fopen(tabHash, "rb+");
    if (!fph) { perror("FH_insere: tabHash"); exit(1); }
    fseek(fph, h * sizeof(int), SEEK_SET);
    int pos;
    fread(&pos, sizeof(int), 1, fph);

    FILE *fpd = fopen(dados, "rb+");
    if (!fpd) {
        fclose(fph);
        //arquivo de dados ainda não existe → cria
        fpd = fopen(dados, "wb");
        if (!fpd) { perror("FH_insere: dados"); exit(1); }
        fclose(fpd);
        fpd = fopen(dados, "rb+");
        if (!fpd) { perror("FH_insere: dados rb+"); exit(1); }
    }

    int ant = -1;
    int prim_livre = -1;    //1º slot com status=0 no chain
    TF aux;

    //percorre o chain da hash
    int cur = pos;
    while (cur != -1) {
        fseek(fpd, cur, SEEK_SET);
        fread(&aux, sizeof(TF), 1, fpd);

        if (strcmp(aux.pessoa, pessoa) == 0) {
            /* pessoa já existe → atualiza e sai */
            strncpy(aux.relacao, relacao, FH_REL_MAX - 1);
            aux.relacao[FH_REL_MAX - 1] = '\0';
            strncpy(aux.papel, papel, FH_ROLE_MAX - 1);
            aux.papel[FH_ROLE_MAX - 1] = '\0';
            aux.status = 1;
            fseek(fpd, cur, SEEK_SET);
            fwrite(&aux, sizeof(TF), 1, fpd);
            fclose(fpd); fclose(fph);
            return;
        }
        if (aux.status == 0 && prim_livre == -1)
            prim_livre = cur;

        ant = cur;
        cur = aux.prox;
    }

    //monta novo registro
    TF novo;
    memset(&novo, 0, sizeof(TF));
    strncpy(novo.pessoa,  pessoa,  FH_NOME_MAX - 1);
    strncpy(novo.relacao, relacao, FH_REL_MAX  - 1);
    strncpy(novo.papel,   papel,   FH_ROLE_MAX - 1);
    novo.prox   = -1;
    novo.status = 1;

    int nova_pos;

    if (prim_livre != -1) {
        //reaproveitamento de slot removido
        nova_pos = prim_livre;
        //mantém o prox original do slot reutilizado
        fseek(fpd, prim_livre, SEEK_SET);
        TF slot;
        fread(&slot, sizeof(TF), 1, fpd);
        novo.prox = slot.prox;
        fseek(fpd, prim_livre, SEEK_SET);
        fwrite(&novo, sizeof(TF), 1, fpd);
    } else {
        //acrescenta no final do arquivo de dados
        fseek(fpd, 0L, SEEK_END);
        nova_pos = (int)ftell(fpd);
        fwrite(&novo, sizeof(TF), 1, fpd);

        //encadeia: atualiza prox do anterior ou cabeçalho
        if (ant != -1) {
            fseek(fpd, ant, SEEK_SET);
            fread(&aux, sizeof(TF), 1, fpd);
            aux.prox = nova_pos;
            fseek(fpd, ant, SEEK_SET);
            fwrite(&aux, sizeof(TF), 1, fpd);
        } else {
            //hash estava vazio → atualiza tabHash[h]
            fseek(fph, h * sizeof(int), SEEK_SET);
            fwrite(&nova_pos, sizeof(int), 1, fph);
        }
    }

    fclose(fpd);
    fclose(fph);
}


//FH_busca_pessoa
int FH_busca_pessoa(const char *tabHash, const char *dados, int m,
                    const char *titulo, int ano,
                    const char *pessoa, TF *out)
{
    int h = CalculoDeHash(titulo, ano, m);

    FILE *fph = fopen(tabHash, "rb");
    if (!fph) { perror("FH_busca_pessoa: tabHash"); exit(1); }
    fseek(fph, h * sizeof(int), SEEK_SET);
    int pos;
    fread(&pos, sizeof(int), 1, fph);
    fclose(fph);

    if (pos == -1) return 0;

    FILE *fpd = fopen(dados, "rb");
    if (!fpd) { perror("FH_busca_pessoa: dados"); exit(1); }

    TF aux;
    int cur = pos;
    while (cur != -1) {
        fseek(fpd, cur, SEEK_SET);
        fread(&aux, sizeof(TF), 1, fpd);
        if (aux.status && strcmp(aux.pessoa, pessoa) == 0) {
            if (out) *out = aux;
            fclose(fpd);
            return 1;
        }
        cur = aux.prox;
    }
    fclose(fpd);
    return 0;
}

//FH_retira_pessoa
int FH_retira_pessoa(const char *tabHash, const char *dados, int m,
                     const char *titulo, int ano,
                     const char *pessoa)
{
    int h = CalculoDeHash(titulo, ano, m);

    FILE *fph = fopen(tabHash, "rb");
    if (!fph) { perror("FH_retira_pessoa: tabHash"); exit(1); }
    fseek(fph, h * sizeof(int), SEEK_SET);
    int pos;
    fread(&pos, sizeof(int), 1, fph);
    fclose(fph);

    if (pos == -1) return 0;

    FILE *fpd = fopen(dados, "rb+");
    if (!fpd) { perror("FH_retira_pessoa: dados"); exit(1); }

    TF aux;
    int cur = pos;
    while (cur != -1) {
        fseek(fpd, cur, SEEK_SET);
        fread(&aux, sizeof(TF), 1, fpd);
        if (aux.status && strcmp(aux.pessoa, pessoa) == 0) {
            aux.status = 0;
            fseek(fpd, cur, SEEK_SET);
            fwrite(&aux, sizeof(TF), 1, fpd);
            fclose(fpd);
            return 1;
        }
        cur = aux.prox;
    }
    fclose(fpd);
    return 0;
}

//FH_listar_filme
void FH_listar_filme(const char *tabHash, const char *dados, int m,
                     const char *titulo, int ano)
{
    int h = CalculoDeHash(titulo, ano, m);
    printf("=== \"%s\" (%d)  [bucket %d] ===\n", titulo, ano, h);

    FILE *fph = fopen(tabHash, "rb");
    if (!fph) { perror("FH_listar_filme: tabHash"); exit(1); }
    fseek(fph, h * sizeof(int), SEEK_SET);
    int pos;
    fread(&pos, sizeof(int), 1, fph);
    fclose(fph);

    if (pos == -1) { printf("  (nenhum registro)\n"); return; }

    FILE *fpd = fopen(dados, "rb");
    if (!fpd) { perror("FH_listar_filme: dados"); exit(1); }

    TF aux;
    int cur = pos, count = 0;
    while (cur != -1) {
        fseek(fpd, cur, SEEK_SET);
        fread(&aux, sizeof(TF), 1, fpd);
        if (aux.status) {
            count++;
            if (strlen(aux.papel) > 0)
                printf("  [%d] %-30s | %-12s | papel: %s\n",
                       count, aux.pessoa, aux.relacao, aux.papel);
            else
                printf("  [%d] %-30s | %s\n",
                       count, aux.pessoa, aux.relacao);
        }
        cur = aux.prox;
    }
    if (count == 0) printf("  (todos os registros foram removidos)\n");
    fclose(fpd);
}

/* =========================================================
 *  _parse_linha
 *   Parseia uma linha do Relationships.txt e extrai:
 *     pessoa, relacao, titulo, papel
 *   Formato:
 *     START Person | <pessoa> | <relacao> | END Movie | <titulo> [| role: <papel>]
 *   Retorna 1 se OK, 0 se linha inválida.
 * ========================================================= */
static int _parse_linha(const char *linha,
                        char *pessoa,  char *relacao,
                        char *titulo,  char *papel)
{
    /* Copia para não destruir o original */
    char buf[512];
    strncpy(buf, linha, 511);
    buf[511] = '\0';

    /* Remove '\n' e '\r' */
    char *p = buf + strlen(buf) - 1;
    while (p >= buf && (*p == '\n' || *p == '\r' || *p == ' '))
        *p-- = '\0';

    /* Tokeniza por " | " */
    const char *sep = " | ";
    char *tok = strtok(buf, "|");   /* "START Person " */
    if (!tok) return 0;

    tok = strtok(NULL, "|");        /* " <pessoa> "    */
    if (!tok) return 0;
    while (*tok == ' ') tok++;
    p = tok + strlen(tok) - 1;
    while (p >= tok && *p == ' ') *p-- = '\0';
    strncpy(pessoa, tok, FH_NOME_MAX - 1);

    tok = strtok(NULL, "|");        /* " <relacao> "   */
    if (!tok) return 0;
    while (*tok == ' ') tok++;
    p = tok + strlen(tok) - 1;
    while (p >= tok && *p == ' ') *p-- = '\0';
    strncpy(relacao, tok, FH_REL_MAX - 1);

    tok = strtok(NULL, "|");        /* " END Movie "   */
    if (!tok) return 0;

    tok = strtok(NULL, "|");        /* " <titulo> "    */
    if (!tok) return 0;
    while (*tok == ' ') tok++;
    p = tok + strlen(tok) - 1;
    while (p >= tok && *p == ' ') *p-- = '\0';
    strncpy(titulo, tok, FH_NOME_MAX - 1);

    /* papel é opcional */
    papel[0] = '\0';
    tok = strtok(NULL, "|");        /* " role: <papel> " ou NULL */
    if (tok) {
        while (*tok == ' ') tok++;
        /* remove prefixo "role: " ou "roles: " */
        if (strncmp(tok, "role: ", 6) == 0)       tok += 6;
        else if (strncmp(tok, "roles: ", 7) == 0)  tok += 7;
        p = tok + strlen(tok) - 1;
        while (p >= tok && *p == ' ') *p-- = '\0';
        strncpy(papel, tok, FH_ROLE_MAX - 1);
    }

    (void)sep; /* evita warning de variável não usada */
    return 1;
}

//FH_carregar_arquivo
int FH_carregar_arquivo(const char *tabHash, const char *dados, int m,
                        const char *relFile, int ano_padrao)
{
    FILE *fp = fopen(relFile, "r");
    if (!fp) { perror("FH_carregar_arquivo"); return -1; }

    char linha[512];
    char pessoa[FH_NOME_MAX], relacao[FH_REL_MAX];
    char titulo[FH_NOME_MAX], papel[FH_ROLE_MAX];
    int inseridos = 0;

    while (fgets(linha, sizeof(linha), fp)) {
        if (strlen(linha) < 5) continue;   /* linha vazia */
        if (_parse_linha(linha, pessoa, relacao, titulo, papel)) {
            FH_insere(tabHash, dados, m,
                      titulo, ano_padrao,
                      pessoa, relacao, papel);
            inseridos++;
        }
    }
    fclose(fp);
    return inseridos;
}

//FH_imprime_tudo
void FH_imprime_tudo(const char *tabHash, const char *dados, int m) {
    FILE *fph = fopen(tabHash, "rb");
    if (!fph) { perror("FH_imprime_tudo: tabHash"); exit(1); }

    int *idx = (int *)malloc(m * sizeof(int));
    if (!idx) { fclose(fph); exit(1); }
    fread(idx, sizeof(int), m, fph);
    fclose(fph);

    FILE *fpd = fopen(dados, "rb");
    if (!fpd) { free(idx); perror("FH_imprime_tudo: dados"); exit(1); }

    printf("\n=== DUMP COMPLETO DA TABELA HASH (%d buckets) ===\n\n", m);
    for (int i = 0; i < m; i++) {
        if (idx[i] == -1) continue;
        printf("Bucket %d:\n", i);
        int cur = idx[i];
        TF x;
        while (cur != -1) {
            fseek(fpd, cur, SEEK_SET);
            fread(&x, sizeof(TF), 1, fpd);
            printf("  $%06d  pessoa=%-30s  rel=%-12s  papel=%-20s  st=%d  prox=%d\n",
                   cur, x.pessoa, x.relacao, x.papel, x.status, x.prox);
            cur = x.prox;
        }
    }
    printf("\n");
    free(idx);
    fclose(fpd);
}
