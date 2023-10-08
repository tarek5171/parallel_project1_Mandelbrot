#include <stdio.h>
#include <stdlib.h>
# include<time.h>
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

typedef struct {
    float real;
    float imag;
    int pixel_res;
} solution;

// Function to calculate the pixel value
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

        // Create a 2D array to store the iteration counts
        int mandelbrot[WIDTH][HEIGHT];

        const float x_min = -2.0;
        const float x_max = 1.0;
        const float y_min = -1.5;
        const float y_max = 1.5;
        complex task;
        int task_index = 0;

        
        for (int x = 0; x < WIDTH; x+=task_size) {

            int x2 = x+task_size;


            //recieve slave rank of done slaves
            int slave_rank;
            MPI_Recv(&slave_rank, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
           

            // Print the task before sending it to the slave
            //printf("Sending to slave %d real: %.2f imag: %.2f\n", slave_rank, task.real, task.imag);

            // Send the task to the slave using MPI_FLOAT data type
            MPI_Send(&x, 1, MPI_INT, slave_rank, 0, MPI_COMM_WORLD);
            MPI_Send(&x2, 1, MPI_INT, slave_rank, 0, MPI_COMM_WORLD);

            //recieve result from slaves and add to 2d array
            int recived[task_size][HEIGHT];
            MPI_Request request;
           
            MPI_Irecv(&recived, sizeof(int) * task_size * HEIGHT, MPI_BYTE, slave_rank, 0, MPI_COMM_WORLD, &request);
            // Wait for the task index to be received
            //MPI_Wait(&request, MPI_STATUS_IGNORE);

            // Add the received results to the mandelbrot array
            
            //printf("\n");

            for (int nx = 0; nx < task_size; nx++) {
            for (int ny = 0; ny < HEIGHT; ny++) {
               //printf("%d %d%d ", recived[nx][ny],nx+x,ny);
            mandelbrot[nx+x][ny] = recived[nx][ny];
            }
            //printf("\n"); // Newline after each row
        }   
        //printf("\n");






            //int result[HEIGHT];
            //MPI_Recv(&result, 1, MPI_INT, slave_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
             //MPI_Recv(result, HEIGHT, MPI_INT, slave_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("%d slave %d sent result %d\n",x,slave_rank, result[0]);

           // for(int i=0;i<HEIGHT;i++){
                //mandelbrot[x][i]=result[i];
                //printf("%d ",mandelbrot[WIDTH][i]);
            //}

            //printf("\n");

            

            task_index++;
        }

        
        // Signal to slaves that no more tasks are available
        for (int i = 1; i < size; i++) {
            int end=-100;
            MPI_Send(&end, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        



        end = clock();
        execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;
//Print the 2D array
       /* for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            printf("%d ", mandelbrot[x][y]);
        }
        printf("\n"); // Newline after each row
    }*/
    
            

    printf("Execution time: %f seconds\n", execution_time);

    save_ppm("mandelbrotttyy.ppm", WIDTH, HEIGHT, &mandelbrot[0][0]);
    } else { // Slave processes
        while (1) {
            int task;
            int task2;
            MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // Signal the master that this slave is ready for a task
            MPI_Recv(&task, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&task2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


            
            if (task == -100) { // No more tasks, exit
                break;
            }

            // Perform the cal_pixel computation on the complex number
            int x1 = task;
            int x2=task2;
            
            int task_array2[task_size][HEIGHT];

            // Define the parameters for the Mandelbrot Set visualization
            const float x_min = -2.0;
            const float x_max = 1.0;
            const float y_min = -1.5;
            const float y_max = 1.5;
            const int max_iterations = 256;


            for (int x = x1; x < x2; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    //printf("R %d : %d \n",x,y);
                // Map pixel coordinates to the complex plane
                complex c;
                c.real = x * (x_max - x_min) / (WIDTH - 1) + x_min;
                c.imag = y * (y_max - y_min) / (HEIGHT - 1) + y_min;

                // Calculate the number of iterations for this pixel
                task_array2[x-x1][y] = cal_pixel(c);
                //printf("%d ",task_array2[x][y]);
                 
                }
                //printf("\n");
            }
            //printf("\n");

            // Print the 2D array
             //for (int x = 0; x < 3; x++) {
            //for (int y = 0; y < HEIGHT; y++) {
            //   printf("%d ", task_array2[x][y]);
           // }
          //  printf("\n"); // Newline after each row
       // }   
      //  printf("\n");
             

            //printf("\n");

            //send result to master

        MPI_Send(&task_array2[0][0], task_size * HEIGHT, MPI_INT, 0, 0, MPI_COMM_WORLD);
            //MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            //MPI_Send(task_array, HEIGHT, MPI_INT, 0, 0, MPI_COMM_WORLD);

            // Print the result
           // printf("Slave %d computed pixel value: %d\n", rank, result);



            
        }
    }

    MPI_Finalize();

    //printf("%d",execution_time);
    return 0;
}
