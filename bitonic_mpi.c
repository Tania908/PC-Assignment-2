#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
//mpirun -np 4 ./bitonic_mpi


void generate_random_array(int array[], int size, unsigned int seed);
void swap(int *p1, int *p2);
int partition(int array[], int low, int high);
void quicksort(int *array, int low, int high);
void display_array(int *array, int size, const char *name);
void compare_arrays(int *quicksort_array, int *global_array, int size);
void compareLow(int *local_array,int SIZE, int myrank,int j);
void compareHigh(int *local_array,int SIZE, int myrank,int j);

int main(int argc, char *argv[]) {
    //printf("Number of elements being generated: %i \n", SIZE*ITERATIONS);
      if (argc < 2) {
        printf("Usage: %s <size_value>\n", argv[0]);
        return 1;  // Exit the program with an error code
    }
    unsigned int initial_seed = (unsigned int)time(NULL);
    int myrank,processors;
    double qs_start_time,qs_end_time,bs_start_time,bs_end_time;
    int size = 1<<atoi(argv[1]);
    int* global_array = NULL;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &processors);
    int local_size = size/processors;
    int *local_array = (int *)malloc(local_size *sizeof(int));
    int *global_array = (int *)malloc(size * sizeof(int));

    //Ensure that all processors generate the array correctly 
    generate_random_array(local_array,local_size,initial_seed + myrank);

    MPI_Barrier(MPI_COMM_WORLD);
    if (myrank == 0) {
        bs_start_time = MPI_Wtime();
    }
    
    
    quicksort(local_array,0, local_size - 1);
    
    //going through hypercube
    int dimensions = log2(processors);
    for(int i =0; i < dimensions ; i++){
        //bit comparison loop
        for(int j=i; j >=0;j--){
            //Determining positions in hypercube network with ranks
            if(((myrank >> (i+1))%2==0 && (myrank >> j)%2==0)||((myrank >>(i+1))%2 !=0 && (myrank >> j)%2 !=0)){
                //both even rank
                compareLow(local_array,local_size,myrank,j);
            }
            else{
                //both odd rank
                compareHigh(local_array,local_size,myrank,j);
            }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    

    MPI_Gather(local_array, local_size, MPI_INT, global_array, local_size, MPI_INT, 0, MPI_COMM_WORLD);
    if (myrank == 0) {
        bs_end_time = MPI_Wtime()-bs_start_time;
        printf("Time taken to do bitonic sort: %f\n", bs_end_time);

        int *quicksort_array = (int *)malloc(size * sizeof(int));
        int **array = (int **)malloc(processors * sizeof(int));

        for (int i = 0; i < processors; i++) {
            array[i] =(int *)malloc(local_size * sizeof(int));
        }

        for(int i =0; i <processors;i++){
            generate_random_array(array[i],local_size, initial_seed+ i);
        }

        int index = 0;

        for (int i = 0; i <processors ; i++) {
            for (int j = 0; j < local_size; j++) {
                quicksort_array[index++] = array[i][j];
            }
        }

        qs_start_time = MPI_Wtime();
        quicksort(quicksort_array, 0, size - 1);
        qs_end_time= MPI_Wtime()-qs_start_time;

        printf("Time taken to do quicksort: %f \n", qs_end_time);
        display_array(quicksort_array,size,"Quicksort sorted array");
        
        for (int i = 0; i < processors; i++) {
            free(array[i]);
            }
        free(array);

        display_array(global_array,size,"MPI sorted array");
        compare_arrays(quicksort_array, global_array, size);
        free(quicksort_array);
         printf("The speedup achieved is: %f\n", qs_end_time/bs_end_time);
    }



    // Free dynamically allocated memory for the local array
    free(local_array);
    MPI_Finalize();
    return 0;
}

void generate_random_array(int *array, int size, unsigned int seed){
    srand(seed);
    for(int i =0 ; i < size;i++){
        array[i] = rand() % 1800;
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
void compare_arrays(int *quicksort_array, int *global_array, int size) {
    int is_correct = 1;
    for (int i = 0; i < size; i++) {
        if (quicksort_array[i] != global_array[i]) {
            is_correct = 0;
            break;
        }
    }
    if (is_correct) {
        printf("The bitonic sort result matches the quicksort result.\n");
    } else {
        printf("The bitonic sort result does not match the quicksort result.\n");
    }
}
