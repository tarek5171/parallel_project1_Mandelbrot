#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

typedef struct {
    double real, imag;
} complex_t;

// Compute the Mandelbrot set value for a given complex number
int compute_mandelbrot_point(complex_t c, int max_iter) {
    complex_t z = { 0, 0 };
    int i;
    for (i = 0; i < max_iter; i++) {
        if (z.real * z.real + z.imag * z.imag > 4.0) {
            break;
        }
        complex_t temp = { z.real * z.real - z.imag * z.imag + c.real, 2 * z.real * z.imag + c.imag };
        z = temp;
    }
    return i;
}

// Compute the Mandelbrot set for a given quadrant of the image
void compute_mandelbrot_quadrant(double xmin, double xmax, double ymin, double ymax,
                                 int width, int height, int max_iter, int quadrant,
                                 int *mandelbrot_set) {
    int i, j;
    int row_start, row_end, col_start, col_end;
    double xstep = (xmax - xmin) / width;
    double ystep = (ymax - ymin) / height;

    // Determine the row and column ranges for this quadrant
    switch (quadrant) {
        case 0:
            row_start = 0;
            row_end = height / 2;
            col_start = 0;
            col_end = width / 2;
            break;
        case 1:
            row_start = 0;
            row_end = height / 2;
            col_start = width / 2;
            col_end = width;
            break;
        case 2:
            row_start = height / 2;
            row_end = height;
            col_start = 0;
            col_end = width / 2;
            break;
        case 3:
            row_start = height / 2;
            row_end = height;
            col_start = width / 2;
            col_end = width;
            break;
    }

    // Compute the Mandelbrot set for this quadrant
    for (i = row_start; i < row_end; i++) {
        for (j = col_start; j < col_end; j++) {
            double x = xmin + j * xstep;
            double y = ymin + i * ystep;
            complex_t c = { x, y };
            mandelbrot_set[i * width + j] = compute_mandelbrot_point(c, max_iter);
        }
    }
}

// Compute the Mandelbrot set using OpenMP
int *parallel_compute_mandelbrot_set(double xmin, double xmax, double ymin, double ymax,
                                     int width, int height, int max_iter) {
    int i, j;
    int *mandelbrot_set = (int *) malloc(sizeof(int) * width * height);

    // Divide the image into four quadrants and compute each quadrant in parallel
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        compute_mandelbrot_quadrant(xmin, xmax, ymin, ymax, width, height, max_iter, tid, mandelbrot_set);
    }

    return mandelbrot_set;
}

int main() {
    double xmin = -2.0;
    double xmax = 2.0;
    double ymin = -2.0;
    double ymax = 2.0;
    int width = 800;
    int height = 800;
    int max_iter = 1000;

    // Compute the Mandelbrot set using OpenMP
    int *mandelbrot_set = parallel_compute_mandelbrot_set(xmin, xmax, ymin, ymax, width, height, max_iter);

    // Write the Mandelbrot set to a file
    FILE *fp;
    fp = fopen("mandelbrot_set.pgm", "w");
    fprintf(fp, "P2\n%d %d\n%d\n", width, height, max_iter);
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            fprintf(fp, "%d ", mandelbrot_set[i * width + j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    free(mandelbrot_set);
    return 0;
}
