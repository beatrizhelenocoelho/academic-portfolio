/******************************************************************************
 * Programacao Concorrente
 * MEEC 21/22
 *
 * Projecto - Parte1
 *                           serial-complexo.c
 * 
 * Compilacao: gcc serial-complexo -o serial-complex -lgd
 *           
 *****************************************************************************/
#include <pthread.h> 
#include <gd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>
#include "image-lib.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>

/* the directories wher output files will be placed */
#define OLD_IMAGE_DIR "/old_photo_PAR_A/"

//estrutura que guarda o inicio e o fim de cada thread
typedef struct Arg_Threads{
      int start;
      int end ;
      struct Arg_Threads* next;
} Arg_Threads;

Arg_Threads **arg_threads;

//estrutura com a informção das imagens
typedef struct Files_data{
    gdImagePtr in;
    gdImagePtr out_smoothed_img ;
	gdImagePtr out_contrast_img ;
	gdImagePtr out_textured_img ;
	gdImagePtr out_sepia_img ;
	gdImagePtr in_texture_img ;
}Files_data;

Files_data ** data;

int THRS=0; // variavél global com o número de threads
char file_ver[100];
int count_thread; 
char ** files;


void*thread_function(void*arg){

	int L = (*(Arg_Threads**)arg)->start;//inicio da thread
    int E = (*(Arg_Threads**)arg)->end;//fim da thread 

	struct timespec start_time_thread, end_time_thread;
	struct timespec *thread_time = (struct timespec*)malloc(sizeof(struct timespec));

	clock_gettime(CLOCK_MONOTONIC, &start_time_thread);
	if ( L !=-1){
		for (int i= L; i<= E;i++){ //percorrer a thread
			char file_ver2[100];
			//ver se ficheiros ja foram criados
			sprintf(file_ver2, "%s%s", file_ver, files[i]); // directory para as imagens com o filtro old  ./Dataset1/Old.../img...
			if( access( file_ver2, F_OK ) != -1){ // se o ficheiro de saida já foi criado, não temos que o voltar a fazer
				printf("%s encontrado\n", files[i]);
			}
			
			 else{
				// se não foi criado, aplicar os diversos filtros para ficarmos com o efeito old-photo
				data[i]->out_contrast_img = contrast_image(data[i]->in);
				gdImageDestroy(data[i]->in);
				data[i]->out_smoothed_img = smooth_image(data[i]->out_contrast_img);
				gdImageDestroy(data[i]->out_contrast_img);
				data[i]->out_textured_img = texture_image(data[i]->out_smoothed_img , data[0]->in_texture_img);
				gdImageDestroy(data[i]->out_smoothed_img);
				data[i]->out_sepia_img = sepia_image(data[i]->out_textured_img);
				gdImageDestroy(data[i]->out_textured_img);
				
				//escrever as imagens na dirctory indicada
				if(write_jpeg_file(data[i]->out_sepia_img, file_ver2) == 0){
				fprintf(stderr, "Impossible to write %s image\n",file_ver2);
				}
				gdImageDestroy(data[i]->out_sepia_img); //free da imagem final, uma vez que já foi escrita
		
			}
		}	
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time_thread);
	*thread_time = diff_timespec(&end_time_thread, &start_time_thread);
	return (void*)thread_time;
}

/******************************************************************************
 * main()
 *
 * Arguments: (none)
 * Returns: 0 in case of sucess, positive number in case of failure
 * Side-Effects: creates thumbnail, resized copy and watermarked copies
 *               of images
 *
 * Description: implementation of the complex serial version 
 *              This application only works for a fixed pre-defined set of files
 *
 *****************************************************************************/

