#include <stdio.h> // For standard I/O functions
#include <mpi.h>   // For MPI functions

int main(int argc, char **argv)
{
    int rank, size;
    int full_array[16];  // Array in process 0 holding 16 integers
    int local_chunk[4];  // Each process will receive 4 integers
    int final_array[16]; // Array to gather results back in process 0

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the rank (ID) of the current process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Ensure the program runs with exactly 4 processes
    if (size != 4)
    {
        if (rank == 0)
        {
            printf("This program requires exactly 4 processes.\n");
        }
        MPI_Finalize(); // Exit MPI if the number of processes is incorrect
        return 1;
    }

    // Process 0 initializes the array with values 1 to 16
    if (rank == 0)
    {
        for (int i = 0; i < 16; i++)
        {
            full_array[i] = i + 1;
        }

        // Print the initialized array
        printf("Process 0: Initial array: ");
        for (int i = 0; i < 16; i++)
        {
            printf("%d ", full_array[i]);
        }
        printf("\n");
    }

    // Scatter the full_array into chunks of 4 integers to each process
    MPI_Scatter(full_array,      // Send buffer (only used by root)
                4,               // Number of elements sent to each process
                MPI_INT,         // Data type of elements
                local_chunk,     // Receive buffer for each process
                4,               // Number of elements received by each process
                MPI_INT,         // Data type of elements
                0,               // Root process (source of scatter)
                MPI_COMM_WORLD); // Communicator

    // Each process multiplies its 4-element chunk by 2
    for (int i = 0; i < 4; i++)
    {
        local_chunk[i] *= 2;
    }

    // Each process prints its modified chunk
    printf("Process %d: Local chunk after multiplication: %d %d %d %d\n",
           rank, local_chunk[0], local_chunk[1], local_chunk[2], local_chunk[3]);

    // Gather the modified chunks back to process 0
    MPI_Gather(local_chunk,     // Send buffer
               4,               // Number of elements to send
               MPI_INT,         // Data type of elements
               final_array,     // Receive buffer (only used by root)
               4,               // Number of elements received from each process
               MPI_INT,         // Data type of elements
               0,               // Root process (destination of gather)
               MPI_COMM_WORLD); // Communicator

    // Process 0 prints the final gathered array
    if (rank == 0)
    {
        printf("Process 0: Final array after gathering: ");
        for (int i = 0; i < 16; i++)
        {
            printf("%d ", final_array[i]);
        }
        printf("\n");
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
