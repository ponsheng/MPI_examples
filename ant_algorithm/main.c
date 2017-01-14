#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <mpi.h>

#ifndef INPUT_FILE
#define INPUT_FILE "cities/fri26_d.txt"
#define N 26
#endif

#define ANT_NUM 10
#define EVAP_RATE 0.9
#define Q 1000
#define TOUR_MAX 1000
#define NUM_THREAD 2

int** map;
float global_phero[N][N] = {0};
int global_tour[N] ;
int global_tour_distance = 0;

int best_tour_distance;
int* gather_array;
int best_node;
int same_path_count=0;
int loop_end = 0;

typedef struct ant{
	int tour[N];	
	int visited[N];
} ant_t;

int read_input(char*input_file);
double rand_d();


int read_input(char* input_file)
{
	FILE* input_fp;
	char *line = NULL;
	size_t len = 0;
	char *token;

	//open file
	input_fp = fopen(input_file,"r");
	if(input_fp == NULL)
	{
		printf("open file %s failed!\n",INPUT_FILE);
		exit(0);
	}
	//get map 
	map = (int**)malloc(sizeof(int*)*N);
	for(int i = 0;i<N;i++)
		map[i] = (int*)malloc(sizeof(int)*N);	

	for(int i=0;i<N;i++)
	{
		getline(&line, &len, input_fp);
		token = strtok(line," ");
		for(int j=0;j<N && token != NULL;j++)
		{
			map[i][j] = atoi(token);
			//printf("%d ",map[i][j]);
			token = strtok(NULL, " ");
		}
		//printf("\n");
	}
	return 0;
}

double rand_d() //random decimal fraction,ie 0~1
{
	return (double)rand() / (double)RAND_MAX;
}

void print_array(int* array)
{
	for(int i=0;i<N;i++)
		printf("%2d ",array[i]);
	printf("\n");	
}

void print_phero(float a[][N])
{
	for(int i=0; i<N; i++)
	{
		for(int j=0;j<N;j++)
			printf("%.2f",a[i][j]);
		printf("\n");
	}
}

void ants_init(ant_t* ants,int n)
{
	int first_city;
	memset(ants,0,ANT_NUM*sizeof(ant_t));	
	for(int i=0; i<ANT_NUM; i++)
	{
		first_city = rand()%N;
		ants[i].tour[0] = first_city;
		ants[i].visited[first_city] = 1;
		//printf("first city: %d\n",first_city);
	}
}

int cal_tour_distance(int* tour)
{
	int distance = 0;
	for(int i=0; i<N-1; i++)
		distance += map[tour[i]][tour[i+1]];
	
	distance += map[tour[N-1]][tour[0]];

	//printf("distance: %d\n",distance);

	return distance;
}

