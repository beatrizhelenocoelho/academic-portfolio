#include "projeto.h"

int devolve_pos_curso(int cinst, char ccurso[10], cursos *curso,  int ncursos)
{
    int pos=0, len, count;
    for (int i=0; i<ncursos;i++) // percorrer o numero de cursos
        //(strcmp (curso[i].ccurso, ccurso))
    {
        if(curso[i].cinstituicao == cinst) //se o codigo da instituicao que o aluno escolher for igual ao codigo da instituicao
        {
            count=0;
            len = strlen(curso[i].ccurso);//tamanho de cararcteres do codigo do curso
            for(int j=0; j<len;j++){ //percorrer os caracteres do codigo do curso
                if (curso[i].ccurso[j]==ccurso[j])//para a mesma instituicao, vamos comparar o codigo do curso do aluno e o codigo do curso, se dodos os caracteres forem iguais, entao é o mesmo curso
                    count++; //aumento do count
            }
            if (count==len){//já percorremos todos os caracteres do codigo do curso
                pos=i;//podemos dizer que a posicao do curso é igual àquele curso
                break;
            }
        }
    }
    return pos;
}


/* --------------------------------------------------------------------------------------------------------------------------
  Colocar os colocados por nota de forma decrecente e caso tenham a mesma nota, devarao ser colocados com ids de forma decrecente
                                      Utilização do Bubble Sort (comparar dois a dois)
                        Compara as notas (candidatura) dos colocados dependendo do curso (pos_curso)
                        Compara os ids (idcandidadato) dos colocados dependendo do curso (pos_curso)
*/

void sort_candidatos_curso(cursos *curso, int pos_curso, candidatos *candidato){
    int temp, m, j;
    for(m=0;m<curso[pos_curso].colocados; m++){ //primeira posicao em comparacao com a seguinte
        for(j=m+1; j<curso[pos_curso].colocados;j++){//segunda posicao em comparacao com a anterior
            if (candidato[curso[pos_curso].id_colocados[m]-1].candidatura < candidato[curso[pos_curso].id_colocados[j]-1].candidatura){ //para ordenar de forma decrescente
                temp = curso[pos_curso].id_colocados[m];//se o primeiro for menor do que o segundo , vamos ter que trocar, assim defenimos temp como a primeira posicao, para depois poder trocar
                curso[pos_curso].id_colocados[m] = curso[pos_curso].id_colocados[j];//igualar a primeira com segunda, para poder defenier temp como segunda
                curso[pos_curso].id_colocados[j] = temp;//temos posicoes tocadas, ou seja de forma decresente (útil quando o primeiro é menor que segundo)
            }//vamos fazendo isto para todas as notas
            else{
                if (candidato[curso[pos_curso].id_colocados[m]-1].candidatura == candidato[curso[pos_curso].id_colocados[j]-1].candidatura){
                    if (candidato[curso[pos_curso].id_colocados[m]-1].idcandidato < candidato[curso[pos_curso].id_colocados[j]-1].idcandidato){
                        temp = curso[pos_curso].id_colocados[m];//se o primeiro for menor do que o segundo , vamos ter que trocar, assim defenimos temp como a primeira posicao, para depois poder trocar
                        curso[pos_curso].id_colocados[m] = curso[pos_curso].id_colocados[j];//igualar a primeira com segunda, para poder defenier temp como segunda
                        curso[pos_curso].id_colocados[j] = temp;//temos posicoes tocadas, ou seja de forma decresente (útil quando o primeiro é menor que segundo)
                    }
                }
            }
        }
    }
}



/* --------------------------------------------------------------------------------------------------------------------------
 Depois de todos os alunos  vamos coloca-los da forma como pedida no enunciado, assim, as notas estão por ordem crecente e caso sejam iguais serão ordenas pelo id decrescente
            Colocar os colocados por nota de forma decrecente e caso tenham a mesma nota, devarao ser colocados com ids de forma decrecente
                                             Utilização do Bubble Sort (comparar dois a dois)
                             Compara as notas (candidatura) dos colocados dependendo do curso (pos_curso)
                             Compara os ids (idcandidadato) dos colocados dependendo do curso (pos_curso)
*/
void sort_muda_candidatos_curso(cursos *curso, int pos_curso, candidatos *candidato){
    int temp, m, j;//comparar os valores das notas de candidatura, atraves de bubble sort(comparar dois a dois)
    for(m=0;m<curso[pos_curso].colocados; m++){ //primeira posicao em comparacao com a seguinte
        for(j=m+1; j<curso[pos_curso].colocados;j++){//segunda posicao em comparacao com a anterior
            if (candidato[curso[pos_curso].id_colocados[m]-1].candidatura > candidato[curso[pos_curso].id_colocados[j]-1].candidatura){ //para ordenar de forma crescente
                temp = curso[pos_curso].id_colocados[m];//se o primeiro for menor do que o segundo , vamos ter que trocar, assim defenimos temp como a primeira posicao, para depois poder trocar
                curso[pos_curso].id_colocados[m] = curso[pos_curso].id_colocados[j];//igualar a primeira com segunda, para poder defenier temp como segunda
                curso[pos_curso].id_colocados[j] = temp;//temos posicoes tocadas, ou seja de forma decresente (útil quando o primeiro é menor que segundo)
            }//vamos fazendo isto para todas as notas
            else{
                if (candidato[curso[pos_curso].id_colocados[m]-1].candidatura == candidato[curso[pos_curso].id_colocados[j]-1].candidatura){
                    if (candidato[curso[pos_curso].id_colocados[m]-1].idcandidato < candidato[curso[pos_curso].id_colocados[j]-1].idcandidato){
                        temp = curso[pos_curso].id_colocados[m];//se o primeiro for menor do que o segundo , vamos ter que trocar, assim defenimos temp como a primeira posicao, para depois poder trocar
                        curso[pos_curso].id_colocados[m] = curso[pos_curso].id_colocados[j];//igualar a primeira com segunda, para poder defenier temp como segunda
                        curso[pos_curso].id_colocados[j] = temp;//temos posicoes tocadas, ou seja de forma decresente (útil quando o primeiro é menor que segundo)
                    }
                }
            }
        }
    }
}


/*   ---------------------------------------------------------------------------------------------
    Função para ir atualizando a nota minima, cada vez que o numero de colocacados no curso muda
  id_ultimo_colocado será o ultimo dos elementos dos id_colocados para um determinado curso (pos_curso)
            A nota minima irá corresponder sempre à nota do ultimo colocado  */

void atualizar_nota_minima(cursos *curso, int pos_curso, candidatos *candidato){
    int n_colocados, id_ult_colocado;
    float nota_ult_colocado;
    n_colocados = curso[pos_curso].colocados; //Obter o numero de colocados
    id_ult_colocado = curso[pos_curso].id_colocados[n_colocados-1]; //Obter o id do ultimo colocado
    nota_ult_colocado = candidato[id_ult_colocado-1].candidatura; //Obter a nota do ultimo colocado;
    curso[pos_curso].min=nota_ult_colocado; //Atualizar a nota do ultimo colocado
}
