#ifndef PROJETO_H_INCLUDED
#define PROJETO_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

typedef struct  //estrutura de dados dos cursos
{
    int  cinstituicao; //identificados da instituicao (ex 2018)
    char *ccurso; //identificador do curso (ex L140)
    char *ninst ; //nome da instituicao
    char *ncurso; //nome do curso
    char *grau; //grau conferido pelo curso
    int vagas; //numero de vagas do curso
    int colocados; // colocamos tambem a informação que muda dos colocados
    int *id_colocados;
    float min;
} cursos;


typedef struct  //estrutura de dados dos candidatos
{
    int  idcandidato; // identificador do candidato
    float pingresso; // nota da prova de ingresso
    float sec; // média do secundário
    float candidatura; // nota de candidatura
    int inst [5]; // vetor com até 5 identificadores da intituição de ensino de cada opção
    char *lista_cursos [5];// vetor com até 5 identificadores de curso de cada opção
    int nOpcoes; //número de opções (não é lido mas depois o código dá), começa com o nº de opçoes e vai diminuido á medida que o aluno não entra, tendo -1 opção
    int opPrior; //opção prioritária(começa a zero e caso não entre aumenta)
    //int instprior;
    //char cursoprior[100];
} candidatos;

typedef struct  //estrutura de dados dos cand
{
    int  idcandidato; // identificador do candidato
    float candidatura; //nota de candidatura
}   nao_colocados;


void guarda_opcoes(candidatos *candidato, int inicio,char *opcoes, int inicio2, int fim, int count);
int devolve_linhas_curso(char copy_cur_file[100]);
int lercursos(cursos *curso, int v, char copy_cur_file[100] );
int devolve_linhas_candidatos(char copy_cand_file[100]);
int lercandidatos (candidatos *candidato, int ncandidatos, char copy_cand_file[100]);


int devolve_pos_curso(int cinst, char ccurso[10], cursos *curso,  int ncursos);
void sort_candidatos_curso(cursos *curso, int pos_curso, candidatos *candidato);
void sort_muda_candidatos_curso(cursos *curso, int pos_curso, candidatos *candidato);
void atualizar_nota_minima(cursos *curso, int pos_curso, candidatos *candidato);

void libertar(cursos*cursop, candidatos*candidatop, int ncursos, int alunos, nao_colocados*nao_clcds);


void ficheiros_saida(cursos*curso, candidatos*candidato,int alunos_nao_colocados,  int ncursos, int alunos, nao_colocados*nao_clcds, char col[100], char info_cursos[100], char info_completa[100], char info_nc[100], char copy_cur_file[100],  char copy_cand_file[100]);



 int devolve_pos_curso(int cinst, char ccurso[10], cursos *curso,  int ncursos);




#endif // PROJETO_H_INCLUDED
