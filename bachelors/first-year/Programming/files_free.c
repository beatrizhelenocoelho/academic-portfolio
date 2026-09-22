#include "projeto.h"


/* ---------------------------------------------------------------------------------------------------------------
             A função libertar resume-se à desalocação de memória ocupada pelos malloc e realloc,
 começamos por desalocar as linhas (em alocação dinâmica) que estão dentro das estruturas e no fim as próprias estruturas

*/
void libertar(cursos*cursop, candidatos*candidatop, int ncursos, int alunos, nao_colocados*nao_clcds)
{

    // free(candidatop);
    // printf(" numero de cursos: %d\n", ncursos);

    free (nao_clcds);
    while (ncursos != 0)//percorrer os cursos
    {

        ncursos--;

        free(cursop[ncursos].ccurso);
        free(cursop[ncursos].ninst);
        free(cursop[ncursos].ncurso);
        free(cursop[ncursos].grau);
        free(cursop[ncursos].id_colocados);

        //for(int k=0; k<cursop[ncursos].colocados; k++)//percorrer alunos colocados
        //{
        //  free(cursop[k].id_colocados);
        //}
    }
    free(cursop);


    while (alunos!=0)
    {
        alunos--;
        for (int i=0; i< (candidatop[alunos].opPrior + candidatop[alunos].nOpcoes); i++)
        {
            free(candidatop[alunos].lista_cursos[i]);
        }
    }
    free(candidatop);

}
