#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>


//#define INPUT_FILE "cities/gr17_d.txt"
#define INPUT_FILE   "cities/fri26_d.txt"
#define N 26
#define ANT_NUM 10
#define EVAP_RATE 0.9
#define Q 1000
#define TOUR_MAX 1000
#define NUM_THREAD 2

int** map;
float phero[N][N] = {0};


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
		first_city = 0;//rand()%N;
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

int main()
{
	
	read_input(INPUT_FILE);
	srand(time(NULL));


	#pragma omp parallel num_threads(NUM_THREAD)
	{
	int tour_count = 0;
	double pro = 0; //probability
	double pro_accum = 0;
	double sum;
	int next;
	int record[N][N];

	int best_tour[N];
	int best_tour_distance=0;

	ant_t ants[ANT_NUM];

	while(tour_count < TOUR_MAX)
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
						next++;
			
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
			if(best_tour_distance == local_distance)
				printf("Same Path\n");
			if(best_tour_distance > local_distance || best_tour_distance == 0)
			{
				printf("Replace\n");
				memcpy(best_tour,ants[i].tour,N*sizeof(int));
				best_tour_distance = local_distance;	
			}

		}
		/*for(int i=0; i<ANT_NUM;i++)
		{
			print_array(ants[i].tour);
			//print_array(ants[i].visited);
		}*/

		#pragma omp critical
		{
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
		}

		#pragma omp barrier	
		
		//for(int i=0; i<N ;i++)
		//	print_array(record[i]);
		//print_phero(phero);

		
		tour_count++ ;
		//printf("%d \n",tour_count);
	}
	
        for(int i=0; i<ANT_NUM;i++)
        {
            print_array(ants[i].tour);
            //print_array(ants[i].visited);
        }

		printf("Best Tour:\nLen = %d\n",best_tour_distance);
		print_array(best_tour);

	printf("end while\n");


	}
	if(map)
		free(map);

	return 0;
}
