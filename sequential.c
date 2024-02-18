#include <stdio.h>
#include <time.h>

#define WIDTH 1000
#define HEIGHT 1000

// Define the complex structure
typedef struct {
    float real;
    float imag;
} complex;

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

int cal_pixel(complex c);

int main() {
    clock_t start, end;
    double execution_time;
    start = clock();

    // Define the parameters for the Mandelbrot Set visualization
    const float x_min = -2.0;
    const float x_max = 1.0;
    const float y_min = -1.5;
    const float y_max = 1.5;
    const int max_iterations = 256;

    // Create a 2D array to store the iteration counts
    int mandelbrot[WIDTH][HEIGHT];

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            // Map pixel coordinates to the complex plane
            complex c;
            c.real = x * (x_max - x_min) / (WIDTH - 1) + x_min;
            c.imag = y * (y_max - y_min) / (HEIGHT - 1) + y_min;

            // Calculate the number of iterations for this pixel
            mandelbrot[x][y] = cal_pixel(c);
        }
    }

    end = clock();
    execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Print the 2D array
   /* for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            printf("%d ", mandelbrot[x][y]);
        }
        printf("\n"); // Newline after each row
    }*/

    printf("Execution time: %f seconds\n", execution_time);
    save_ppm("mandelbrotttt.ppm", WIDTH, HEIGHT, &mandelbrot[0][0]);

    return 0;
}

// The cal_pixel function remains the same as in your original code
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
