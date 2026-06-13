#include "TARVBM_bin.h"
#include <string.h>

void escrever_dados_arquivo(NO_BIN *no, int t);
void gerar_nome_folha(OFFSET offset, char *saida);
NO_BIN *ler_dados_folha(NO_BIN *folha_principal, int t);

#define META_SIZE (sizeof(int) + sizeof(OFFSET))

//manipulação do metadata
void escrever_metadata(TARVBM_BIN *arv) {
    fseek(arv->arquivo, 0, SEEK_SET);
    fwrite(&arv->t, sizeof(int), 1, arv->arquivo);
    fwrite(&arv->raiz_offset, sizeof(arv->raiz_offset), 1, arv->arquivo);
}
void ler_metadata(TARVBM_BIN *arv) {
    fseek(arv->arquivo, 0, SEEK_SET);
    fread(&arv->t, sizeof(int), 1, arv->arquivo);
    fread(&arv->raiz_offset, sizeof(arv->raiz_offset), 1, arv->arquivo);
}

//manipulação do no temporario em MP
void escrever_no(FILE *arq, NO_BIN *no, int t) {
    if (!no) return;
    if (no->folha) {
        escrever_dados_arquivo(no, t);
    }
    int i;
    int max_chaves = 2 * t - 1;
    int max_filhos = 2 * t;

    fseek(arq, no->offset, SEEK_SET);

    fwrite(&no->nchaves, sizeof(int), 1, arq);
    fwrite(&no->folha, sizeof(int), 1, arq);

    fwrite(no->folha_nome, sizeof(char), MAX_TAM_NOME, arq);
    fwrite(no->prox_folha_nome, sizeof(char), MAX_TAM_NOME, arq);

    for (i = 0; i < max_chaves; i++) {
        fwrite(no->chave[i], sizeof(char), MAX_TAM_CHAVE, arq);
    }
    fwrite(no->offset_chaves, sizeof(OFFSET), 2 * t - 1, arq);
    fwrite(no->offset_filhos, sizeof(OFFSET), max_filhos, arq);
}
void liberar_no(NO_BIN *no, int t) {
    int i;
    int max_chaves = 2 * t - 1;

    if (no) {
        if (no->chave) {
            for (i = 0; i < max_chaves; i++) {
                free(no->chave[i]);
            }
            free(no->chave);
        }
        free(no->offset_chaves);
        free(no->offset_filhos);
        free(no->folha_nome);
        free(no->prox_folha_nome);
        free(no);
    }
}
NO_BIN* criar_no(long offset, int t, int folha) {
    int i;
    int max_chaves = 2 * t - 1;
    int max_filhos = 2 * t;

    NO_BIN *no = malloc(sizeof(NO_BIN));

    no->offset = offset;
    no->nchaves = 0;
    no->folha = folha;
    no->prox_folha_nome = calloc(MAX_TAM_NOME, sizeof(char));
    no->folha_nome = calloc(MAX_TAM_NOME, sizeof(char));
    no->chave = calloc(max_chaves, sizeof(char*));

    for (i = 0; i < max_chaves; i++) {
        no->chave[i] = calloc(MAX_TAM_CHAVE, sizeof(char));
    }
    no->offset_chaves = malloc(sizeof(OFFSET) * (2 * t - 1));
    for(int i = 0; i < 2 * t - 1; i++){
        no->offset_chaves[i] = -1;
    }
    
    no->offset_filhos = malloc(max_filhos*sizeof(OFFSET));
    for (i = 0; i < max_filhos; i++) {
        no->offset_filhos[i] = -1;
    }

    if (folha) {
        gerar_nome_folha(offset, no->folha_nome);
    }
    return no;
}
NO_BIN* ler_no(FILE *arq, OFFSET offset, int t) {
    if (offset < 0) return NULL;
    int max_chaves = 2 * t - 1;
    
    NO_BIN *no = (NO_BIN*)malloc(sizeof(NO_BIN));
    no->offset = offset;
    no->folha_nome = calloc(MAX_TAM_NOME, sizeof(char));
    no->prox_folha_nome = calloc(MAX_TAM_NOME, sizeof(char));

    no->chave = malloc(sizeof(char*) * max_chaves);
    for (int i = 0; i < max_chaves; i++) {
        no->chave[i] = calloc(MAX_TAM_CHAVE, sizeof(char));
    }
    no->offset_chaves = malloc(sizeof(OFFSET) * max_chaves);
    no->offset_filhos = (OFFSET*)malloc(sizeof(OFFSET) * (2*t));
    
    fseek(arq, offset, SEEK_SET);
    fread(&no->nchaves, sizeof(int), 1, arq);
    fread(&no->folha, sizeof(int), 1, arq);
    fread(no->folha_nome, sizeof(char), MAX_TAM_NOME, arq);
    fread(no->prox_folha_nome, sizeof(char), MAX_TAM_NOME, arq);
    for (int i = 0; i < max_chaves; i++) {
        fread(no->chave[i], sizeof(char), MAX_TAM_CHAVE, arq);
    }
    fread(no->offset_chaves, sizeof(OFFSET), max_chaves, arq);
    fread(no->offset_filhos, sizeof(OFFSET), 2*t, arq);
    
    return no;
}
OFFSET obter_proximo_offset(FILE *arq) {
    fseek(arq, 0, SEEK_END);
    return ftell(arq);
}

