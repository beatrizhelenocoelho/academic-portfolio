#include "projeto.h"


/*--------------------------------------------------------------------------------------------------------------------------
   Para cada fichero de saída, como temos que ter a opção do terminal, decidimos criar 4 variaveis e inicializa-las a "a",
              só mudamos o seu valor, caso o utilizador queira mudar o nome de algum ficheiro de saída
*/
void ficheiros_saida(cursos*curso, candidatos*candidato,int alunos_nao_colocados,  int ncursos, int alunos, nao_colocados*nao_clcds, char col[100], char info_cursos[100], char info_completa[100], char info_nc[100], char copy_cur_file[100],  char copy_cand_file[100])
{
    FILE *fp_col;
    FILE *fp_cursos;
    FILE *fp_com;
    FILE *fp_nc;
    // ficehiros de saida

//ficheiro das colocacoes dos alunos

    if ((strcmp(col,"a"))==0) //verificar se o utilizado pretende mudar o nome do ficheiro de saida
    {
        strcpy(col, "CNAES_Colocacoes.csv");//se o valor da variavel permanecer a "a", então, não foi invocada essa opco no terminal
    }
    fp_col = fopen(col, "w");

    fprintf(fp_col, "Candidato\tNota\tOpção\tInstituição\tCurso\n");//printf da primeira linha (cabeçalho)

     //imprimir no ficheiro de saída as informações guardadas no struct, neste caso, as colocações
    for (int i=0; i<ncursos; i++)//passa por todos os cursos
    {
        for(int k=0; k<curso[i].colocados; k++)//passar por todos os colocados
        {
            fprintf(fp_col, "%d\t%f\t%d\t%s\t%s\n", curso[i].id_colocados[k], candidato[curso[i].id_colocados[k]-1].candidatura, candidato[curso[i].id_colocados[k]-1].opPrior+1,curso[i].ninst,curso[i].ncurso );
        }

    }

    //fichero com as informacoes dos cursos

    if ((strcmp(info_cursos,"a"))==0) //verificar se o utilizado pretende mudar o nome do ficheiro de saida
    {
        strcpy(info_cursos, "CNAES_Cursos.csv");
    }
    fp_cursos= fopen(info_cursos, "w");

    fprintf(fp_cursos, "Instituição\tCurso\tVagas\tColocações\tNota mínima\n");

    for (int i=0; i<ncursos; i++)
    {
        fprintf(fp_cursos, "%s\t%s\t%d\t%d\t%f\n", curso[i].ninst, curso[i].ncurso, curso[i].vagas, curso[i].colocados,curso[i].min );
    }

    //ficheiro com informacoes completas

    if ((strcmp(info_completa,"a"))==0) //verificar se o utilizado pretende mudar o nome do ficheiro de saida
    {
        strcpy(info_completa, "CNAES_Completo.csv");
    }
    fp_com= fopen(info_completa,"w");

    //fprintf(fp_com, "Instituição\tCurso\tVagas/Opção\tColocações\tNota mínima\n");
    for (int i=0; i<ncursos; i++)
    {
        fprintf(fp_com, "%s\t%s\t%d\t%d\t%f\n", curso[i].ninst, curso[i].ncurso, curso[i].vagas,curso[i].colocados,curso[i].min );
        for(int k=0; k<curso[i].colocados; k++)
        {
            fprintf(fp_com, "%d\t%f\t%d\t%s\t%s\n", curso[i].id_colocados[k], candidato[curso[i].id_colocados[k]-1].candidatura, candidato[curso[i].id_colocados[k]-1].opPrior+1,curso[i].ninst,curso[i].ncurso );
        }
    }


    //ficheiro com informacão dos não colocados

    if ((strcmp(info_nc,"a"))==0) //verificar se o utilizado pretende mudar o nome do ficheiro de saida
    {
        strcpy(info_nc, "CNAES_NC.csv");
    }
    fp_nc= fopen(info_nc, "w");

    fprintf(fp_nc, "Candidato\tNota\n");
    for (int i=0; i<alunos_nao_colocados; i++){
        fprintf(fp_nc, "%d\t%f\n", nao_clcds[i].idcandidato, nao_clcds[i].candidatura);
    }

    fclose(fp_col);
    fclose(fp_cursos);
    fclose(fp_com);
    fclose(fp_nc);
}
