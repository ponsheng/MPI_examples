#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define NUM 8
#define DIR_NAME "input"
#define KEYWORD_FILE "keyword.txt"

typedef struct entry 
{
	char* string;
	int len;
	struct entry* next;
} entry;

char** keywords = NULL;
int* keyword_table = NULL;

int keyword_count =0;

entry* list_head = NULL;
entry* list_end = NULL;

//entry* all_head = NULL;
//int first = 0;

void examine (char* examine_string);
int enqueue(char* string, int read);
int dequeue(char** string);
void clean_list();
void readkeyword();

int main(){
	DIR *dir_stream;
	int producer_count = 0;
	int stop = 0;

	dir_stream = opendir(DIR_NAME);

	if(errno)
		printf("opendir error: %d\n",errno);
		
	readkeyword(KEYWORD_FILE);

	#pragma omp parallel num_threads(NUM)
	{
	if(omp_get_thread_num() < NUM/2)
	{
		struct dirent* dirent;
		FILE * fp;
		char filename[30] = "\0" ;
		char * line = NULL;
		size_t len = 0;
		ssize_t read;
	
		while(1)
		{
			#pragma omp critical(readdir)
			{
				dirent = readdir(dir_stream);
			}
			if(dirent == NULL)
				break;
			if(dirent->d_type == DT_REG)
			{
				filename[0] = '\0';
				strcat(filename,DIR_NAME);
                strcat(filename,"/");
                strcat(filename,dirent->d_name);
				printf("proc %d read  |%s|\n",omp_get_thread_num(),filename);
				
				fp = fopen(filename, "r");
				if (fp == NULL)
				{
					printf("fopen fail\n");
					exit(EXIT_FAILURE);
				}

				//getline and send to queue	
				while ((read = getline(&line, &len, fp)) != -1)
				{
					//printf("read %zu \n", read);
					//printf("%s", line);
					enqueue(line,read);
				}
				//if(line)
				//	free(line);
				fclose(fp);
			}
		}
		printf("%d finish\n",omp_get_thread_num());

		int _count = __sync_fetch_and_add(&producer_count,1);
		printf("producer count: %d %d\n",_count,NUM/2);

		if( _count == NUM/2 -1)
		{
			//printf("%d last\n",omp_get_thread_num());
			printf("all producers finish\n");
			stop = 1;
			if(closedir(dir_stream))
				printf("closedir error: %d",errno);
		}
	}

	//consumer
	else if(omp_get_thread_num() >= NUM/2)
	{
		//goto end;
		//fgetc(stdin);
		char* string;
		char* token;
		char* save_ptr;

		while( list_head||!stop)
		{
			if(!dequeue(&string))
			{
				//printf("%d get %s",omp_get_thread_num(),string);
				//printf("%d get \n",omp_get_thread_num());
				//process string
	
				token = strtok_r(string," ",&save_ptr);

				while(token != NULL)
				{
					//printf("%s\n",token);
					examine(token);	
					token = strtok_r(NULL," ",&save_ptr);
				}

				//process end			
	
				free(string);
			}
		}
		end:
		printf("%d finish\n",omp_get_thread_num());	
	}

	}//omp finish

	for(int i =0; i<keyword_count; i++)
	{
		printf("%s: %d\n",keywords[i],keyword_table[i]);
	}
	
	clean_list();
	free(keywords[0]);
	free(keywords);
	free(keyword_table);
	return 0;
}

int enqueue(char* string, int read)
{
	struct entry* node_ptr;
	node_ptr = (struct entry*) malloc (sizeof(struct entry));
	node_ptr->string = (char*)malloc(sizeof(char)*(read+1));
	strcpy(node_ptr->string, string);
	node_ptr->len = read;
	node_ptr->next = NULL;
	#pragma omp critical(list)
	{
		//printf("len: %d \n", node_ptr->len);
		if(list_head)
			list_end->next= node_ptr;
		else
			list_head = node_ptr;

		list_end = node_ptr;
	}	
		//printf("len: %d \n", read);
		//printf("%s", string);
	return 0;
}

int dequeue(char** string)
{
	if(!list_head)
		return -1;
	
	entry* temp;
	#pragma omp critical(list)
	{
		*string = list_head->string;
		temp = list_head;
		list_head = list_head->next;
		free(temp);
	}

	return 0;
}

void clean_list()
{
	entry* temp;
	int c = 0;
	while(list_head)
	{
		c++;
		free(list_head->string);
		temp = list_head;
		list_head = list_head->next;	
		free(temp);
		//printf("clean one");
	}
	printf("clean %d entry\n",c);
}

void readkeyword()
{
	FILE* keyword_fp;
	char * keyword_line = NULL;
	char * get_line = NULL;

	size_t keyword_len = 0;
	ssize_t keyword_read;
	char *token;
	int size = 0;

	 

	keyword_fp = fopen(KEYWORD_FILE, "r");
	keyword_read = getline(&get_line, &keyword_len, keyword_fp);
	fclose(keyword_fp);	

	keyword_line = strdup(get_line);

	

	//count size
	token = strtok(get_line," ");
	while( token != NULL && *token != '\n' ) 
	{
		size++;
		//printf( " %s\n", token);
		token = strtok(NULL, " ");
	}
	printf("keyword count: %d\n",size);
	keyword_count = size;

	//fill into global keywords
	keywords = (char **) malloc(sizeof(char*)*size);
	keyword_table = (int*) malloc(sizeof(int) * size);
	
	memset(keyword_table, 0, size*sizeof(int));

	keywords[0] =  strtok(keyword_line," ");
	printf("%s\n",keywords[0]);
	for(int i =1; i<size; i++)
	{
		keywords[i] = strtok(NULL, " ");
		printf("%s: %d\n",keywords[i],keyword_table[i]);
	}
	
	free(get_line);
}

void examine(char* string)
{
	for(int i=0; i<keyword_count;i++)
	{
		char* p = string;
		for ( ; *p; ++p) *p = tolower(*p);
		//printf("%s com to %s\n",string,keywords[i]);
		if(! strcmp(string,keywords[i]))
		{
			//printf("%d get %s\n",omp_get_thread_num(),string);
			__sync_fetch_and_add(&(keyword_table[i]),1);
		}
	}
}




