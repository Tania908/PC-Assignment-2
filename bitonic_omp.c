#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <math.h>
#include <string.h>

const int ASCENDING =1;
const int DESCENDING =0;

//Initializing methods
void generate_random_array(int [], int size, unsigned int);
void parallelBitonicSort(int *a,int size, int tthreads);
void bitonicMerge(int *a, int lo, int cnt, int dir);
void swap(int *p1, int *p2);
int partition(int array[], int low, int high);
void quicksort(int array[], int low, int high);
int compare_arrays(int *arr1, int *arr2, int size);
void display_array(int *array, int size, const char *name);


int main(int argc, char *argv[]){

  // Check if an argument is provided
    if (argc < 3) {
        printf("Usage: %s <size_value> <process_value>\n", argv[0]);
        return 1;  // Exit the program with an error code
    }

    int power  = atoi(argv[1]);
    int size = 1<< power;
    int tthreads = atoi(argv[2]);
  

    printf("Number of elements being generated: %i \n", size);
     
     // Dynamically allocate memory for the arrays
    int *array = malloc(size * sizeof(int *));
    
    int *quicksort_array = malloc(size * sizeof(int));
    
    int *bitonic_array = malloc(size * sizeof(int));

    unsigned int initial_seed = (unsigned int)time(NULL);
    double qs_start_time,qs_end_time,bs_start_time,bs_end_time;


    generate_random_array(array,size, initial_seed);

    // Duplicate merged_array for quicksort and bitonic sort
    memcpy(quicksort_array, array, size * sizeof(int));
    memcpy(bitonic_array, array, size * sizeof(int));

    // Display unsorted arrays depending on given argument

    // display_array(quicksort_array, size, "Quicksort Array");
    // display_array(bitonic_array, size, "Bitonic Array");

    //Quicksort
    qs_start_time = omp_get_wtime();
    quicksort(quicksort_array, 0, size - 1);
    qs_end_time= omp_get_wtime()-qs_start_time;
    printf("Time taken to do quicksort: %f \n", qs_end_time);

    //Sorting with bitonic sort
    bs_start_time = omp_get_wtime();
    parallelBitonicSort(bitonic_array,size,tthreads);
    bs_end_time= omp_get_wtime()-bs_start_time;
    printf("Time taken to bitonic sort: %f \n", bs_end_time);
    
     // Display sorted arrays
     
    // display_array(quicksort_array, size, "Quicksort Sorted Array");
    // display_array(bitonic_array, size, "Bitonic Sorted Array");

   // Validate the sorted arrays
    if (compare_arrays(quicksort_array, bitonic_array, size)) {
        printf("The arrays are sorted identically.\n");
    } else {
        printf("The arrays are not sorted identically.\n");
    }

    printf("The speedup achieved is: %f\n", qs_end_time/bs_end_time);

 // Free dynamically allocated memory
    free(array);
    free(quicksort_array);
    free(bitonic_array);


  return 0;
}

//Methods

void generate_random_array(int array[], int size, unsigned int seed){
    srand(seed);
    for(int i =0 ; i < size;i++){
        array[i] = rand() % 1800;
    }
}

void parallelBitonicSort(int *a,int size,int tthreads)
{

    for (int k = 2; k <= size; k = 2 * k) {
        for (int j = k >> 1; j > 0; j = j >> 1) {
            #pragma omp parallel for  num_threads(tthreads)
            for (int i = 0; i < size; i++) {
               int ij = i ^ j;
                if (ij > i) {
                    if ((i & k) == 0 && a[i] > a[ij])
                        swap(&a[i], &a[ij]);
                    if ((i & k) != 0 && a[i] < a[ij])
                        swap(&a[i], &a[ij]);
                }

            }
        }
    }
    bitonicMerge(a,0,size,ASCENDING);
}

void bitonicMerge(int *a, int lo, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = lo; i < lo + k; i++) {
            if ((dir == ASCENDING && a[i] > a[i + k]) ||
                (dir == DESCENDING && a[i] < a[i + k])) {
                swap(&a[i], &a[i + k]);
            }
        }
        bitonicMerge(a, lo, k, dir);
        bitonicMerge(a, lo + k, k, dir);
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
void quicksort(int array[], int low, int high) {
    if (low >= high) return;

    if (low < high) {
        int pi = partition(array, low, high);

        quicksort(array, low, pi - 1);
        quicksort(array, pi + 1, high);
    }
}

// Compare arrays
int compare_arrays(int *arr1, int *arr2, int size) {
    for (int i = 0; i < size; i++) {
        if (arr1[i] != arr2[i]) {
            return 0; // Arrays are not identical
        }
    }
    return 1; // Arrays are identical
}

// Function to display an array
void display_array(int *array, int size, const char *name) {
    printf("%s:\n", name);
    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("\n\n");
}