int main(int argc,char *argv[])
{
	int comm_size_tmp;
	int my_id;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size_tmp);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	const int comm_size = comm_size_tmp;

	read_input(INPUT_FILE);
	srand(time(NULL));
	if(my_id == 0)
		printf("Choosed file: %s\n\n",INPUT_FILE);	


	gather_array = (int*) malloc(sizeof(int) * comm_size);

	#pragma omp parallel num_threads(NUM_THREAD)
	{
	float phero[N][N] = {0};
	int tour_count = 0;
	double pro = 0; //probability
	double pro_accum = 0;
	double sum;
	int next;
	int record[N][N];

	int local_tour[N];
	int local_tour_distance=0;

	ant_t ants[ANT_NUM];

	while(tour_count < TOUR_MAX && loop_end == 0)
	{
		ants_init(ants,ANT_NUM);
		memset(record,0,N*N*sizeof(int));
		for(int i=0;i<N-1;i++)
		{
			for(int a=0;a< ANT_NUM;a++)
			{
				pro = rand_d();
				//printf("pro:%f\n",pro);
				ant_t* ant_p = &ants[a];
				sum = 0;
				pro_accum = 0;
				//Sum of phero
				for(int j=0;j<N;j++)
				{
					if(ant_p->visited[j]==0)
					{
						//printf("j:%d %f / %d\n",j,phero[ant_p->tour[i]][j],map[ant_p->tour[i]][j]);
						sum += 	 (phero[ant_p->tour[i]][j]+1)/(map[ant_p->tour[i]][j]);
					}
				}
				//printf("sum:%f\n",sum);
				next = -1;
				while(pro_accum < pro)
				{
					next++;
					while(ant_p->visited[next]!=0)
					{
						next++;
						if(next >= N)
							printf("next exceed%d %d\n",next,ant_p->visited[next]);
					}
			
					pro_accum += (phero[ant_p->tour[i]][next]+1)/map[ant_p->tour[i]][next]/sum;
				}
				if(next >= N)
				{
					printf("error: pro:%f accum:%f sum:%f next:%d\n",pro,pro_accum,sum,next);
					exit(0);
				}
				//printf("next: %d accum: %f\n",next,pro_accum);
				ant_p->visited[next] = 1;
				ant_p->tour[i+1] = next;

				//thread unsafety
				record[ant_p->tour[i]][next]++;
				//
			}
		}

		for(int i=0; i<ANT_NUM; i++)
		{
			int local_distance = cal_tour_distance(ants[i].tour);
			if(local_tour_distance == local_distance);
				//printf("Same Path\n");
			if(local_tour_distance > local_distance || local_tour_distance == 0)
			{
				//printf("Replace\n");
				memcpy(local_tour,ants[i].tour,N*sizeof(int));
				local_tour_distance = local_distance;	
			}

		}

		
		int phero_count;
		for(int i=0; i<N ;i++)
		{
			//print_array(record[i]);
			for(int j=i;j<N;j++)
			{
				phero_count = record[i][j] + record[j][i];
				//printf("count %d\n",phero_count);
				phero[i][j] = phero[j][i] = phero[j][i]*(1-EVAP_RATE) + Q*(float)phero_count/(map[i][j]+1);
				//printf("%d ",phero[j][i]*EVAP_RATE + Q*phero_count/(map[i][j]+1))
			}
		}
		
		#pragma omp critical
		{
			//if(local_tour_distance == global_tour_distance)
				//printf("Same Path\n");
			if(local_tour_distance < global_tour_distance || global_tour_distance == 0)
			{
				//printf("Replace\n");
				memcpy(global_tour,local_tour,N*sizeof(int));
				global_tour_distance = local_tour_distance;
			}

		}


		#pragma omp single
		{
		//mpi data exchange
		MPI_Allreduce(&global_tour_distance, &best_tour_distance, 1,MPI_INT, MPI_MIN, MPI_COMM_WORLD);
		//printf("best_distance: %d\n",best_tour_distance);

		if(my_id == 0)
		{
			if(global_tour_distance == best_tour_distance)
			{
				same_path_count++;
				if(same_path_count > 50)
				{
					loop_end = 1;
					printf("Reach terminate condition\n");
				}
			}	
			else
				same_path_count = 0;
		}

		MPI_Allgather(&global_tour_distance, 1, MPI_INT,gather_array,1,MPI_INT,MPI_COMM_WORLD);

		for(best_node=0; best_node<comm_size; best_node++)
		{
			if(gather_array[best_node] == best_tour_distance)
				break;
			
		}
		if(best_node == comm_size)
			printf("MPI_ERROR\nplease rerun program");
		//printf("Best node: %d\n",best_node);
		MPI_Bcast(&global_tour_distance,1,MPI_INT,best_node,MPI_COMM_WORLD);
		MPI_Bcast(global_tour,N,MPI_INT,best_node,MPI_COMM_WORLD);	
		MPI_Bcast(&loop_end,1,MPI_INT,0,MPI_COMM_WORLD);
		}
		#pragma omp barrier

		tour_count++ ;
	}

		/*#pragma omp critical	
		{
        for(int i=0; i<ANT_NUM;i++)
        {
            print_array(ants[i].tour);
            //print_array(ants[i].visited);
        }

		printf("Thread: %d Best Tour:\nLen = %d\n",omp_get_thread_num(),local_tour_distance);
		//print_array(local_tour);
		printf("\n");
		}*/
	}

	if(my_id == 0)
	{
		printf("Result best tour route: ");
		print_array(global_tour);
		printf("\nMin distance: %d\n",best_tour_distance);
	}
	//printf("\nNode %d Best Tour:\nLen = %d\n",my_id,global_tour_distance);
	//print_array(global_tour);

	MPI_Finalize();
	if(map)
		free(map);
	free(gather_array);
	return 0;
}
