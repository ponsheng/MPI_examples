#include <stdio.h>
#include <string.h>	//For strlen
#include <mpi.h>	//For MPI functions

#define MAX_STRING 100

int main(void)
{
	char greeting[MAX_STRING];
	int comm_size;
	int my_id;

	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&comm_size);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_id);
	
	if(my_id != 0)
	{
		sprintf(greeting, "Greetings from process %d of %d!"
			,my_id,comm_size);
		MPI_Send(greeting,strlen(greeting),MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}
	else
	{
		printf("Greeting from process %d of %d!\n", my_id,comm_size);
		int q =1;
		MPI_Status status;
		int number;
		for(; q<comm_size;q++)
		{
			MPI_Recv(greeting, MAX_STRING, MPI_CHAR, q, 0,
				MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &number);
			printf("%s\n",greeting);
			printf("Received %d numbers Message source = %d, "
            	"tag = %d\n",
            	number, status.MPI_SOURCE, status.MPI_TAG);
		}
	}
	MPI_Finalize();
	return 0;
}

