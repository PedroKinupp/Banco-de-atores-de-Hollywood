#ifndef FILMEHASH_H
#define FILMEHASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================
 *  FilmeHash
 *
 *  Estrutura lógica:
 *    tabHash  : arquivo de inteiros (tamanho M)
 *               tabHash[h] = posição (byte offset) do 1º
 *               registro do bucket h no arquivo de dados,
 *               ou -1 se o bucket estiver vazio.
 *
 *    dados    : arquivo de registros TF
 *        
 *
 *  Chave de hash: "TÍTULO|ANO"
 * ========================================================= */

#define FH_NOME_MAX  128   /* tamanho máximo de strings     */
#define FH_ROLE_MAX  128
#define FH_REL_MAX    32   /* ACTED_IN, DIRECTED, etc.      */

/* Registro gravado no arquivo de dados */
typedef struct tfregistro {
    char   pessoa[FH_NOME_MAX]; /* nome do ator/diretor/...  */
    char   relacao[FH_REL_MAX]; /* ACTED_IN / DIRECTED / ... */
    char   papel[FH_ROLE_MAX];  /* papel (role), se houver   */
    int    prox;                /* offset do próximo no chain */
    int    status;              /* 1=ativo, 0=removido        */
} TF;

/* ---- Funções públicas ----------------------------------- */

/*
 * CalculoDeHash
 *   Calcula o índice h = hash("titulo|ano") % m
 *   de forma que TODOS que trabalharam no mesmo filme
 *   gerem o mesmo índice.
 *
 *   titulo : nome completo do filme  (ex.: "The Matrix")
 *   ano    : ano de lançamento       (ex.: 1999)
 *   m      : tamanho da tabela hash
 *   Retorno: índice [0, m)
 */
int CalculoDeHash(const char *titulo, int ano, int m);

/*
 * FH_inicializa
 *   Cria (ou recria) os dois arquivos em branco.
 *   tabHash : nome do arquivo-índice
 *   dados   : nome do arquivo de registros
 *   m       : número de buckets
 */
void FH_inicializa(const char *tabHash, const char *dados, int m);

/*
 * FH_insere
 *   Insere a relação (pessoa × filme) na tabela.
 *   Se a pessoa já existe para aquele filme,
 *   atualiza o registro (idempotente).
 */
void FH_insere(const char *tabHash, const char *dados, int m,
               const char *titulo, int ano,
               const char *pessoa, const char *relacao,
               const char *papel);

/*
 * FH_busca_pessoa
 *   Retorna 1 e preenche *out se a pessoa for encontrada
 *   no filme indicado; retorna 0 caso contrário.
 */
int FH_busca_pessoa(const char *tabHash, const char *dados, int m,
                    const char *titulo, int ano,
                    const char *pessoa, TF *out);

/*
 * FH_retira_pessoa
 *   Marca como removido (status=0) o registro da pessoa
 *   no filme indicado.
 *   Retorna 1 se removeu, 0 se não encontrou.
 */
int FH_retira_pessoa(const char *tabHash, const char *dados, int m,
                     const char *titulo, int ano,
                     const char *pessoa);

/*
 * FH_listar_filme
 *   Imprime todos os registros ativos do bucket do filme.
 */
void FH_listar_filme(const char *tabHash, const char *dados, int m,
                     const char *titulo, int ano);

/*
 * FH_carregar_arquivo
 *   Lê o arquivo Relationships.txt e popula a tabela.
 *   Linhas com ano=0 usam ano padrão 0 (sem ano no dataset).
 *   ano_padrao : valor usado quando não há ano no arquivo
 *   Retorna o número de registros inseridos.
 */
int FH_carregar_arquivo(const char *tabHash, const char *dados, int m,
                        const char *relFile, int ano_padrao);

/*
 * FH_imprime_tudo
 *   Imprime toda a tabela (depuração).
 */
void FH_imprime_tudo(const char *tabHash, const char *dados, int m);

#endif /* FILMEHASH_H */
