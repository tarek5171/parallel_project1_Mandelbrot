#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define WIDTH 1000
#define HEIGHT 1000
#define task_size 2

// Function to save the 2D array as a PPM image file
void save_ppm(const char* filename, int width, int height, int* data) {
    FILE* fp;
    fp = fopen(filename, "wb");
    fprintf(fp, "P6\n%d %d\n255\n", width, height);

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int color = data[row * width + col] % 256;
            fputc(color, fp);  // Red
            fputc(color, fp);  // Green
            fputc(color, fp);  // Blue
        }
    }

    fclose(fp);
}

// Define a struct to represent a complex number
typedef struct {
    float real;
    float imag;
} complex;

int cal_pixel(complex c) {
    int count, max;
    complex z;
    float temp, lengthsq;
    max = 256;
    z.real = 0;
    z.imag = 0;
    count = 0;

    do {
        temp = z.real * z.real - z.imag * z.imag + c.real;
        z.imag = 2 * z.real * z.imag + c.imag;
        z.real = temp;
        lengthsq = z.real * z.real + z.imag * z.imag;
        count++;
    } while ((lengthsq < 4.0) && (count < max));

    return count;
}

int main(int argc, char* argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        fprintf(stderr, "This program requires at least 2 processes.\n");
        MPI_Finalize();
        return 1;
    }

    clock_t start, end;
    double execution_time;

    if (rank == 0) { // Master process
        start = clock();

        int mandelbrot[WIDTH][HEIGHT];

        const float x_min = -2.0;
        const float x_max = 1.0;
        const float y_min = -1.5;
        const float y_max = 1.5;
        int rows_per_process = HEIGHT / (size - 1);
        int remaining_rows = HEIGHT % (size - 1);
        int start_row = 0;

        for (int dest = 1; dest < size; dest++) {
            int end_row = start_row + rows_per_process + (dest <= remaining_rows ? 1 : 0);

            MPI_Send(&start_row, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            MPI_Send(&end_row, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

            start_row = end_row;
        }

        for (int dest = 1; dest < size; dest++) {
            int recived[task_size][HEIGHT];
            MPI_Recv(&recived, sizeof(int) * task_size * HEIGHT, MPI_BYTE, dest, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int nx = 0; nx < task_size; nx++) {
                for (int ny = 0; ny < HEIGHT; ny++) {
                    mandelbrot[nx + nx * task_size][ny] = recived[nx][ny];
                }
            }
        }

        end = clock();
        execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;

        printf("Execution time: %f seconds\n", execution_time);

        save_ppm("mandelbrot.ppm", WIDTH, HEIGHT, &mandelbrot[0][0]);
    } else { // Slave processes
        int start_row, end_row;
        MPI_Recv(&start_row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&end_row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int task_array2[task_size][HEIGHT];

        const float x_min = -2.0;
        const float x_max = 1.0;
        const float y_min = -1.5;
        const float y_max = 1.5;

        for (int x = start_row; x < end_row; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                complex c;
                c.real = x * (x_max - x_min) / (WIDTH - 1) + x_min;
                c.imag = y * (y_max - y_min) / (HEIGHT - 1) + y_min;

                task_array2[x - start_row][y] = cal_pixel(c);
            }
        }

        MPI_Send(&task_array2[0][0], task_size * HEIGHT, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}

