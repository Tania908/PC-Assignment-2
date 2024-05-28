#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//make sure size is of 2^N
const int SIZE = (1<< 2); // 2^18
const int ITERATIONS =4;
void swap(int *p1, int *p2);
int partition(int array[], int low, int high);
void quicksort(int array[], int low, int high);
void generate_random_array(int array[], int size, unsigned int seed);
void display_array(int *array, int size, const char *name);


//MAIN
int main() {
    // int argc, char *argv[]


    printf("Number of elements being generated: %i \n", SIZE*ITERATIONS);
    int merged_size = SIZE* ITERATIONS;
    int *merged_array = malloc(merged_size * sizeof(int));
    unsigned int initial_seed = (unsigned int)time(NULL);


  // Dynamically allocate memory for the arrays
    int **array = malloc(ITERATIONS * sizeof(int *));
    for (int i = 0; i < ITERATIONS; i++) {
        array[i] = malloc(SIZE * sizeof(int));
    }
    //generating array
    for(int i =0; i <ITERATIONS;i++ ){
        generate_random_array(array[i],SIZE, initial_seed+ i);
    }

    // Merge the arrays
    int index = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        for (int j = 0; j < SIZE; j++) {
            merged_array[index++] = array[i][j];
        }
    }
    display_array(merged_array,merged_size,"Generated array");
    // Sorting the array via quick sort
    clock_t start_time = clock();

    quicksort(merged_array, 0, merged_size - 1);

    clock_t finish_time = clock();
    double time = ((double)(finish_time-start_time)) / CLOCKS_PER_SEC;

    //Print time taken for quicksort to sort array
    printf("\nTime taken using quicksort: %.6f seconds\n", time);


    display_array(merged_array,merged_size,"Sorted array");
    // Free dynamically allocated memory
    for (int i = 0; i < ITERATIONS; i++) {
        free(array[i]);
    }
    free(array);
    free(merged_array);

    return 0;
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
void quicksort(int array[], int low, int high) {
    if (low < high) {
        int pi = partition(array, low, high);

        quicksort(array, low, pi - 1);
        quicksort(array, pi + 1, high);
    }
}
void generate_random_array(int array[], int size, unsigned int seed){
    srand(seed);
    for(int i =0 ; i < size;i++){
        array[i] = rand() %101;
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
