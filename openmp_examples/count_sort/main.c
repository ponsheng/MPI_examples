#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define RANGE 1000000
#define BASE 0

void print_array(int n,int *array);
void Count_sort (int a [], int n );

int main(int argc,char **argv)
{
	if(argc <2)
	{
		printf("Usage: count_sort N \n\n  N - size of array to sort\n");
		exit(0);
	}

	struct timeval start, end;	
	srand( time(NULL) );

	int num = strtol( argv[1], NULL, 10);;
	int a[num];

	for(int i=0; i<num; i++)
	{
		a[i] = rand() % RANGE + BASE;
	}	

	//print_array(num,a);

	gettimeofday(&start, NULL);

	Count_sort(a, num );

	gettimeofday(&end, NULL);
	
	//print_array(num,a);

	printf("%ld us \n", ((end.tv_sec -                                       
	start.tv_sec)*1000*1000+(end.tv_usec - start.tv_usec)));

	return 0;
}

void print_array(int n,int *array)
{
	for(int i=0; i<n; i++)
		printf("%d ",array[i]);
	printf("\n");
}
