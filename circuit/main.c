/*ing a brute-force sequential solution.
 *
 *   The particular circuit being tested is "wired" into the
 *   logic of function 'checkCircuit'. All combinations of
 *   inputs that satisfy the circuit are printed.
 *
 *   16-bit version by Michael J. Quinn, Sept 2002.
 *   Extended to 32 bits by Joel C. Adams, Sept 2013.
 */

#include <stdio.h>     // printf()
#include <limits.h>    // UINT_MAX
#include <mpi.h>

#define xstr(s) str(s)
#define str(s) #s

#if COMM == 1
#define COMM_type SERIAL
#elif COMM == 2
#define COMM_type TREE
#endif

int checkCircuit (int, long);

int main (int argc, char *argv[]) {
   	long i;               /* loop variable (64 bits) */
   	int count = 0;        /* number of solutions */
	int comm_size;
	int my_id;
		

	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	i = my_id;

	if(my_id == 0)
		printf("Communication method: %s\n",xstr (COMM_type));

	double startTime = 0.0, totalTime = 0.0;
    startTime = MPI_Wtime();
	for(; i<UINT_MAX/25536;i+=comm_size)
	{
		count += checkCircuit (my_id, i);
	}
	totalTime = MPI_Wtime() - startTime;
    printf("Process %d finished in time %f secs. comm_size: %d\n", my_id, totalTime, comm_size);
	fflush (stdout);

#if COMM == 2
	int index = 1;
	int a;
	int total_count = count;
	int half;
	while(index  < comm_size)
	{
		index *= 2;
		half = index/2;
		if(my_id == 0)
			printf("id = 0 ,%d %d %d",index ,half,comm_size);
		if(my_id % index == 0)
		{
			//printf("Rev index: %d \n",index);
			MPI_Recv(&count, sizeof(int), MPI_INT, my_id + half, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			total_count += count;
			//printf("%d recev from %d\n", my_id, my_id + half);
		}
		else if((my_id % index) == half)
		{
			//printf("Send index: %d \n",index);
			MPI_Send(&total_count,sizeof(int),MPI_INT, my_id - half, 0, MPI_COMM_WORLD);
		}
	}
	//printf("jump out while %d %d %d \n",index,my_id, comm_size);
	if(my_id ==0 )
	{
		printf("\nA total of %d solutions were found.\n\n", total_count);
	}
	
	
#elif  COMM == 1
    if(my_id == 0)
    {
        int q =1;
        int total_count = count;
		
        for(; q<comm_size;q++)
        {
            MPI_Recv(&count, sizeof(int), MPI_INT, q, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			total_count += count;
            printf("receive from %d \n",q);
        }
		printf("\nA total of %d solutions were found.\n", total_count);
		printf("Compute over\n");
		totalTime = MPI_Wtime() - startTime;
    	printf("Total elapsed :time %f secs.\n", totalTime);
    }
	else
	{
		MPI_Send(&count,sizeof(int),MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
#endif

	MPI_Finalize();
   	return 0;
}

/* EXTRACT_BIT is a macro that extracts the ith bit of number n.
 *
 * parameters: n, a number;
 *             i, the position of the bit we want to know.
 *
 * return: 1 if 'i'th bit of 'n' is 1; 0 otherwise 
 */

#define EXTRACT_BIT(n,i) ( (n & (1<<i) ) ? 1 : 0)


/* checkCircuit() checks the circuit for a given input.
 * parameters: id, the id of the process checking;
 *             bits, the (long) rep. of the input being checked.
 *
 * output: the binary rep. of bits if the circuit outputs 1
 * return: 1 if the circuit outputs 1; 0 otherwise.
 */

#define SIZE 32

int checkCircuit (int id, long bits) {
   int v[SIZE];        /* Each element is a bit of bits */
   int i;

   for (i = 0; i < SIZE; i++)
     v[i] = EXTRACT_BIT(bits,i);

   if ( ( (v[0] || v[1]) && (!v[1] || !v[3]) && (v[2] || v[3])
       && (!v[3] || !v[4]) && (v[4] || !v[5])
       && (v[5] || !v[6]) && (v[5] || v[6])
       && (v[6] || !v[15]) && (v[7] || !v[8])
       && (!v[7] || !v[13]) && (v[8] || v[9])
       && (v[8] || !v[9]) && (!v[9] || !v[10])
       && (v[9] || v[11]) && (v[10] || v[11])
       && (v[12] || v[13]) && (v[13] || !v[14])
       && (v[14] || v[15]) )
       ||
          ( (v[16] || v[17]) && (!v[17] || !v[19]) && (v[18] || v[19])
       && (!v[19] || !v[20]) && (v[20] || !v[21])
       && (v[21] || !v[22]) && (v[21] || v[22])
       && (v[22] || !v[31]) && (v[23] || !v[24])
       && (!v[23] || !v[29]) && (v[24] || v[25])
       && (v[24] || !v[25]) && (!v[25] || !v[26])
       && (v[25] || v[27]) && (v[26] || v[27])
       && (v[28] || v[29]) && (v[29] || !v[30])
       && (v[30] || v[31]) ) )
   {
      /*printf ("%d) %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d \n", id,
         v[31],v[30],v[29],v[28],v[27],v[26],v[25],v[24],v[23],v[22],
         v[21],v[20],v[19],v[18],v[17],v[16],v[15],v[14],v[13],v[12],
         v[11],v[10],v[9],v[8],v[7],v[6],v[5],v[4],v[3],v[2],v[1],v[0]);
      fflush (stdout);*/
      return 1;
   } else {
      return 0;
   }
}


