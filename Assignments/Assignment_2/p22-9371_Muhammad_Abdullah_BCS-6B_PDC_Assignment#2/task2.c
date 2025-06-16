#include <stdio.h>
#include <mpi.h>

#define MAX_ARRAY_SIZE 16   // Total number of elements in the array
#define MAX_SEGMENT_SIZE 16 // Maximum size a worker can receive

int main(int argc, char *argv[])
{
    int rank, size;

    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    // Get the rank (ID) of this process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check if number of processes exceeds the limit
    if (size > 16)
    {
        if (rank == 0)
            printf("Error: Maximum 16 processes allowed.\n");
        MPI_Finalize();
        return 1;
    }

    int array_size = MAX_ARRAY_SIZE;            // Number of elements in the array
    int array[MAX_ARRAY_SIZE];                  // Array to be processed (only used by master)
    int result[MAX_ARRAY_SIZE];                 // Array to store squared results
    int segment[MAX_SEGMENT_SIZE];              // Buffer for segment each worker will process
    int segment_size = array_size / (size - 1); // Basic segment size (excluding master)
    int remainder = array_size % (size - 1);    // Remainder to be distributed among first few workers

    if (rank == 0)
    { // Master process
        // Initialize the array with values from 1 to 16
        for (int i = 0; i < array_size; i++)
        {
            array[i] = i + 1;
        }

        // Array to hold MPI request objects for sends (size + data) to each worker
        MPI_Request requests[2 * (size - 1)];
        int req_index = 0;
        int offset = 0; // Keeps track of where we are in the array

        // Distribute array segments using non-blocking sends
        for (int i = 1; i < size; i++)
        {
            int send_size = segment_size;

            // Distribute remainder elements to first few workers
            if (i <= remainder)
            {
                send_size += 1;
            }

            // Non-blocking send of segment size to worker i
            MPI_Isend(&send_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[req_index++]);

            // Non-blocking send of actual segment data to worker i
            MPI_Isend(&array[offset], send_size, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[req_index++]);

            offset += send_size;
        }

        // Wait for all non-blocking sends to complete
        MPI_Waitall(req_index, requests, MPI_STATUSES_IGNORE);

        // Reset index for receiving results
        req_index = 0;
        offset = 0;

        // Reuse the same requests array to receive results from workers
        for (int i = 1; i < size; i++)
        {
            int recv_size = segment_size;
            if (i <= remainder)
            {
                recv_size += 1;
            }

            // Non-blocking receive from worker i
            MPI_Irecv(&result[offset], recv_size, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[req_index++]);

            offset += recv_size;
        }

        // Wait for all results to be received
        MPI_Waitall(req_index, requests, MPI_STATUSES_IGNORE);

        // Print the final array containing squared values
        printf("Final squared array: ");
        for (int i = 0; i < array_size; i++)
        {
            printf("%d ", result[i]);
        }
        printf("\n");
    }
    else
    {                            // Worker processes
        MPI_Request requests[2]; // One for size, one for data
        int recv_size;

        // Non-blocking receive of segment size from master
        MPI_Irecv(&recv_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &requests[0]);

        // Non-blocking receive of segment data from master
        // Note: we allocate full buffer but only process recv_size
        MPI_Irecv(segment, MAX_SEGMENT_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, &requests[1]);

        // Wait until both receives are done
        MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);

        // Square each element in the received segment
        for (int i = 0; i < recv_size; i++)
        {
            segment[i] = segment[i] * segment[i];
        }

        // Non-blocking send of the computed segment back to master
        MPI_Request send_request;
        MPI_Isend(segment, recv_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &send_request);

        // Ensure send completes before process exits
        MPI_Wait(&send_request, MPI_STATUS_IGNORE);
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
