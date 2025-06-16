#include <stdio.h>
#include <mpi.h>

#define MAX_ARRAY_SIZE 16   // Total number of elements in the array
#define MAX_SEGMENT_SIZE 16 // Maximum size of the segment each worker can handle

int main(int argc, char *argv[])
{
    int rank, size;

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the rank (ID) of the current process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Ensure no more than 16 processes are used
    if (size > 16)
    {
        if (rank == 0)
            printf("Error: Maximum 16 processes allowed.\n");
        MPI_Finalize();
        return 1;
    }

    int array_size = MAX_ARRAY_SIZE;            // Total elements in the main array
    int array[MAX_ARRAY_SIZE];                  // Original array (used only by master)
    int result[MAX_ARRAY_SIZE];                 // Final result array to store squares
    int segment[MAX_SEGMENT_SIZE];              // Buffer for each worker's segment
    int segment_size = array_size / (size - 1); // Base segment size for each worker
    int remainder = array_size % (size - 1);    // Extra elements to distribute evenly

    double start_time, end_time;
    // Synchronize all processes before starting the computation
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    if (rank == 0)
    { // Master process
        // Initialize the array with values 1 to 16
        for (int i = 0; i < array_size; i++)
        {
            array[i] = i + 1;
        }

        // Distribute segments of the array to worker processes
        int offset = 0;
        for (int i = 1; i < size; i++)
        {
            int send_size = segment_size;

            // Distribute remaining elements evenly to the first few workers
            if (i <= remainder)
            {
                send_size += 1;
            }

            // Send the size of the segment
            MPI_Send(&send_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

            // Send the actual segment of the array
            MPI_Send(&array[offset], send_size, MPI_INT, i, 0, MPI_COMM_WORLD);

            offset += send_size;
        }

        // Collect results from workers
        offset = 0;
        for (int i = 1; i < size; i++)
        {
            int recv_size = segment_size;
            if (i <= remainder)
            {
                recv_size += 1;
            }

            // Receive the squared segment from each worker
            MPI_Recv(&result[offset], recv_size, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            offset += recv_size;
        }

        // Print the final squared array
        printf("Final squared array: ");
        for (int i = 0; i < array_size; i++)
        {
            printf("%d ", result[i]);
        }
        printf("\n");
    }
    else
    { // Worker processes
        int recv_size;

        // Receive the size of the segment from the master
        MPI_Recv(&recv_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Receive the actual segment from the master
        MPI_Recv(segment, recv_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Compute the square of each element in the segment
        for (int i = 0; i < recv_size; i++)
        {
            segment[i] = segment[i] * segment[i];
        }

        // Send the squared segment back to the master
        MPI_Send(segment, recv_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    // Synchronize all processes after computation
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    if (rank == 0)
    {
        printf("Blocking communication time: %f seconds\n", end_time - start_time);
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}