//manipulação das folhas em arquivos
void gerar_nome_folha(OFFSET offset, char *saida){
    sprintf(saida, "folha_%ld.bin", offset);
}
void escrever_dados_arquivo(NO_BIN *no, int t){
    FILE *f = fopen(no->folha_nome, "wb");
    if (!f) return;

    fwrite(&no->nchaves, sizeof(int), 1, f);
    fwrite(no->offset_chaves, sizeof(OFFSET), 2 * t - 1, f);
    fwrite(no->prox_folha_nome, sizeof(char), MAX_TAM_NOME, f);

    for (int i = 0; i < no->nchaves; i++) {
        fwrite(no->chave[i], sizeof(char), MAX_TAM_CHAVE, f);
    }

    fclose(f);
}
NO_BIN *ler_dados_folha(NO_BIN *folha_principal, int t){
    if (!folha_principal || !folha_principal->folha) return NULL;

    FILE *f = fopen(folha_principal->folha_nome, "rb");
    if (!f) return NULL;

    NO_BIN *no = criar_no(folha_principal->offset, t, 1);

    strcpy(no->folha_nome, folha_principal->folha_nome);

    fread(&no->nchaves, sizeof(int), 1, f);
    fread(no->offset_chaves, sizeof(OFFSET), 2 * t - 1, f);
    fread(no->prox_folha_nome, sizeof(char), MAX_TAM_NOME, f);

    for (int i = 0; i < no->nchaves; i++) {
        fread(no->chave[i], sizeof(char), MAX_TAM_CHAVE, f);
    }

    fclose(f);
    return no;
}
int apagar_arquivo_folha(NO_BIN *no) {
    if (!no || !no->folha || !no->folha_nome) return 0;

    printf("Tentando apagar arquivo: [%s]\n", no->folha_nome);

    if (remove(no->folha_nome) == 0) {
        printf("Arquivo apagado\n");
        return 1;
    }

    perror("Erro ao apagar arquivo");
    return 0;
}
void destruir_no(NO_BIN *no, int t) {
    if (no && no->folha) {
        apagar_arquivo_folha(no);
    }

    liberar_no(no, t);
}

