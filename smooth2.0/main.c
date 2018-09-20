/**************************************************************

        The program reads an BMP image file and creates a new
        image that is the negative of the input file.

**************************************************************/
#include "qdbmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "pixel.h"

#ifndef NSmooth
#define NSmooth 100
#endif

void smooth(struct pixel*, struct pixel*, int, int, int);

int main(int argc, char *argv[]) {
    double startwtime, endwtime;
    UCHAR r, g, b;
    UINT width, height;
    UINT x, y;
    BMP *bmp;
    struct pixel *in, *out;
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    /* Check arguments */
    if (argc != 3) {
        if (rank==0) {
            fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        }
        return 0;
    }

    if (rank == 0) {

        /* Read an image file */
        bmp = BMP_ReadFile(argv[1]);

        if ( BMP_GetError() != BMP_OK ) {
            fprintf( ( stdout ), "BMP error: %s\n", BMP_GetErrorDescription() );
            MPI_Abort(MPI_COMM_WORLD, -1);
            return -1;
        }

        /* Get image's dimensions */
        width = BMP_GetWidth(bmp);
        height = BMP_GetHeight(bmp);

        printf("Read bmp file %s, Depth: %d, Width: %d, Height: %d\n", 
                argv[1], BMP_GetDepth( bmp ), width, height);
    
        in = (struct pixel*) malloc(height * width * sizeof(struct pixel));
        out = (struct pixel*) malloc(height * width * sizeof(struct pixel));

        /* Iterate through all the image's pixels */
        for (x = 0; x < width; ++x) {
            for (y = 0; y < height; ++y) {
                /* Get pixel's RGB values */
                BMP_GetPixelRGB(bmp, x, y, &r, &g, &b);
                (in+x*height+y)->r = r;
                (in+x*height+y)->g = g;
                (in+x*height+y)->b = b;
            }
        }
        startwtime = MPI_Wtime();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    smooth(in, out, width, height, NSmooth);

    if (rank == 0) {
        endwtime = MPI_Wtime();
        printf("Time: %lf\n", endwtime - startwtime);

        /* Iterate through all the image's pixels */
        for (x = 0; x < width; ++x) {
            for (y = 0; y < height; ++y) {
                r = (out+x*height+y)->r;
                g = (out+x*height+y)->g;
                b = (out+x*height+y)->b;

                BMP_SetPixelRGB(bmp, x, y, r, g, b);
            }
        }

        /* Save result */
        BMP_WriteFile(bmp, argv[2]);
        if ( BMP_GetError() != BMP_OK ) {
            fprintf( ( stdout ), "BMP error: %s\n", BMP_GetErrorDescription() );
            MPI_Abort(MPI_COMM_WORLD, -2);
            return -2;
        }

        /* Free all memory allocated for the image */
        BMP_Free(bmp);
    }
    free(in);
    free(out);
    MPI_Finalize();
    return 0;
}
