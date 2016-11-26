#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

int thread_count;

void *hello(void* rank);

int main(int argc,char* argv[])
{
	long thread;
	pthread_t* thread_id;

	thread_count = strtol(argv[1], NULL, 10);
	thread_id = malloc(thread_count * sizeof(pthread_t));

	for(thread = 0;thread < thread_count;thread++)
		pthread_create( &thread_id[thread], NULL, hello, (void*) thread);

	for(thread = 0;thread < thread_count;thread++)
		pthread_join( thread_id[thread], NULL);

	free(thread_id);
	return 0;
}

void *hello(void* rank)
{
	long my_id = (long) rank;
	printf("%ld out of %d\n", my_id, thread_count);
	return NULL;
}
