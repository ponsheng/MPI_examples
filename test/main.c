#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 4

int main(int argc, char *argv[])
{
    int rank, size;     // for storing this process' rank, and the number of processes
    int *sendcounts;    // array describing how many elements to send to each process
    int *displs;        // array describing the displacements where each segment begins

    int sum = 0;                // Sum of counts. Used to calculate displacements
    int rec_buf[100];          // buffer where the received data should be stored

    // the data to be distributed
    int data[SIZE][SIZE] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	int rem = (SIZE*SIZE)%size; // elements remaining after division among processes 

    sendcounts = malloc(sizeof(int)*size);
    displs = malloc(sizeof(int)*size);

    // calculate send counts and displacements
    int i;
    for(i = 0; i < size; i++) {
        sendcounts[i] = (SIZE*SIZE)/size;

        displs[i] = sum;
        sum += sendcounts[i];
    }

    // print calculated send counts and displacements for each process
    if (0 == rank) {
        for (i = 0; i < size; i++) {
            printf("sendcounts[%d] = %d\tdispls[%d] = %d\n", i, sendcounts[i], i, displs[i]);
        }
    }

    // divide the data among processes as described by sendcounts and displs
    MPI_Scatterv(data, sendcounts, displs, MPI_INT, rec_buf, SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    // print what each process received
    printf("%d: ", rank);
    for (i = 0; i < sendcounts[rank]; i++) {
        printf("%d\t", rec_buf[i]);
    }
    printf("\n");

    MPI_Finalize();

    free(sendcounts);
    free(displs);

    return 0;
}
