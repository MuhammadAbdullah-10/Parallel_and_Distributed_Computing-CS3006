#include <stdio.h>
#include <mpi.h>

#define ARRAY_SIZE 8 // Number of elements in each array

int main(int argc, char *argv[])
{
    int rank, size;

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the rank (ID) of the current process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Ensure there are at least 4 processes to run this program
    if (size < 4)
    {
        if (rank == 0)
            printf("Error: At least 4 processes required.\n");
        MPI_Finalize();
        return 1;
    }

    int array1[ARRAY_SIZE];       // First array (initialized by Process 0)
    int array2[ARRAY_SIZE];       // Second array (initialized by Process 0)
    int result1[ARRAY_SIZE];      // Stores squared values from array1 (computed by Process 1)
    int result2[ARRAY_SIZE];      // Stores squared values from array2 (computed by Process 2)
    int final_result[ARRAY_SIZE]; // Final aggregated array (computed by Process 3)

    if (rank == 0)
    {
        // -------------------- Process 0: Data Distributor --------------------
        // Initialize both arrays
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            array1[i] = i + 1;       // array1: 1 to 8
            array2[i] = (i + 1) * 2; // array2: 2 to 16
        }

        // Send array1 to Process 1 using tag 10
        MPI_Send(array1, ARRAY_SIZE, MPI_INT, 1, 10, MPI_COMM_WORLD);

        // Send array2 to Process 2 using tag 20
        MPI_Send(array2, ARRAY_SIZE, MPI_INT, 2, 20, MPI_COMM_WORLD);
    }
    else if (rank == 1)
    {
        // -------------------- Process 1: Compute squares of array1 --------------------
        MPI_Status status;

        // Receive array1 from Process 0 with tag 10
        MPI_Recv(array1, ARRAY_SIZE, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);

        // Compute square of each element in array1
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            result1[i] = array1[i] * array1[i];
        }

        // Send result1 to Process 3 using tag 30
        MPI_Send(result1, ARRAY_SIZE, MPI_INT, 3, 30, MPI_COMM_WORLD);
    }
    else if (rank == 2)
    {
        // -------------------- Process 2: Compute squares of array2 --------------------
        MPI_Status status;

        // Receive array2 from Process 0 with tag 20
        MPI_Recv(array2, ARRAY_SIZE, MPI_INT, 0, 20, MPI_COMM_WORLD, &status);

        // Compute square of each element in array2
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            result2[i] = array2[i] * array2[i];
        }

        // Send result2 to Process 3 using tag 40
        MPI_Send(result2, ARRAY_SIZE, MPI_INT, 3, 40, MPI_COMM_WORLD);
    }
    else if (rank == 3)
    {
        // -------------------- Process 3: Aggregate results --------------------
        MPI_Status status;

        // Dynamically receive two arrays from Processes 1 and 2
        for (int i = 0; i < 2; i++)
        {
            // Determine which result buffer to receive into
            MPI_Recv(i == 0 ? result1 : result2, ARRAY_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            // Print info about the received message
            int source = status.MPI_SOURCE;
            int tag = status.MPI_TAG;
            printf("Process 3 received message from Process %d with tag %d\n", source, tag);
        }

        // Aggregate: sum corresponding elements of result1 and result2
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            final_result[i] = result1[i] + result2[i];
        }

        // Display the final aggregated array
        printf("Final aggregated array: ");
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            printf("%d ", final_result[i]);
        }
        printf("\n");
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
