#include "projeto.h"


/*
   Como o número de opções pode variar de aluno para aluno, decidimos ler as opções como um todo e guarda-las separadamente na função guarda_opcoes.
*/
void guarda_opcoes(candidatos *candidato, int inicio,char *opcoes, int inicio2, int fim, int count){
    int i=0, j=0;
    char opcoes_aluno[50]={};
    char curso[10]={};
    int codigo;

    for(i=inicio2;i<=fim;i++,j++){
        opcoes_aluno[j]=opcoes[i];
    }//puts(opcoes_aluno);


    sscanf(opcoes_aluno, "%d,%s", &codigo, curso); //vai contar as virgulas das opções

    if(count==2){
        candidato[inicio].inst[0]=codigo;//COPIA O Codigo da inst

        candidato[inicio].lista_cursos[0] = malloc(sizeof (char)*(strlen(curso)+1));
        strcpy(candidato[inicio].lista_cursos[0],curso);

        candidato[inicio].opPrior=0;

    }

    else if(count==4){
        candidato[inicio].inst[1]=codigo;

        candidato[inicio].lista_cursos[1] = malloc(sizeof (char)*(strlen(curso)+1));
        strcpy(candidato[inicio].lista_cursos[1],curso);
        // printf("com uma opcacao %s", candidato[inicio].inst[0]);

        candidato[inicio].nOpcoes=2;
    }

    else if(count==6){
        candidato[inicio].inst[2]=codigo;

        candidato[inicio].lista_cursos[2] = malloc(sizeof (char)*(strlen(curso)+1));
        strcpy(candidato[inicio].lista_cursos[2],curso);

        candidato[inicio].nOpcoes=3;
    }

    else if(count==8){
        candidato[inicio].inst[3]=codigo;

        candidato[inicio].lista_cursos[3] = malloc(sizeof (char)*(strlen(curso)+1));
        strcpy(candidato[inicio].lista_cursos[3],curso);

        candidato[inicio].nOpcoes=4;
    }

    else{
        candidato[inicio].inst[4]=codigo;

        candidato[inicio].lista_cursos[4] = malloc(sizeof (char)*(strlen(curso)+1));
        strcpy(candidato[inicio].lista_cursos[4],curso);

        candidato[inicio].nOpcoes=5;
    }
}

//função para ler número de cursos

/* --------------------------------------------------------------------------------------------------------------
               A função devolve_linhas_curso , consiste numa leitura inicial do ficheiro,
  Para contar o número de linhas, ou seja, o número de cursos/candidatos (nº de cursos(ncursos))= nº de linhas-1).
     Utilizamos esta função para poder alocar as estruturas dos cursos e dos candidatos em alocação dinamica.*/
int devolve_linhas_curso(char copy_cur_file[100])
{
    FILE *fp;
    int linhas=0;
    char c;

    if ((strcmp(copy_cur_file,"a"))==0)
    {
        strcpy(copy_cur_file, "cursos.csv");
    }

    fp=fopen(copy_cur_file, "r");

    if(fp==NULL)
    {
        printf("Cannot open the file\n");
    }

    else
    {

        do //ciclo para contar o número de linhas do ficheiro dos cursos, sendo a 1º titulo nºde cursos = linhas-1, para depois alocar o número certo de cursos
        {
            c=fgetc(fp);
            //printf("%c\n", c);
            if (c == '\n')
            {
                linhas++;
            }
        }
        while (c != EOF);//enquanto as linhas para depois alocar exatamente o número certo de cursos (linhas-1)

        rewind(fp);}
    return linhas-1;
}

/*  ---------------------------------------------------------------------------------------------------
   A função lercursos, consiste em ler e guardar as informações dos ficheiros de entrada nas estruturas.
*/

