#include <stdio.h>     // printf()
#include <mpi.h>

int main (int argc, char *argv[]) {
    int comm_size_tmp;
    int my_id;
    
    int name_len;
    char machine[MPI_MAX_PROCESSOR_NAME];

        

    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size_tmp);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Get_processor_name(machine, &name_len);
    const int comm_size = comm_size_tmp;

    printf("Size= %d,id= %d,machine= %s,name_len= %d\n",comm_size,my_id,machine,name_len);

	MPI_Finalize();
    return 0;
}
