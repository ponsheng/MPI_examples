#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "pixel.h"

void smooth(struct pixel *in, struct pixel *out, int w, int h, int n) {
    struct pixel *tmp, *dest;
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Bcast(&h, 1, MPI_INT , 0, MPI_COMM_WORLD);
    MPI_Bcast(&w, 1, MPI_INT , 0, MPI_COMM_WORLD);

    if (rank != 0) {
        return;
        in = (struct pixel*) malloc(h * w * sizeof(struct pixel));
        out = (struct pixel*) malloc(h * w * sizeof(struct pixel));

    }

    dest = out;
    for (int t = 0; t < n; t++ ) {
        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {
                int top = y > 0 ? y-1 : h-1;
                int down = y < h-1 ? y+1 : 0;
                int left = x > 0 ? x-1 : w-1;
                int right = x < w-1 ? x +1 : 0;

                (dest+x*h+y)->r = (double) ((in+x*h+y)->r + (in+x*h+top)->r + (in+x*h+down)->r + (in+left*h+y)->r + (in+right*h+y)->r ) / 5 +0.5;
                (dest+x*h+y)->g = (double) ((in+x*h+y)->g + (in+x*h+top)->g + (in+x*h+down)->g + (in+left*h+y)->g + (in+right*h+y)->g ) / 5 +0.5;
                (dest+x*h+y)->b = (double) ((in+x*h+y)->b + (in+x*h+top)->b + (in+x*h+down)->b + (in+left*h+y)->b + (in+right*h+y)->b ) / 5 +0.5;
            }
        }
        tmp = dest;
        dest = in;
        in = tmp;
    }

    if (rank != 0) {
        free(in);
        free(out);
    } else {
        if (out != in) {
            memcpy ( out, in, w * h * sizeof(struct pixel));
        }
    }
}
