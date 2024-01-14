#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char** argv) {
    int my_rank, size, n;
    float *X = NULL, max, m = 0, var = 0;
    int i, localn, root = 0;
    float *localX, localmax, localsum = 0;
    float localvar = 0;
    float *Delta = NULL;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (my_rank == root) {
        printf("Enter the length of the vector: ");
        scanf("%d", &n);
        X = (float *)malloc(n * sizeof(float));
        printf("Enter the numbers of the vector:\n");
        for (i = 0; i < n; i++) {
            scanf("%f", &X[i]);
        }
    }
    
    // Broadcast the value of n to all processes using MPI_Send and MPI_Receive
    if (my_rank == root) {
        for (i = 1; i < size; i++) {
            MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&n, 1, MPI_INT, root, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Calculate localn
    localn = (my_rank != size - 1) ? n / size : n - (n / size) * (size - 1);
    localX = (float *)malloc(localn * sizeof(float));

    // Scatter X to all processes using MPI_Send and MPI_Receive
    if (my_rank == root) {
        int offset = localn;
        for (i = 1; i < size; i++) {
            int sendn = (i != size - 1) ? n / size : n - (n / size) * (size - 1);
            MPI_Send(X + offset, sendn, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            offset += sendn;
        }
        memcpy(localX, X, localn * sizeof(float));
    } else {
        MPI_Recv(localX, localn, MPI_FLOAT, root, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Perform local computations
    localmax = localX[0];
    localsum = 0;
    for (i = 0; i < localn; i++) {
        if (localX[i] > localmax) localmax = localX[i];
        localsum += localX[i];
    }

    // Calculate local variance
    for (i = 0; i < localn; i++) {
        localvar += (localX[i] - m) * (localX[i] - m);
    }

    // Calculate local Δ vector
    Delta = (float *)malloc(localn * sizeof(float));
    for (i = 0; i < localn; i++) {
        Delta[i] = (localX[i] - m) * (localX[i] - m);
    }

    // Gather local max, local var, and local Δ vectors at root using MPI_Send and MPI_Receive
    if (my_rank == root) {
        max = localmax;
        var = localvar;
        for (i = 1; i < size; i++) {
            float recv_max, recv_var;
            MPI_Recv(&recv_max, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&recv_var, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (recv_max > max) max = recv_max;
            var += recv_var;
        }
        for (i = 1; i < size; i++) {
            float *recv_Delta = (float *)malloc(localn * sizeof(float));
            MPI_Recv(recv_Delta, localn, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Merge received Delta vectors into the main Delta vector
            memcpy(Delta + i * localn, recv_Delta, localn * sizeof(float));
            free(recv_Delta);
        }
    } else {
        MPI_Send(&localmax, 1, MPI_FLOAT, root, 0, MPI_COMM_WORLD);
        MPI_Send(&localvar, 1, MPI_FLOAT, root, 0, MPI_COMM_WORLD);
        MPI_Send(Delta, localn, MPI_FLOAT, root, 0, MPI_COMM_WORLD);
    }

    // Finalize MPI
    MPI_Finalize();

    if (my_rank == root) {
        m = localsum / n;
        var /= n;
        printf("Max: %f, Average: %f, Variance: %f\n", max, m, var);
        printf("Vector Delta: ");
        for (i = 0; i < n; i++) {
            printf("%f ", Delta[i]);
        }
        printf("\n");
    }

    // Free memory
    free(localX);
    free(Delta);
    if (my_rank == root) {
        free(X);
    }

    return 0;
}