//criar, abrir e fechar
TARVBM_BIN* TARVBM_BIN_criar(const char *nome_arquivo, int t){
    TARVBM_BIN *arv = (TARVBM_BIN*)malloc(sizeof(TARVBM_BIN));
    arv->nome_arquivo = (char*)malloc(strlen(nome_arquivo) + 1);
    strcpy(arv->nome_arquivo, nome_arquivo);
    arv->t = t;
    arv->raiz_offset = 0;

    arv->arquivo = fopen(arv->nome_arquivo, "w+b");
    if (!arv->arquivo) {
        printf("Erro ao abrir arquivo\n");
        free(arv->nome_arquivo);
        free(arv);
        return NULL;
    }
    arv->raiz_offset = META_SIZE;
    escrever_metadata(arv);

    NO_BIN *raiz = criar_no(arv->raiz_offset, t, 1);
    gerar_nome_folha(raiz->offset, raiz->folha_nome);

    escrever_no(arv->arquivo, raiz, arv->t);
    liberar_no(raiz, arv->t);

    return arv;
}
TARVBM_BIN* TARVBM_BIN_abrir(const char *nome_arquivo) {
    TARVBM_BIN *arv = (TARVBM_BIN*)malloc(sizeof(TARVBM_BIN));
    arv->nome_arquivo = (char*)malloc(strlen(nome_arquivo) + 1);
    strcpy(arv->nome_arquivo, nome_arquivo);
    
    arv->arquivo = fopen(arv->nome_arquivo, "r+b");
    if (!arv->arquivo) {
        printf("Erro ao abrir arquivo\n");
        free(arv->nome_arquivo);
        free(arv);
        return NULL;
    }
    
    ler_metadata(arv);
    return arv;
}
void TARVBM_BIN_fechar(TARVBM_BIN *arv) {
    if (arv) {
        if (arv->arquivo) {
            escrever_metadata(arv);
            fclose(arv->arquivo);
        }
        free(arv->nome_arquivo);
        free(arv);
    }
}


//busca
OFFSET busca_recursiva(FILE *arq, OFFSET offset, const char *chave, int t) {
    if (offset < 0) return -1;
    NO_BIN *no = ler_no(arq, offset, t);
    if (!no) return -1;
    int i = 0;
    while (i < no->nchaves && strcmp(chave, no->chave[i]) >= 0) {
        i++;
    }
    if (no->folha) {
        if (i > 0 && strcmp(chave, no->chave[i - 1]) == 0) {
            liberar_no(no, t);
            return offset; // offset da folha onde está a chave
        }

        liberar_no(no, t);
        return -1;
    }
    OFFSET prox = no->offset_filhos[i];
    liberar_no(no, t);

    return busca_recursiva(arq, prox, chave, t);
}
OFFSET TARVBM_BIN_busca(TARVBM_BIN *arvore, const char *chave) {
    if (!arvore || !chave) return -1;
    if (arvore->raiz_offset < 0) return -1;

    return busca_recursiva(arvore->arquivo, arvore->raiz_offset, chave, arvore->t);
}