int lercursos(cursos *curso, int v, char copy_cur_file[100] )//dois * para chamar por referencia
{
    FILE *fp;
    int inicio=0, h=0, linhas=0;
    char c, buffer_ccurso[10], buffer_ninst[300], buffer_ncurso[300], buffer_grau[100];
    //buffer para conseguir fazer alocacao dinamica dos char da estrututura cursos

    //inicializei copy_cur_file a "a" no main logo se continuar igual a "a"é porque não estamos a utilizar o getopt
    //fazemos isto para conseguir abrir um ficheiro com um nome diferente no terminal
    if ((strcmp(copy_cur_file,"a"))==0)
    {
        strcpy(copy_cur_file, "cursos.csv");
    }

    fp=fopen(copy_cur_file, "r");

    if(fp==NULL)
    {
        printf("Cannot open the file\n");
    }

    else
    {

        do //ciclo para contar o número de linhas do ficheiro dos cursos, sendo a 1º titulo nºde cursos = linhas-1, para depois alocar o número certo de cursos
        {
            c=fgetc(fp);
            //printf("%c\n", c);
            if (c == '\n')
            {
                linhas++;
            }
        }
        while (c != EOF);//enquanto as linhas para depois alocar exatamente o número certo de cursos (linhas-1)

        rewind(fp); //voltar a apontar para a primeira linha



        printf("File cursos sucessfully opened\n");

        char ch;
        do {
            ch = fgetc(fp);
        } while (ch != '\n');


        do
        {
            fscanf(fp,"%d,%[^,],%[^,],%[^,],%[^,],%d", &curso[inicio].cinstituicao,buffer_ccurso,buffer_ninst,buffer_ncurso,buffer_grau,&curso[inicio].vagas);

           curso[inicio].ccurso = ((char*) malloc((sizeof(char))*(strlen(buffer_ccurso)+1)));
           strcpy(curso[inicio].ccurso, buffer_ccurso);
           /*if (curso[inicio].ccurso == NULL)
            { printf("Falha a alocar memória\n");*/


            curso[inicio].ninst = (char*) malloc(sizeof(char)*(strlen(buffer_ninst)+1));
            strcpy(curso[inicio].ninst, buffer_ninst);

            curso[inicio].ncurso = (char*) malloc(sizeof(char)*(strlen(buffer_ncurso)+1));
            strcpy(curso[inicio].ncurso, buffer_ncurso);

            curso[inicio].grau = (char*) malloc(sizeof(char)*(strlen(buffer_grau)+1));
            strcpy(curso[inicio].grau, buffer_grau);


            curso[inicio].id_colocados = (int*) malloc(sizeof(int)*(curso[inicio].vagas)); // alocar o id_colocados de forma dinamica

            // com o -v
            if (v==0) // se não utilizamos o -v(getopt) fica tudo igual
            {
                h=1;// o ciclo é acionado, logo o v tem de no final ser igual a zero para o próximo.
                // se não colocar nada o v para o próximo será o número de vagas do antigo.
                v = curso[inicio].vagas;
            }
            curso[inicio].vagas=v;
            curso[inicio].min=10;
            curso[inicio].colocados=0;
            inicio++;
            if (h==1)
            {
                v=0;//aqui garantimos que v =0 para o próximo.
            }
        }
        while (inicio<linhas-1);

    }
    inicio++;

    //for (int i=0; i<inicio-1; i++)
    //{
    //printf("%d\n", curso[i].cinstituicao);
    //puts(curso[i].ccurso);
    //puts(curso[i].ninst);
    //puts(curso[i].ncurso);
    //puts(curso[i].grau);
    //printf("%d\n", curso[i].vagas);

    //}

    return inicio-1;
}

/* -----------------------------------------------------------------------
Funcao semelhante à develove_linhas_curso, mas desta vez paraos candidatatos
                    Diz o número de candidatos*/

int devolve_linhas_candidatos(char copy_cand_file[100])
{
    FILE *fp;

    int linhas = 0;
    char c;
    //_candidatos * candidato;

    if ((strcmp(copy_cand_file, "a")) == 0) {
        strcpy(copy_cand_file, "candidatos.csv");
    }

    fp = fopen(copy_cand_file, "r");

    if (fp == NULL) {
        printf("Cannot open the file\n");
    } else {
        do //
        {
            c = fgetc(fp);
            //printf("%c\n", c);
            if (c == '\n') {
                linhas++;
            }
        }
        while (c != EOF);
    }
    return linhas-1;
}

