#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "pixel.h"

void smooth(struct pixel **in, struct pixel **out, int w, int h, int n) {
    struct pixel **tmp, **dest;
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank != 0) {
        return;
    }

    dest = out;
    for (int t = 0; t < n; t++ ) {
        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {
                int top   = y > 0   ? y-1 : h-1;
                int down  = y < h-1 ? y+1 : 0;
                int left  = x > 0   ? x-1 : w-1;
                int right = x < w-1 ? x+1 : 0;

                dest[x][y].r = (double) (in[x][y].r + in[x][top].r + in[x][down].r 
                        + in[left][y].r + in[right][y].r ) / 5 +0.5;
                dest[x][y].g = (double) (in[x][y].g + in[x][top].g + in[x][down].g 
                        + in[left][y].g + in[right][y].g ) / 5 +0.5;
                dest[x][y].b = (double) (in[x][y].b + in[x][top].b + in[x][down].b 
                        + in[left][y].b + in[right][y].b ) / 5 +0.5;
            }
        }
        tmp  = dest;
        dest = in;
        in   = tmp;
    }
    if (out != in) {
        memcpy ( out[0], in[0], w * h * sizeof(struct pixel));
    }
}