//inserção
void dividir_filho(FILE *arq, NO_BIN *x, int i, OFFSET offset_filho, int t){
    NO_BIN *y = ler_no(arq, offset_filho, t);

    OFFSET novo_offset = obter_proximo_offset(arq);
    NO_BIN *z = criar_no(novo_offset, t, y->folha);
    int j;
    if(!y->folha){
        z->nchaves = t-1;
        for(j=0;j<t-1;j++){
            strcpy(z->chave[j], y->chave[j+t]);
        }
        for(j=0; j<t; j++){
            z->offset_filhos[j] = y->offset_filhos[j+t];
            y->offset_filhos[j+t] = -1;
        }
    }
    else{
        z->nchaves = t;

        for(j = 0; j < t; j++) {
            strcpy(z->chave[j], y->chave[j + t - 1]);
            z->offset_chaves[j] = y->offset_chaves[j + t - 1];

            y->offset_chaves[j + t - 1] = -1;
        }

        strcpy(z->prox_folha_nome, y->prox_folha_nome);
        strcpy(y->prox_folha_nome, z->folha_nome);
    }
    y->nchaves = t-1;
    for(j=x->nchaves; j>=i; j--) x->offset_filhos[j+1]=x->offset_filhos[j];
    x->offset_filhos[i] = novo_offset;
    for(j=x->nchaves; j>=i; j--) strcpy(x->chave[j],x->chave[j-1]);
    if (y->folha) {
        strcpy(x->chave[i - 1], z->chave[0]);
    } else {
        strcpy(x->chave[i - 1], y->chave[t - 1]);
    }
    x->nchaves++;

    escrever_no(arq, y, t);
    escrever_no(arq, z, t);
    escrever_no(arq, x, t);

    liberar_no(y, t);
    liberar_no(z, t);
}
void insere_nao_completo(FILE* arq, OFFSET offset, const char* chave, OFFSET offset_pessoa, int t){
    NO_BIN *x = ler_no(arq, offset, t);
    int i = x->nchaves - 1;
    if(x->folha){
        while((i>=0) && (strcmp(chave, x->chave[i]) < 0)){
            strcpy(x->chave[i+1], x->chave[i]);
            x->offset_chaves[i + 1] = x->offset_chaves[i];
            i--;
        }
        strcpy(x->chave[i+1], chave);
        x->offset_chaves[i + 1] = offset_pessoa;
        x->nchaves++;

        escrever_no(arq, x, t);
        liberar_no(x, t);
        return;
    }
    while((i>=0) && (strcmp(chave,x->chave[i]) < 0)) i--;
    i++;
    NO_BIN *filho = ler_no(arq, x->offset_filhos[i], t);
    if(filho->nchaves == ((2*t) - 1)){
        liberar_no(filho, t);
        dividir_filho(arq, x, i + 1, x->offset_filhos[i], t);

        if(strcmp(chave,x->chave[i]) > 0) i++;
    } else {
        liberar_no(filho, t);
    }
    OFFSET prox = x->offset_filhos[i];

    liberar_no(x, t);

    insere_nao_completo(arq, prox, chave, offset_pessoa, t);
}

void TARVBM_BIN_insere(TARVBM_BIN *arvore, const char *chave, OFFSET offset_pessoa) {
    if (!arvore || !chave) return;

    if (arvore->raiz_offset == -1) {
        OFFSET novo_offset = obter_proximo_offset(arvore->arquivo);
        NO_BIN *nova_raiz = criar_no(novo_offset, arvore->t, 1);

        strcpy(nova_raiz->chave[0], chave);
        nova_raiz->offset_chaves[0] = offset_pessoa;
        nova_raiz->nchaves = 1;

        arvore->raiz_offset = novo_offset;

        escrever_no(arvore->arquivo, nova_raiz, arvore->t);
        escrever_metadata(arvore);

        liberar_no(nova_raiz, arvore->t);
        return;
    }
    
    if (TARVBM_BIN_busca(arvore, chave) != -1) {
        return;
    }
    NO_BIN *raiz = ler_no(arvore->arquivo, arvore->raiz_offset, arvore->t);

    if (raiz->nchaves == (2 * arvore->t) - 1) {
        OFFSET novo_raiz_offset = obter_proximo_offset(arvore->arquivo);

        NO_BIN *nova_raiz = criar_no(novo_raiz_offset, arvore->t, 0);
        nova_raiz->nchaves = 0;
        nova_raiz->folha = 0;
        nova_raiz->offset_filhos[0] = arvore->raiz_offset;

        escrever_no(arvore->arquivo, nova_raiz, arvore->t);

        dividir_filho(arvore->arquivo, nova_raiz, 1, arvore->raiz_offset, arvore->t);

        arvore->raiz_offset = novo_raiz_offset;
        escrever_metadata(arvore);

        insere_nao_completo( arvore->arquivo, novo_raiz_offset, chave, offset_pessoa, arvore->t);
        liberar_no(nova_raiz, arvore->t);
    } else {
        insere_nao_completo( arvore->arquivo, arvore->raiz_offset, chave, offset_pessoa, arvore->t);
    }

    liberar_no(raiz, arvore->t);
}


