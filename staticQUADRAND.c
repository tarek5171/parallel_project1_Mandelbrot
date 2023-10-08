#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

typedef struct {
    double real, imag;
} Complex;

int computeMandelbrotPoint(Complex c, int maxIter) {
    Complex z = {0, 0};
    int iterations;
    for (iterations = 0; iterations < maxIter; iterations++) {
        if (z.real * z.real + z.imag * z.imag > 4.0) {
            break;
        }
        Complex temp = {
            z.real * z.real - z.imag * z.imag + c.real,
            2 * z.real * z.imag + c.imag
        };
        z = temp;
    }
    return iterations;
}

void computeMandelbrotQuadrant(double xmin, double xmax, double ymin, double ymax,
                                int width, int height, int maxIter, int quadrant,
                                int *mandelbrotSet) {
    int i, j;
    int rowStart, rowEnd, colStart, colEnd;
    double xStep = (xmax - xmin) / width;
    double yStep = (ymax - ymin) / height;

    switch (quadrant) {
        case 0:
            rowStart = 0;
            rowEnd = height / 2;
            colStart = 0;
            colEnd = width / 2;
            break;
        case 1:
            rowStart = 0;
            rowEnd = height / 2;
            colStart = width / 2;
            colEnd = width;
            break;
        case 2:
            rowStart = height / 2;
            rowEnd = height;
            colStart = 0;
            colEnd = width / 2;
            break;
        case 3:
            rowStart = height / 2;
            rowEnd = height;
            colStart = width / 2;
            colEnd = width;
            break;
    }

    for (i = rowStart; i < rowEnd; i++) {
        for (j = colStart; j < colEnd; j++) {
            double x = xmin + j * xStep;
            double y = ymin + i * yStep;
            Complex c = {x, y};
            mandelbrotSet[i * width + j] = computeMandelbrotPoint(c, maxIter);
        }
    }
}

int *parallelComputeMandelbrotSet(double xmin, double xmax, double ymin, double ymax,
                                   int width, int height, int maxIter) {
    int *mandelbrotSet = (int *)malloc(sizeof(int) * width * height);

    double startTime = omp_get_wtime(); // Record start time

    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        computeMandelbrotQuadrant(xmin, xmax, ymin, ymax, width, height, maxIter, tid, mandelbrotSet);
    }

    double endTime = omp_get_wtime(); // Record end time
    printf("Elapsed time: %f seconds\n", endTime - startTime);

    return mandelbrotSet;
}

int main() {
    double xmin = -2.0;
    double xmax = 2.0;
    double ymin = -2.0;
    double ymax = 2.0;
    int width = 1000;
    int height = 1000;
    int maxIter = 256;

    int *mandelbrotSet = parallelComputeMandelbrotSet(xmin, xmax, ymin, ymax, width, height, maxIter);

    FILE *fp;
    fp = fopen("mandelbrot_setttt.pgm", "w");
    fprintf(fp, "P2\n%d %d\n%d\n", width, height, maxIter);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fprintf(fp, "%d ", mandelbrotSet[i * width + j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    free(mandelbrotSet);
    return 0;
}