int main(int argc, char*argv[]){
	
	FILE *fp, *fpsaida;
	int nn_files = 0;
	char timing[]="timing_";
	char linha[100];
	int t=0;//saber o numero de threads por imagem
    struct timespec start_time_total, end_time_total;
    struct timespec start_time_seq, end_time_seq;
    struct timespec start_time_par, end_time_par;
	char read_file[100];
	char apoio_read[100];
	
	clock_gettime(CLOCK_MONOTONIC, &start_time_total);
	clock_gettime(CLOCK_MONOTONIC, &start_time_seq);
	
	THRS = atoi(argv[2]);//numero de threads
	count_thread= THRS;
	long int v[THRS][2]; // vetor com os tempos das threads 
	
	char*nome_pasta = strdup(argv[1]);//nome_pasta guarda o ./Dataset1 e nao altera
	char nome_data[100];//para guardar o ./Dataset1

	

	//para guardar o nome do argv[1] com um "/" no final, e ser usado para ler o image-list
	strcpy(nome_data, nome_pasta);

	char n_texture[100];
	strcpy(n_texture, nome_data);

	strcat(nome_data, "/"); //./Dataset1/

	

	//guardar o argv[1]
	strcpy(apoio_read, nome_data);

	//diretoria onde vamos colocar as imagens com o filtro old
	strcpy(file_ver, nome_pasta);
	strcat(file_ver, OLD_IMAGE_DIR); // ./Dataset1./old_photo_PAR_A/


	//para criar ficheiro com o timing das threads 
    int buffer_size = snprintf(NULL, 0, "%s%s%d", nome_data, timing, THRS);
    char saida[buffer_size + 5]; // +1 para o caractere nulo terminador +4..txt
    snprintf(saida, sizeof(saida), "%s%s%d.txt", nome_data, timing, THRS);
	fpsaida = fopen(saida, "w");
	
	
	//nome_data original tinha a diretoria onde esta o image-list
	strcat(nome_data, "image-list.txt");
	fp = fopen (nome_data, "r");
	//contar o numero de imagens que vão ser processadas
    while (fgets(linha, 100, fp)!= NULL){
        nn_files++;
    }
	fclose(fp);
    
	files = calloc(sizeof(char*), nn_files);
	fp = fopen (nome_data, "r"); //voltar a abrir o image-list para guardar o nome das imagens a ser processadas
	for (int i = 0; i< nn_files; i++){
        files[i] = calloc(sizeof(char), 100);
        fgets(files[i], 100, fp);
        if (files[i][strlen(files[i])-1] =='\n'){
            files[i][strlen(files[i])-1] = '\0';
        }
       
    }

	//alocar as estruturas que dependem do numero de imagens, e numero de threads
	arg_threads = malloc(THRS * sizeof(Arg_Threads*));
	data = malloc(nn_files * sizeof(Files_data*));
	for (int c = 0; c <nn_files ; c++){
		data[c]=  malloc(sizeof(Files_data));
	}
	for (int d =0; d < THRS; d++){
		arg_threads[d]=  malloc(sizeof(Arg_Threads));
		arg_threads[d]->next = NULL;
	}

	//diretoria do output
	if (create_directory(file_ver) == 0){
		fprintf(stderr, "Impossible to create %s directory\n", file_ver);
		exit(-1);
	}

	strcat(n_texture, "/paper-texture.png");
	if( access( "./paper-texture.png", F_OK ) != -1){ // se o ficheiro de saida já foi criado, não temos que o voltar a fazer
		data[0]->in_texture_img = read_png_file("./paper-texture.png");
	}else{
		data[0]->in_texture_img =  read_png_file(n_texture);
	}

	
	clock_gettime(CLOCK_MONOTONIC, &end_time_seq);
	clock_gettime(CLOCK_MONOTONIC, &start_time_par);

	pthread_t thread_id[THRS];
	
	for (int i = 0; i <nn_files ; i++){	
		//diretoria do argv[1]
		strcpy(read_file, apoio_read);
		//para ir buscar a imagem de input na diretoria certa
		strcat(read_file,files[i]);
	    data[i]->in = read_jpeg_file(read_file); //ler a imagem de input
		if ( data[i]->in == NULL){
			fprintf(stderr, "Impossible to read %s image\n", read_file); 
			continue;
		}
		
	}
		int D, K;
        if ((nn_files%THRS==0)){ //o numero de threads permite divir equitativamente as imagens pelo numero de threads
            D = nn_files/THRS;
        }else{ //se nao divir equitativamente as imagens temos que acrescentar um, sendo que a divisao é feita por baixo
            D = (nn_files/THRS)+1;
        }
        for(int i=0; i<THRS; i++)
        {  
			if (i<nn_files){ //se a thread em que estamos for menor do que o numero de imagens nao queremos processar imagens então a thread não vai ter imagens para tratar
				arg_threads[i]->start=i*D;
				
				if ((arg_threads[i]->start + D - 1)>=nn_files-1){
					arg_threads[i]->end = nn_files-1;      
				}else{
				arg_threads[i]->end= arg_threads[i]->start + D - 1;
				}
			}else{
				arg_threads[i]->start=-1;
				arg_threads[i]->end=-1;
			}
			//para criar as threads(id da thread, NUll-thread é criada com os atributos de default, é chamada na thread_function, argumento de thread_function)  
        	pthread_create(&thread_id[i], NULL,  thread_function, (void*)&arg_threads[i]); 
        }

        for(int i=0; i<THRS; i++){
			struct timespec *thread_time;
        	pthread_join(thread_id[i], (void**)&thread_time);
			v[THRS - count_thread][0]=thread_time->tv_sec; //tempo de cada thread segundos
			v[THRS - count_thread][1]=thread_time->tv_nsec; //parte fracional do tempo de cada thread
			count_thread--;
			free(thread_time);
        }

		gdImageDestroy(data[0]->in_texture_img);
	
	clock_gettime(CLOCK_MONOTONIC, &end_time_par);
	clock_gettime(CLOCK_MONOTONIC, &end_time_total);


	struct timespec par_time = diff_timespec(&end_time_par, &start_time_par);
	struct timespec seq_time = diff_timespec(&end_time_seq, &start_time_seq);
	struct timespec total_time = diff_timespec(&end_time_total, &start_time_total);
    //para escrever os tempos, a parte sequecial, paralela, e total
	printf("\tseq \t %10jd.%09ld\n", seq_time.tv_sec, seq_time.tv_nsec);
    printf("\tpar \t %10jd.%09ld\n", par_time.tv_sec, par_time.tv_nsec);
    printf("total \t %10jd.%09ld\n", total_time.tv_sec, total_time.tv_nsec);
	
	//escrever no ficheiro de tempo os tempos de cada thread, bem como o numero de imgens processadas por cada thread
	fprintf(fpsaida, "total \t%d  %10jd.%09ld\n", THRS, total_time.tv_sec, total_time.tv_nsec);
	for (int h=0; h<THRS; h++ ){
		if(arg_threads[h]->start ==-1){
			t=0;
		}
		else
		 	t=arg_threads[h]->end -arg_threads[h]->start +1;
		
		fprintf(fpsaida, "thread_%d \t%d  %10jd.%09ld\n", h, t, v[h][0], v[h][1]);
	}
	fclose(fp);
	fclose(fpsaida);
	//libertar a memoria
	for (int c = 0; c <nn_files ; c++){
		free(data[c]);
		free(files[c]);
	}
	for (int d = 0; d < THRS; d++){
		free(arg_threads[d]);
	}
	free(files);
	free(arg_threads);
	free(data);
	free(nome_pasta);
	exit(0);
        
}