//remoção
int menor_chave_subarvore(TARVBM_BIN *arv, OFFSET offset_no, int t, char *saida){
    if (offset_no == -1) return 0;

    NO_BIN *no = ler_no(arv->arquivo, offset_no, t);
    if (!no) return 0;

    while (!no->folha) {
        OFFSET prox = no->offset_filhos[0];
        liberar_no(no, t);

        if (prox == -1) return 0;

        no = ler_no(arv->arquivo, prox, t);
        if (!no) return 0;
    }

    if (no->nchaves <= 0) {
        liberar_no(no, t);
        return 0;
    }

    strcpy(saida, no->chave[0]);
    liberar_no(no, t);
    return 1;
}

void atualizar_separadores(TARVBM_BIN *arv, NO_BIN *no, int t){
    if (!no || no->folha) return;

    for (int k = 0; k < no->nchaves; k++) {
        char menor[MAX_TAM_CHAVE];

        if (menor_chave_subarvore(arv, no->offset_filhos[k + 1], t, menor)) {
            strcpy(no->chave[k], menor);
        }
    }
}
OFFSET remover(TARVBM_BIN *arv, OFFSET offset_no, const char *chave, int t){
    if (offset_no == -1) return -1;

    NO_BIN *no = ler_no(arv->arquivo, offset_no, t);
    if (!no) return -1;

    OFFSET prox_remocao = -1;
    OFFSET pessoa_removida = -1;
    int destruir_y = 0;
    int i = 0;
    while (i < no->nchaves && strcmp(no->chave[i], chave) < 0) {
        i++;
    }

    if(no->folha){
        pessoa_removida = -1;
        if(i < no->nchaves && strcmp(no->chave[i], chave) == 0){
            pessoa_removida = no->offset_chaves[i];
            for(int j = i; j < no->nchaves - 1; j++){
                strcpy(no->chave[j], no->chave[j + 1]);
                no->offset_chaves[j] = no->offset_chaves[j + 1];
            }
            no->nchaves--;
            no->offset_chaves[no->nchaves] = -1;

            if (no->nchaves == 0 && no->offset == arv->raiz_offset) {
                arv->raiz_offset = -1;
                escrever_metadata(arv);
                destruir_no(no, t);
                return pessoa_removida;
            }

            escrever_no(arv->arquivo, no, t);
        }
        liberar_no(no, t);
        return pessoa_removida;
    }

    if (i < no->nchaves && strcmp(chave, no->chave[i]) == 0) {
        i++;
    }
    NO_BIN *y = ler_no(arv->arquivo, no->offset_filhos[i], t);
    if (!y) {
        liberar_no(no, t);
        return offset_no;
    }

    if(y->nchaves > t-1){ //pode descer nesse filho
        OFFSET prox = y->offset;
        liberar_no(y, t);

        OFFSET ret = remover(arv, prox, chave, t);

        atualizar_separadores(arv, no, t);
        escrever_no(arv->arquivo, no, t);
        liberar_no(no, t);
        return ret;
    }

    NO_BIN *z_d = NULL;
    NO_BIN *z_e = NULL;
    if(i < no->nchaves) z_d = ler_no(arv->arquivo, no->offset_filhos[i + 1], t);
    if(i > 0) z_e = ler_no(arv->arquivo, no->offset_filhos[i - 1], t);

    if (z_d && z_d->nchaves >= t) { // empresta da direita
        char separador[MAX_TAM_CHAVE];
        char primeira_z[MAX_TAM_CHAVE];
        strcpy(separador, no->chave[i]);
        strcpy(primeira_z, z_d->chave[0]);

        if (!y->folha) {
            strcpy(y->chave[y->nchaves], separador);
            y->offset_filhos[y->nchaves + 1] = z_d->offset_filhos[0];
            y->nchaves++;
            strcpy(no->chave[i], primeira_z);
        } else {
            strcpy(y->chave[y->nchaves], primeira_z);
            y->offset_chaves[y->nchaves] = z_d->offset_chaves[0];
            y->nchaves++;
        }

        for (int j = 0; j < z_d->nchaves - 1; j++) {
            strcpy(z_d->chave[j], z_d->chave[j + 1]);

            if (z_d->folha) {
                z_d->offset_chaves[j] = z_d->offset_chaves[j + 1];
            }
        }
        if (z_d->folha){
            z_d->offset_chaves[z_d->nchaves - 1] = -1;
        }else{
            for (int j = 0; j < z_d->nchaves; j++) {
                z_d->offset_filhos[j] = z_d->offset_filhos[j + 1];
            }
        }
        
        z_d->nchaves--;
        if (y->folha) { //separador vira o depois do primeiro_z
            strcpy(no->chave[i], z_d->chave[0]);
        }
        
        escrever_no(arv->arquivo, y, t);
        escrever_no(arv->arquivo, z_d, t);
        escrever_no(arv->arquivo, no, t);

        prox_remocao = y->offset;

        liberar_no(z_d, t);
        if (z_e) liberar_no(z_e, t);
    }
    else if (z_e && z_e->nchaves >= t) { // empresta da esquerda
        char separador[MAX_TAM_CHAVE];
        char ultima_z[MAX_TAM_CHAVE];
        strcpy(separador, no->chave[i - 1]);
        strcpy(ultima_z, z_e->chave[z_e->nchaves - 1]);

        for (int j = y->nchaves; j > 0; j--) {
            strcpy(y->chave[j], y->chave[j - 1]);
        }
        if (y->folha) { // se for folha, desloco também o offset dos elementos
            for (int j = y->nchaves; j > 0; j--) {
                y->offset_chaves[j] = y->offset_chaves[j - 1];
            }
        }

        if (!y->folha) {
            for (int j = y->nchaves + 1; j > 0; j--) {
                y->offset_filhos[j] = y->offset_filhos[j - 1];
            }

            strcpy(y->chave[0], separador);
            y->offset_filhos[0] = z_e->offset_filhos[z_e->nchaves];
        } else {
            strcpy(y->chave[0], ultima_z);
            y->offset_chaves[0] = z_e->offset_chaves[z_e->nchaves - 1];
            strcpy(no->chave[i - 1], ultima_z);
            z_e->offset_chaves[z_e->nchaves - 1] = -1;
        }
        
        y->nchaves++;
        z_e->nchaves--;

        escrever_no(arv->arquivo, y, t);
        escrever_no(arv->arquivo, z_e, t);
        escrever_no(arv->arquivo, no, t);

        prox_remocao = y->offset;

        if (z_d) liberar_no(z_d, t);
        liberar_no(z_e, t);
    }
    else { // caso 3B: fusao
        int j;
        if (z_d) { // funde y com irmão da direita
            int pos = y->nchaves;
            if (!y->folha) {
                strcpy(y->chave[pos], no->chave[i]);
                pos++;
            }
            for (j = 0; j < z_d->nchaves; j++) {
                strcpy(y->chave[pos + j], z_d->chave[j]);

                if (y->folha) {
                    y->offset_chaves[pos + j] = z_d->offset_chaves[j];
                }
            }
            if (!y->folha) {
                for (j = 0; j <= z_d->nchaves; j++) {
                    y->offset_filhos[pos + j] = z_d->offset_filhos[j];
                }
            } else {
                strcpy(y->prox_folha_nome, z_d->prox_folha_nome);
            }

            y->nchaves = pos + z_d->nchaves;
            for (j = i; j < no->nchaves - 1; j++) {
                strcpy(no->chave[j], no->chave[j + 1]);
                no->offset_filhos[j + 1] = no->offset_filhos[j + 2];
            }

            no->nchaves--;
            no->offset_filhos[no->nchaves + 1] = -1;

            prox_remocao = y->offset;

            escrever_no(arv->arquivo, y, t);
            escrever_no(arv->arquivo, no, t);

            destruir_no(z_d, t);
            if (z_e) liberar_no(z_e, t);
        } 
        else if (z_e) { // funde irmão da esquerda com y
            int pos = z_e->nchaves;

            if (!z_e->folha) {
                strcpy(z_e->chave[pos], no->chave[i - 1]);
                pos++;
            }

            for (j = 0; j < y->nchaves; j++) {
                strcpy(z_e->chave[pos + j], y->chave[j]);

                if (z_e->folha) {
                    z_e->offset_chaves[pos + j] = y->offset_chaves[j];
                }
            }

            if (!z_e->folha) {
                for (j = 0; j <= y->nchaves; j++) {
                    z_e->offset_filhos[pos + j] = y->offset_filhos[j];
                }
            } else {
                strcpy(z_e->prox_folha_nome, y->prox_folha_nome);
            }

            z_e->nchaves = pos + y->nchaves;

            for (j = i - 1; j < no->nchaves - 1; j++) {
                strcpy(no->chave[j], no->chave[j + 1]);
                no->offset_filhos[j + 1] = no->offset_filhos[j + 2];
            }

            no->nchaves--;
            no->offset_filhos[no->nchaves + 1] = -1;

            prox_remocao = z_e->offset;
            destruir_y = 1;

            escrever_no(arv->arquivo, z_e, t);
            escrever_no(arv->arquivo, no, t);

            liberar_no(z_e, t);
            if (z_d) liberar_no(z_d, t);
        }
    }
    if (prox_remocao == -1) {
        prox_remocao = y->offset;
    }

    if (destruir_y) {
        destruir_no(y, t);
    } else {
        liberar_no(y, t);
    }

    /*
     * Se o nó atual é a raiz e ficou sem chaves após fusão,
     * a nova raiz deve ser o primeiro filho.
     */
    if (no->nchaves == 0 && no->offset == arv->raiz_offset) {
        OFFSET nova_raiz = no->offset_filhos[0];

        arv->raiz_offset = nova_raiz;
        escrever_metadata(arv);

        liberar_no(no, t);

        if (nova_raiz == -1) {
            return -1;
        }

        return remover(arv, nova_raiz, chave, t);
    }

    atualizar_separadores(arv, no, t);
    escrever_no(arv->arquivo, no, t);

    OFFSET ret = remover(arv, prox_remocao, chave, t);

    NO_BIN *no_atualizado = ler_no(arv->arquivo, offset_no, t);
    if (no_atualizado) {
        atualizar_separadores(arv, no_atualizado, t);
        escrever_no(arv->arquivo, no_atualizado, t);
        liberar_no(no_atualizado, t);
    }

    liberar_no(no, t);
    return ret;
}
OFFSET TARVBM_BIN_remove(TARVBM_BIN *arvore, char *chave){
    if (!arvore || !chave) return -1;
    if (arvore->raiz_offset < 0) return -1;
    if (TARVBM_BIN_busca(arvore, chave) == -1) return -1;

    OFFSET ret = remover(arvore, arvore->raiz_offset, chave, arvore->t);
    escrever_metadata(arvore);
    return ret; //retorna o offset do elemento retirado no arquivo dele
}


