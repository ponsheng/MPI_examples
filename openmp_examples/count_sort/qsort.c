#include <stdlib.h>

int cmpfunc(const void * a, const void * b)
{
	return (*(int*)a > *(int*)b);
}

void Count_sort (int a [], int n )
{
	qsort(a,n,sizeof(int),cmpfunc);
}
