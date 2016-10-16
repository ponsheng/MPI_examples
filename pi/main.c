#include <stdio.h>     // printf()
#include <limits.h>    // UINT_MAX
#include <mpi.h>

int main (int argc, char *argv[]) {
    long i;               /* loop variable (64 bits) */
    int count = 0;        /* number of solutions */                             
    int comm_size_tmp;
    int my_id;
        

    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size_tmp);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	const int comm_size = comm_size_tmp;
	//counting time
	double startTime = 0.0, totalTime = 0.0;
    startTime = MPI_Wtime();

	/*compute start*/


	/*compute over*/

    int index = 1;
    int a;
    int total_count = count;
    int half;

    while(index  < comm_size)
    {
        index *= 2;
        half = index/2;
        if(my_id % index == 0)
        {
            MPI_Recv(&count, sizeof(int), MPI_INT, my_id + half, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_count += count;
        }
        else if((my_id % index) == half)
        {
            MPI_Send(&total_count,sizeof(int),MPI_INT, my_id - half, 0, MPI_COMM_WORLD);
        }
    }

    if(my_id ==0 )
    {
        printf("\nA total of %d solutions were found.\n\n", total_count);
        totalTime = MPI_Wtime() - startTime;
        printf("Total elapsed :time %f secs.\n", totalTime);
    }

	MPI_Finalize();
    return 0;
}