//impressão
void imprimir_recursivo(FILE *arq, OFFSET offset, int andar, int t) {
    if (offset < 0) return;
    
    NO_BIN *no = ler_no(arq, offset, t);
    if (!no) return;
    
    if (!no->folha) {
        imprimir_recursivo(arq, no->offset_filhos[no->nchaves], andar + 1, t);
    }
    
    int i, j;
    for (i = no->nchaves - 1; i >= 0; i--) {
        for (j = 0; j <= andar; j++) printf("\t\t\t");
        printf("%s\n", no->chave[i]);
        if (!no->folha) {
            imprimir_recursivo(arq, no->offset_filhos[i], andar + 1, t);
        }
    }
    
    liberar_no(no, t);
}

void TARVBM_BIN_imprime(TARVBM_BIN *arvore) {
    printf("\n=== Arvore B+ em Memoria Secundaria ===\n");
    imprimir_recursivo(arvore->arquivo, arvore->raiz_offset, 0, arvore->t);
    printf("\n");
}

void num_folhas(FILE* arq, int* cont, OFFSET end, int t){
    if(end<0) return;
    NO_BIN *no = ler_no(arq, end, t);
    if(no->folha){
        (*cont)++;
        liberar_no(no, t);
        return;
    }
    for(int i=0; i<=no->nchaves; i++){
        num_folhas(arq, cont, no->offset_filhos[i], t);
    }
}
int TARVBM_num_folhas(TARVBM_BIN* arv){
    NO_BIN* raiz = ler_no(arv->arquivo, arv->raiz_offset, arv->t);
    int contador = 0;
    num_folhas(arv->arquivo, &contador, raiz->offset, arv->t);
    return contador;
}

