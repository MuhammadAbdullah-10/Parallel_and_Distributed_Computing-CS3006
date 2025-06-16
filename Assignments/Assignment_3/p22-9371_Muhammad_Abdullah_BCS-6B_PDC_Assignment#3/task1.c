#include <stdio.h> // For standard input/output functions
#include <mpi.h>   // For MPI functions

int main(int argc, char **argv)
{
    int rank, value;

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the rank (ID) of the current process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // If this is the root process (rank 0), set the value
    if (rank == 0)
    {
        value = 59;
        printf("Process 0: Initial value = %d\n", value);
    }
    else
    {
        // Other processes initialize value to 0
        value = 0;
    }
    // Broadcast the value from process 0 to all processes in MPI_COMM_WORLD
    MPI_Bcast(&value, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // All processes (Including 0) print the received value
        printf("Process %d: Received value = %d\n", rank, value);
    // Clean up the MPI environment
    MPI_Finalize();
    return 0;
}
