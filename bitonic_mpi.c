#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
//mpirun -np 4 ./bitonic_mpi

const int SIZE = (1<< 18);
const int ITERATIONS = 4;
int myrank, processors;

void generate_random_array(int array[], int size, unsigned int seed);
void swap(int *p1, int *p2);
int partition(int array[], int low, int high);
void quicksort(int array[], int low, int high);
void display_array(int *array, int size, const char *name);
void compareLow(int *local_array,int SIZE, int myrank,int j);
void compareHigh(int *local_array,int SIZE, int myrank,int j);

int main(int argc, char *argv[]) {
    //printf("Number of elements being generated: %i \n", SIZE*ITERATIONS);
    double start_time, end_time;
   unsigned int initial_seed = (unsigned int)time(NULL);

    //Initializing Comm group
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &processors);
    int global_size = SIZE * ITERATIONS;

    int *local_array = (int *)malloc(SIZE*sizeof(int));
    int *global_array = (int *)malloc(global_size*sizeof(int));
   
    //Ensure that all processors generate the array correctly 
  
    generate_random_array(local_array,SIZE,initial_seed + myrank);
    MPI_Barrier(MPI_COMM_WORLD);
    //display_array(local_array,SIZE,"Generated array");
      if (myrank == 0) {
        start_time = MPI_Wtime();
    }
     quicksort(local_array,0, SIZE- 1);
    //going through hypercube
    int dimensions = log2(4);
    for(int i =0; i < dimensions ; i++){
        //bit comparison loop
        for(int j=i; j >=0;j--){
            //Determining positions in hypercube network with ranks
            if(((myrank >> (i +1))%2==0 && (myrank >> j)%2==0)||((myrank >>(i+1))%2 !=0 && (myrank >> j)%2 !=0)){
                //both even rank
                compareLow(local_array,SIZE,myrank,j);
            }
            else{
                //both odd rank
                compareHigh(local_array,SIZE,myrank,j);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (myrank == 0) {
        end_time = MPI_Wtime()-start_time;
       printf("Time Elapsed (Sec): %f\n", end_time - start_time);
    }
    

    MPI_Gather(local_array, SIZE, MPI_INT, global_array, SIZE, MPI_INT, 0, MPI_COMM_WORLD);
      if (myrank == 0) {
        //display_array(global_array, global_size, "Gathered array");
        free(global_array);
    }



    // Free dynamically allocated memory for the local array
    free(local_array);
    MPI_Finalize();
    return 0;
}

void generate_random_array(int *array, int size, unsigned int seed){
    srand(seed);
    for(int i =0 ; i < size;i++){
        array[i] = rand() %101;
    }
}
//Swapping elements
void swap(int *p1, int *p2) {
    int temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

//partiting based on pivot
int partition(int array[], int low, int high) {
    int pivot = array[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (array[j] < pivot) {
            i++;
            swap(&array[i], &array[j]);
        }
    }

    swap(&array[i + 1], &array[high]);
    return i + 1;
}

//recursive quicksort method
void quicksort(int *array, int low, int high) {
    if (low < high) {
        int pi = partition(array, low, high);

        quicksort(array, low, pi - 1);
        quicksort(array, pi + 1, high);
    }
}

// Function to display an array
void display_array(int *array, int size, const char *name) {
    printf("%s:\n", name);
    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("\n\n");
}

void compareLow(int *local_array,int SIZE, int myrank,int j){
    int neighbour = myrank ^ (1<<j);
    int *receivebuf = (int*) malloc(SIZE * sizeof(int));
    MPI_Sendrecv(local_array, SIZE, MPI_INT, neighbour, 0, receivebuf, SIZE, MPI_INT, neighbour, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    int* merged = (int*) malloc(2 * SIZE * sizeof(int));
    int i = 0, k = 0, l = 0;
     while (k < SIZE && l < SIZE) {
        if (local_array[k] < receivebuf[l]) {
            merged[i++] = local_array[k++];
        } else {
            merged[i++] = receivebuf[l++];
        }
    }
    while (k < SIZE) {
        merged[i++] = local_array[k++];
    }

    while (l < SIZE) {
        merged[i++] = receivebuf[l++];
    }

    for (i = 0; i < SIZE; i++) {
        local_array[i] = merged[i];
    }

    free(receivebuf);
    free(merged);

}

void compareHigh(int *local_array,int SIZE, int myrank,int j){
    int neighbour = myrank ^ (1<<j);
    int *receivebuf = (int*) malloc(SIZE * sizeof(int));
    
    MPI_Sendrecv(local_array, SIZE, MPI_INT, neighbour, 0, receivebuf, SIZE, MPI_INT, neighbour, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    int *merged = (int*) malloc(2 * SIZE * sizeof(int));
    
    int i = 0, k = SIZE-1, l = SIZE-1;
     
     while (k >= 0 && l >= 0) {
        if (local_array[k] > receivebuf[l]) {
            merged[i++] = local_array[k--];
        } else {
            merged[i++] = receivebuf[l--];
        }
    }
    while (k >=0) {
        merged[i++] = local_array[k--];
    }

    while (l >= 0) {
        merged[i++] = receivebuf[l--];
    }

    for (i = 0; i < SIZE; i++) {
        local_array[i] = merged[SIZE-1-i];
    }

    free(receivebuf);
    free(merged);
}
