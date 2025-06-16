#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int rank, size;

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the rank (ID) of the current process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Ensure there are at least 4 processes for the ring to function meaningfully
    if (size < 4)
    {
        if (rank == 0)
            printf("Error: At least 4 processes required.\n");
        MPI_Finalize();
        return 1;
    }

    const int M = 3;   // Number of complete cycles to perform around the ring
    int counter = 0;   // Shared counter passed around the ring
    int terminate = 0; // Flag to indicate when to terminate
    MPI_Status status; // MPI status object to get message metadata

    // Determine neighbors in the ring topology
    int next = (rank + 1) % size;        // Next process in ring
    int prev = (rank - 1 + size) % size; // Previous process in ring (wrap around)

    if (rank == 0)
    {
        // Only Process 0 starts the ring communication
        counter = 0;
        printf("Process 0 starting with counter: %d\n", counter);
        // Send initial counter to next process with tag 1 (active message)
        MPI_Send(&counter, 1, MPI_INT, next, 1, MPI_COMM_WORLD);
    }

    // Loop until termination flag is set
    while (!terminate)
    {
        // Receive message from previous process (could be counter or termination signal)
        MPI_Recv(&counter, 1, MPI_INT, prev, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == 0)
        {
            // If tag is 0, this is a termination signal
            terminate = 1;

            // Forward termination signal to next process
            MPI_Send(&counter, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
            break;
        }

        // If not termination, increment counter
        counter++;

        if (rank == 0)
        {
            // Process 0 checks if the desired number of cycles has been completed
            printf("Process 0 received counter: %d\n", counter);

            if (counter >= M * size)
            {
                // If M full cycles completed, initiate termination
                printf("Process 0 initiating termination after %d cycles\n", M);
                terminate = 1;

                // Send termination message to next process
                MPI_Send(&counter, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
                break;
            }
        }

        // Forward the updated counter to the next process
        MPI_Send(&counter, 1, MPI_INT, next, 1, MPI_COMM_WORLD);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}