long tamanho_arquivo(const char *nome){
    FILE *f = fopen(nome, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long tam = ftell(f);

    fclose(f);
    return tam;
}
OFFSET offset_por_nome_folha(const char *nome){
    OFFSET off;
    if (!nome || strlen(nome) == 0) return -1;

    if (sscanf(nome, "folha_%ld.bin", &off) == 1) {
        return off;
    }

    return -1;
}
OFFSET acha_prim_folha(TARVBM_BIN *arv){
    if (!arv || arv->raiz_offset == -1) return -1;

    NO_BIN *no = ler_no(arv->arquivo, arv->raiz_offset, arv->t);
    if (!no) return -1;

    while (!no->folha) {
        OFFSET prox = no->offset_filhos[0];
        liberar_no(no, arv->t);
        if (prox == -1) return -1;
        no = ler_no(arv->arquivo, prox, arv->t);
        if (!no) return -1;
    }

    OFFSET ret = no->offset;
    liberar_no(no, arv->t);

    return ret;
}
void TARVBM_BIN_imprime_todos(TARVBM_BIN* arv){
    if(!arv || arv->raiz_offset == -1) return;

    OFFSET folha = acha_prim_folha(arv);
    if (folha == -1) return;

    NO_BIN *aux = ler_no(arv->arquivo, folha, arv->t);
    if (!aux) return;

    NO_BIN *f = ler_dados_folha(aux, arv->t);
    liberar_no(aux, arv->t);

    while (f) {
        printf("\nFolha offset %ld | arquivo: %s\n", f->offset, f->folha_nome);

        for (int i = 0; i < f->nchaves; i++) {
            printf("%s, ", f->chave[i]);
        }

        printf("\nprox_folha_nome: %s", f->prox_folha_nome);
        printf("\nTamanho do arquivo: %ld bytes\n", tamanho_arquivo(f->folha_nome));

        char prox_nome[MAX_TAM_NOME];
        strcpy(prox_nome, f->prox_folha_nome);

        liberar_no(f, arv->t);
        f = NULL;

        if (strlen(prox_nome) == 0) break;

        OFFSET prox_offset = offset_por_nome_folha(prox_nome);
        if (prox_offset == -1) break;

        NO_BIN *prox_principal = ler_no(arv->arquivo, prox_offset, arv->t);
        if (!prox_principal) break;

        /*
        * IMPORTANTE:
        * pega os dados pelo arquivo externo,
        * mas mantém o encadeamento vindo do nó principal.
        */
        f = ler_dados_folha(prox_principal, arv->t);

        if (f) {
            strcpy(f->prox_folha_nome, prox_principal->prox_folha_nome);
        }

        liberar_no(prox_principal, arv->t);
    }
}
