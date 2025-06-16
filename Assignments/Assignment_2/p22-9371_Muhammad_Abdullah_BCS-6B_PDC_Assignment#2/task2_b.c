#include <stdio.h>
#include <mpi.h>

#define MAX_ARRAY_SIZE 16
#define MAX_SEGMENT_SIZE 16 // Maximum segment size for workers

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size > 16) {
        if (rank == 0) printf("Error: Maximum 16 processes allowed.\n");
        MPI_Finalize();
        return 1;
    }

    int array_size = MAX_ARRAY_SIZE;
    int array[MAX_ARRAY_SIZE];
    int result[MAX_ARRAY_SIZE];
    int segment[MAX_SEGMENT_SIZE];
    int segment_size = array_size / (size - 1); // Workers = size - 1 (master excluded)
    int remainder = array_size % (size - 1);

    if (rank == 0) { // Master
        // Initialize array
        for (int i = 0; i < array_size; i++) {
            array[i] = i + 1; // Example: 1 to 16
        }

        // Array to store send requests
        MPI_Request requests[2 * (size - 1)]; // 2 requests per worker (size + data)
        int req_index = 0;

        // Distribute segments using non-blocking sends
        int offset = 0;
        for (int i = 1; i < size; i++) {
            int send_size = segment_size;
            if (i <= remainder) {
                send_size = send_size + 1;
            }
            // Non-blocking send for segment size
            MPI_Isend(&send_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[req_index++]);
            // Non-blocking send for segment data
            MPI_Isend(&array[offset], send_size, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[req_index++]);
            offset = offset + send_size;
        }

        // Omitted MPI_Waitall for sends

        // Array to store receive requests
        req_index = 0;
        offset = 0;
        for (int i = 1; i < size; i++) {
            int recv_size = segment_size;
            if (i <= remainder) {
                recv_size = recv_size + 1;
            }
            // Non-blocking receive for squared segment
            MPI_Irecv(&result[offset], recv_size, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[req_index++]);
            offset = offset + recv_size;
        }

        // Omitted MPI_Waitall for receives

        // Print final array (likely before receives complete)
        printf("Final squared array: ");
        for (int i = 0; i < array_size; i++) {
            printf("%d ", result[i]);
        }
        printf("\n");
    } else { // Workers
        MPI_Request requests[2]; // For receiving size and data
        int recv_size;

        // Non-blocking receive for segment size
        MPI_Irecv(&recv_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &requests[0]);
        // Non-blocking receive for segment data
        MPI_Irecv(segment, MAX_SEGMENT_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, &requests[1]);

        // Omitted MPI_Waitall for receives

        // Compute squares (using potentially uninitialized recv_size and segment)
        for (int i = 0; i < recv_size; i++) {
            segment[i] = segment[i] * segment[i];
        }

        // Non-blocking send for squared segment
        MPI_Request send_request;
        MPI_Isend(segment, recv_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &send_request);

        // Omitted MPI_Wait for send
    }

    MPI_Finalize();
    return 0;
}