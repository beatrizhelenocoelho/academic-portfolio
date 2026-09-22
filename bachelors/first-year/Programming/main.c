#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "projeto.h"


int main(int argc, char *argv[])
{

    int opt, v=0, ncandidatos=0, linhas, linhas_cand;
    char copy_cur_file[100];//copy_cur_file != 0 -> muda o ficheiro inicial dos cursos de nome
    char copy_cand_file[100];
    strcpy( copy_cur_file, "a");
    strcpy( copy_cand_file, "a");
    char col[100], info_cursos[100], info_completa[100], info_nc[100];
    strcpy( col, "a");
    strcpy( info_cursos, "a");
    strcpy( info_completa, "a");
    strcpy( info_nc, "a");

    int alunos=0, ncursos=0, alunos_colocar=0, id_temp, n_colocados, prior, alunos_nao_colocados=0, id;
    cursos *curso;

    candidatos *candidato;
    // nao_colocados nao_clcds [500]={};
    nao_colocados *nao_clcds;
    nao_clcds= malloc(sizeof(*nao_clcds)*alunos_nao_colocados);


    while ((opt = getopt(argc, argv, "hvnicoumx")) != -1) {
        switch (opt) {
            case 'h':
                printf("-v valor número de vagas a considerar em cada curso.\n");
                printf("-n valor número de candidatos a considerar da lista de candidatos\n");
                printf("-i filename nome do ficheiro entrada com lista de cursos\n");
                printf("-c filename nome do ficheiro entrada com lista de candidatos\n");
                printf("-o filename nome do ficheiro de saída com lista de colocações\n");
                printf("-u filename nome do ficheiro de saída com lista de universidades e colocados\n");
                printf("-m filename nome do ficheiro de saída com lista de cursos\n");
                printf("-x filename nome do ficheiro de saída com lista de não colocados\n");
                return 0;
            case 'v':

                if ((strcmp(argv[optind],"-n"))==0 ) // or o resto, saber se é isto
                {
                    printf("Erro!");
                    EXIT_FAILURE;
                }
                v = atoi(argv[optind]); // v é o número de vagas pretendido
                if (v<0)
                {
                    printf("número de vagas é negativo, impossivel.");
                }

                break;
            case 'n':

                ncandidatos = atoi(argv[optind]); // ncandidatos é o número de candidatos pretendido
                if (ncandidatos<0)
                {
                    printf("número de vagas é negativo, impossivel.");
                }

                break;
            case 'i':

                strcpy( copy_cur_file, argv[optind]);

                break;
            case 'c':

                strcpy( copy_cand_file, argv[optind]);

                break;
            case 'o': //colocações

                strcpy( col, argv[optind]);

                break;
            case 'u':

                strcpy( info_completa, argv[optind]);

                break;
            case 'm':

                strcpy( info_cursos, argv[optind]);

                break;
            case 'x':

                strcpy( info_nc, argv[optind]);

                break;

        }
    }

    linhas = devolve_linhas_curso(copy_cur_file);
    curso = malloc (sizeof (*curso)*(linhas));

    if (curso == NULL)
    {
        printf("Falha a alocar memória\n");
    }


    linhas_cand = devolve_linhas_candidatos(copy_cand_file);
    candidato = malloc(sizeof(*candidato)*(linhas_cand));


    ncursos=lercursos(curso, v, copy_cur_file);
    alunos=lercandidatos(candidato, ncandidatos, copy_cand_file);
    //nao_clcds=malloc(sizeof(NULL));


    printf("Alunos:%d\n", alunos);
    printf("Cursos:%d\n", ncursos);
    alunos_colocar=alunos;
    float minima=0, nota=0;
    int vagas_disponiveis, pos_curso;
    int alunos_por_colocar[alunos];

    //printf("%d\n", ncursos);

    for(int i=0; i<alunos; i++){
        alunos_por_colocar[i]=candidato[i].idcandidato-1;//guardar os ids de todos os alunos que estao por colocar
        //printf("%d\n", alunos_por_colocar[i]+1);
    }



    while(alunos_colocar>0)
    {
        for(int i=0; i<alunos_colocar; i++){

            id = candidato[alunos_por_colocar[i]].idcandidato;//ID do aluno por colocar

            if(candidato[alunos_por_colocar[i]].nOpcoes >0)
           {
            prior = candidato[alunos_por_colocar[i]].opPrior; //Ve qual a opcao prioritaria
            pos_curso=devolve_pos_curso(candidato[alunos_por_colocar[i]].inst[prior], candidato[alunos_por_colocar[i]].lista_cursos[prior], curso, ncursos); //Obter a posicao do curso prioritario no vector de structs do tipo curso

            minima=curso[pos_curso].min; //Ve qual a nota minima do curso prioritario
            nota=candidato[alunos_por_colocar[i]].candidatura;
            vagas_disponiveis = curso[pos_curso].vagas-curso[pos_curso].colocados;//vagas disponiveis = a vagas do curso menos as vagas ocupadas pelos colocados

            }



            //Remover o aluno da lista de alunos por colocar visto que nao tem mais opcoes as quais se pode candidatar, vamos coloca-lo nos nao colocados
              if(candidato[alunos_por_colocar[i]].nOpcoes == 0)
              {
                for (int k = i - 1; k < alunos_colocar - 1; k++)
                {
                    alunos_por_colocar[k] = alunos_por_colocar[k+1];
                }
                alunos_colocar--;

                nao_clcds= realloc(nao_clcds, sizeof(*nao_clcds)*(alunos_nao_colocados+1));

                //Por numa estrutura
                nao_clcds[alunos_nao_colocados].idcandidato = id;
                nao_clcds[alunos_nao_colocados].candidatura = nota;
                alunos_nao_colocados++;
                break;


            }


            else{
                if(vagas_disponiveis > 0)//Verificar se ainda existem vagas disponiveis ou se e necessarior ir comparar as notas minimas
                {
                    curso[pos_curso].id_colocados[curso[pos_curso].colocados] = candidato[alunos_por_colocar[i]].idcandidato; //Adicionar o aluno a lista de colocados no curso
                    curso[pos_curso].colocados++; //Incrementar o numero de colocados no curso
                    sort_candidatos_curso(curso, pos_curso, candidato);//Ordenar a lista de candidatos de acordo com as notas de candidatura
                    atualizar_nota_minima(curso, pos_curso, candidato);//Atualizar a nota minima do curso
                    for (int k = i - 1; k < alunos_colocar - 1; k++) //Visto que o aluno foi colocado deve se colocar na lista de alunos por colocar
                    {
                        alunos_por_colocar[k] = alunos_por_colocar[k+1];
                    }
                    alunos_colocar--;
                    break;//caso tenhamos removido algum aluno da lista dos por colocar devemos recomeçar o ciclo for
                }
                    //já nao existem vagas
                else{
                    if(nota>curso[pos_curso].min)//nota do candidato é maior do que a nota minima atual, assim aluno entra no curso
                    {
                        curso[pos_curso].id_colocados= realloc(curso[pos_curso].id_colocados, sizeof(int)*(curso[pos_curso].colocados +1));
                        curso[pos_curso].id_colocados[curso[pos_curso].colocados] = candidato[alunos_por_colocar[i]].idcandidato; //Adicionar o aluno a lista de colocados no curso
                        curso[pos_curso].colocados++;
                        sort_candidatos_curso(curso, pos_curso, candidato);//Ordenar a lista de candidatos de acordo com as notas de candidatura
                        atualizar_nota_minima(curso, pos_curso, candidato);//Atualizar a nota minima do curso
                        minima=curso[pos_curso].min;

                        for (int k = i - 1; k < alunos_colocar - 1; k++) //Visto que o aluno foi colocado deve ser retirado da lista de alunos por colocar
                        {
                            alunos_por_colocar[k] = alunos_por_colocar[k+1];
                        }
                        alunos_colocar--;


                        for(int n = 0; n < curso[pos_curso].colocados; n++){//percorrer todos os colocado
                            id = curso[pos_curso].id_colocados[n];
                            nota=candidato[id-1].candidatura;
                            if(nota == minima){
                                id_temp = n; //criar um id temporario para saber os colocados que têm nota igual à minima
                                break;
                            }
                        }

                        n_colocados = curso[pos_curso].colocados; //numero de colocados
                        if(id_temp >= curso[pos_curso].vagas){ //os alunos com nota minima, no caso em que estao a exceder as vagas
                            //printf("Remover as minimas\n");
                            int pos_inicial = id_temp;
                            for(int n = id_temp; n < n_colocados; n++){ //percorrer os candidatos que estao a exceder o numero de vagas

                                //Posicao que esta a remover:pos_inicial
                                //Vez que esta a fazer:n-1
                                //printf("ID que esta a ser removido:curso[pos_curso].id_colocados[pos_inicial]

                                curso[pos_curso].id_colocados= realloc(curso[pos_curso].id_colocados, sizeof(int)*(curso[pos_curso].colocados +1));
                                alunos_por_colocar[alunos_colocar] = curso[pos_curso].id_colocados[pos_inicial]-1;
                                alunos_colocar++;//volta para a lista de alunos por colocar


                                for (int k = pos_inicial ; k < curso[pos_curso].colocados; k++)//Visto que o aluno nao foi colocado deve se
                                {

                                    curso[pos_curso].id_colocados[k] = curso[pos_curso].id_colocados[k+1];
                                }
                                curso[pos_curso].colocados--;
                                //curso[pos_curso].id_colocados = realloc(curso[pos_curso].id_colocados, ((sizeof(curso[pos_curso].id_colocados)/sizeof(int))-1));


                            }
                        }

                        atualizar_nota_minima(curso, pos_curso, candidato);//Atualizar a nota minima do curso
                        break;
                    }
                    else{//a nota nao  é maior
                        if(nota == curso[pos_curso].min)//se a nota for igual á nota minima, temos que adicionar uma vaga
                        {
                            curso[pos_curso].id_colocados= realloc(curso[pos_curso].id_colocados, sizeof(int)*(curso[pos_curso].colocados +1));
                            curso[pos_curso].id_colocados[curso[pos_curso].colocados] = candidato[alunos_por_colocar[i]].idcandidato; //Adicionar o aluno a lista de colocados no curso
                            curso[pos_curso].colocados++; //Incrementar o numero de colocados no curso
                            sort_candidatos_curso(curso, pos_curso, candidato);//Ordenar a lista de candidatos de acordo com as notas de candidatura

                            for (int k = i - 1; k < alunos_colocar - 1; k++) //Visto que o aluno foi colocado deve se colocar na lista de alunos por colocar
                            {
                                alunos_por_colocar[k] = alunos_por_colocar[k+1];
                            }
                            alunos_colocar--;


                            break;//caso tenhamos removido algum aluno da lista dos por colocar devemos recomeçar o ciclo for
                        }
                        else{ //a nota é menor, logo não entra
                            if(candidato[alunos_por_colocar[i]].nOpcoes>0)//adicionar uma nova opcao prioritaria
                            {
                                candidato[alunos_por_colocar[i]].opPrior++;//Atribuir uma nova opcao prioritaria ao aluno que nao tinha notas suficientes para entrar na ultima opcao prioritaria
                                candidato[alunos_por_colocar[i]].nOpcoes--;//Diminuir o numero de opcoes restantes ao aluno que nao tinha nota suficiente para entrar na opcao prioritaria
                                //Ainda tem opcoes restantes
                                break;
                            }
                        }
                    }

                }

                printf("\n");
            }
        }
    }

   //Para fazer a ordenação como indicado no enunciado e diminuir o tempo do programa, só ordenamos no final
    for (int a=0;a<ncursos;a++)
    {
    sort_muda_candidatos_curso(curso, a, candidato);
    }


    ficheiros_saida(curso,candidato, alunos_nao_colocados,ncursos,alunos,nao_clcds,col, info_cursos, info_completa, info_nc, copy_cur_file, copy_cand_file);
    libertar(curso, candidato, ncursos, alunos, nao_clcds);

}
