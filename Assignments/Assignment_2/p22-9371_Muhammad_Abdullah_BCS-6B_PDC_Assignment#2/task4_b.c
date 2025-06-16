#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 4) {
        if (rank == 0) printf("Error: At least 4 processes required.\n");
        MPI_Finalize();
        return 1;
    }

    const int M = 3; // Number of cycles
    int counter_cw = 0; // Clockwise counter
    int counter_ccw = 0; // Counterclockwise counter
    int terminate = 0; // Termination flag
    MPI_Status status[2];
    MPI_Request requests[2];

    // Define neighbors
    int next = (rank + 1) % size; // Clockwise
    int prev = (rank - 1 + size) % size; // Counterclockwise

    if (rank == 0) {
        // Start the ring in both directions
        counter_cw = 0;
        counter_ccw = 0;
        printf("Process 0 starting with clockwise counter: %d, counterclockwise counter: %d\n", counter_cw, counter_ccw);
        MPI_Send(&counter_cw, 1, MPI_INT, next, 1, MPI_COMM_WORLD); // Clockwise
        MPI_Send(&counter_ccw, 1, MPI_INT, prev, 2, MPI_COMM_WORLD); // Counterclockwise
    }

    int cycles_completed = 0;
    while (!terminate) {
        // Non-blocking receives for both directions
        MPI_Irecv(&counter_cw, 1, MPI_INT, prev, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[0]);
        MPI_Irecv(&counter_ccw, 1, MPI_INT, next, MPI_ANY_TAG, MPI_COMM_WORLD, &requests[1]);
        MPI_Waitall(2, requests, status);

        // Check for termination from either direction
        if (status[0].MPI_TAG == 0 || status[1].MPI_TAG == 0) {
            terminate = 1;
        } else {
            // Increment counters
            counter_cw++;
            counter_ccw++;

            if (rank == 0) {
                cycles_completed++;
                printf("Process 0 cycle %d: clockwise counter = %d, counterclockwise counter = %d\n", cycles_completed, counter_cw, counter_ccw);
                if (cycles_completed >= M) {
                    printf("Process 0 initiating termination after %d cycles\n", M);
                    terminate = 1;
                }
            }
        }

        // Send updated counters or termination signal
        if (terminate) {
            MPI_Send(&counter_cw, 1, MPI_INT, next, 0, MPI_COMM_WORLD); // Terminate clockwise
            MPI_Send(&counter_ccw, 1, MPI_INT, prev, 0, MPI_COMM_WORLD); // Terminate counterclockwise
        } else {
            MPI_Send(&counter_cw, 1, MPI_INT, next, 1, MPI_COMM_WORLD); // Clockwise
            MPI_Send(&counter_ccw, 1, MPI_INT, prev, 2, MPI_COMM_WORLD); // Counterclockwise
        }
    }

    MPI_Finalize();
    return 0;
}