#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size;
    const int original_size = 16; // Fixed array size
    int chunk_size; // Number of elements per process (for divisible portion)
    int divisible_size; // Size of the divisible portion
    int remainder; // Remaining elements
    int *full_array = NULL; // Array in process 0
    int *local_chunk = NULL; // Each process's chunk
    int *final_array = NULL; // To store gathered results

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Compute the divisible portion of the array
    chunk_size = original_size / size; // Base chunk size per process
    divisible_size = chunk_size * size; // Largest size divisible by number of processes
    remainder = original_size % size; // Remaining elements to be handled by process 0

    if (chunk_size == 0) {
        if (rank == 0) {
            printf("Error: Too many processes (%d) for array size %d. Each process must get at least 1 element.\n", size, original_size);
        }
        MPI_Finalize();
        return 1;
    }

    // Allocate arrays
    local_chunk = (int*)malloc(chunk_size * sizeof(int));
    if (rank == 0) {
        full_array = (int*)malloc(original_size * sizeof(int));
        final_array = (int*)malloc(original_size * sizeof(int));

        // Initialize the array with values 1 to 16
        for (int i = 0; i < original_size; i++) {
            full_array[i] = i + 1;
        }
        printf("Process 0: Initial array (size %d): ", original_size);
        for (int i = 0; i < original_size; i++) {
            printf("%d ", full_array[i]);
        }
        printf("\n");
    }

    // Scatter the divisible portion of the array to all processes
    MPI_Scatter(full_array, chunk_size, MPI_INT, local_chunk, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process multiplies its chunk by 2
    for (int i = 0; i < chunk_size; i++) {
        local_chunk[i] *= 2;
    }
    printf("Process %d: Local chunk after multiplication: ", rank);
    for (int i = 0; i < chunk_size; i++) {
        printf("%d ", local_chunk[i]);
    }
    printf("\n");

    // Gather the modified chunks back to process 0
    MPI_Gather(local_chunk, chunk_size, MPI_INT, final_array, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Process 0 handles the remaining elements (if any)
    if (rank == 0 && remainder > 0) {
        printf("Process 0: Handling %d remaining elements: ", remainder);
        for (int i = divisible_size; i < original_size; i++) {
            final_array[i] = full_array[i] * 2;
            printf("%d ", final_array[i]);
        }
        printf("\n");
    }

    // Process 0 prints the final array
    if (rank == 0) {
        printf("Process 0: Final array after gathering: ");
        for (int i = 0; i < original_size; i++) {
            printf("%d ", final_array[i]);
        }
        printf("\n");
    }

    // Clean up
    free(local_chunk);
    if (rank == 0) {
        free(full_array);
        free(final_array);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}