/*  ---------------------------------------------------------------------------------------------------
   A função lercandidatos, consiste em ler e guardar as informações dos ficheiros de entrada nas estruturas.
*/
int lercandidatos (candidatos *candidato, int ncandidatos, char copy_cand_file[100]) //ler
{
    FILE *fp;

    int inicio=0, final=0, linhas=0;
    char c, opcoes [1000];
    //_candidatos * candidato;

    if ((strcmp(copy_cand_file,"a"))==0)
    {
        strcpy(copy_cand_file, "candidatos.csv");
    }

    fp=fopen(copy_cand_file, "r");

    if(fp==NULL)
    {
        printf("Cannot open the file\n");
    }
    else
    {
        do //
        {
            c=fgetc(fp);
            //printf("%c\n", c);
            if (c == '\n')
            {
                linhas++;
            }
        }
        while (c != EOF);//contar as linhas para depois alocar exatamente o número certo de cursos (linhas-1)

        rewind(fp); //voltar a apontar para a primeira linha

        //*candidatop = malloc(sizeof(candidatos)*(linhas-1));
        //candidato = *candidatop;


        if (ncandidatos==0) // se não utilizamos o -v(getopt) fica tudo igual
        {
            ncandidatos = linhas-1; // não é, na verdade o número de candidatos é igual a linhas -1, mas fazemos asssim para em baixo ler só linhas.
        }

        printf("File candidatos sucessfully opened\n");

        char ch;
        do {
            ch = fgetc(fp);
        } while (ch != '\n');
        do
        {
            final = 0;
            fscanf(fp,"%d,%f,%f,%f,%[^\n]", &candidato[inicio].idcandidato,&candidato[inicio].pingresso,&candidato[inicio].sec,&candidato[inicio].candidatura, opcoes);
            // puts(opcoes);


            int i=0,count=0,inicio2=0, fim=0;
            while(opcoes[i] != '\0')//enquanto nao chegarmos ao fim da sting,
            {
                if (opcoes[i] == ',') //encontrar virgulas
                {
                    count++;
                    if (opcoes[i-1] != ','){
                        if (count%2==0)//virgula par
                        {
                            fim=i-1;//mandar para a funcao
                            guarda_opcoes(candidato,inicio,opcoes,inicio2,fim,count);
                            inicio2=fim+2;//para a proxima virgula
                        }
                    }
                    else
                        final = 1;
                }
                i++;

                //if(opcoes[i] == ',' && opcoes[i+1] == ','){ //se encontrar duas virgulas seguidas e sinal que nao existem mais opcoes e sai do loop
                //   final = 1;
                //   break;
                //}
            }
            if(count>8 && final == 0){
                fim=i-1;//mandar para a funcao
                guarda_opcoes(candidato,inicio,opcoes,inicio2,fim,count);
            }
            inicio++;
        }
        while(inicio<ncandidatos); //(ch=fgetc(fp))!=EOF
    } inicio++;
/*
    for (int i=0; i<inicio-1; i++)
    {
        printf("ID%d\n",candidato[i].idcandidato );
        printf("%f\n",candidato[i].pingresso);
        printf("%f\n",candidato[i].sec );
        printf("%f\n",candidato[i].candidatura);
        printf("%d\n",candidato[i].inst[0]);
        puts(candidato[i].lista_cursos[0]);
        printf("%d\n",candidato[i].inst[1]);
        puts(candidato[i].lista_cursos[1]);
        printf("%d\n",candidato[i].inst[2]);
        puts(candidato[i].lista_cursos[2]);
        printf("%d\n",candidato[i].inst[3]);
        puts(candidato[i].lista_cursos[3]);
        printf("%d\n",candidato[i].inst[4]);
        puts(candidato[i].lista_cursos[4]);
        printf("Opcoes restantes : %d\n", candidato[i].nOpcoes);

    }
    */
    return inicio-1;
}
