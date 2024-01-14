#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int main(int argc, char *argv[]) {
    int rank, size, N, i, j, isStrictlyDominant = 1;
    double *A, *B, m, minVal = INFINITY;
    int minPos = -1;

    // Αρχικοποίηση του MPI
    MPI_Init(&argc, &argv);

    // Λήψη του rank και του size
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Έλεγχος αν έχει δοθεί η παράμετρος N
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Μετατροπή της παραμέτρου σε ακέραιο
    N = atoi(argv[1]); 

    // Δέσμευση μνήμης για τους πίνακες A και B
    A = (double *)malloc(N * N * sizeof(double));
    B = (double *)malloc(N * N * sizeof(double));

    // Έλεγχος αν η δέσμευση μνήμης ήταν επιτυχής
    if (A == NULL || B == NULL) {
        printf("Memory allocation failed for array A or B\n");
        free(A);
        MPI_Finalize();
        return 1;
    }

    // Ο πίνακας A συμπληρώνεται από τον χρήστη
    if (rank == 0) {
        printf("Please enter the elements of the array\n");
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                scanf("%lf", &A[i * N + j]);
            }
        }
    }

    // Εκπομπή του πίνακα A σε όλες τις διεργασίες
    MPI_Bcast(A, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Έλεγχος αν ο πίνακας A είναι strictly dominant
    // Κάθε διεργασία ελέγχει ένα υποσύνολο των γραμμών του πίνακα
    for (i = rank; i < N; i += size) {
        double rowSum = 0.0, diagVal = fabs(A[i * N + i]);
        for (j = 0; j < N; j++) {
            if (i != j) {
                rowSum += fabs(A[i * N + j]);
            }
        }
        if (diagVal <= rowSum) {
            isStrictlyDominant = 0;
            break;
        }
    }

    // Συγχρονισμός των διεργασιών και έλεγχος αν ο πίνακας είναι strictly dominant
    int globalDominant;
    MPI_Allreduce(&isStrictlyDominant, &globalDominant, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

    // Εκτύπωση του αποτελέσματος
    if (rank == 0) {
        if (globalDominant) {
            printf("Yes\n");
        } else {
            printf("No\n");
            MPI_Finalize();
            return 0;
        }
    }

    // Υπολογισμός του μέγιστου απόλυτου στοιχείου της διαγωνίου του πίνακα A
    m = fabs(A[rank * N + rank]);
    for (i = rank + size; i < N; i += size) {
        double temp = fabs(A[i * N + i]);
        if (temp > m) {
            m = temp;
        }
    }

    // Συγχρονισμός των διεργασιών και εύρεση του μέγιστου απόλυτου στοιχείου
    double globalMax;
    MPI_Allreduce(&m, &globalMax, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

    // Δημιουργία του πίνακα B
    for (i = rank; i < N; i += size) {
        for (j = 0; j < N; j++) {
            int index = i * N + j;
            if (i == j) {
                B[index] = globalMax;
            } else {
                B[index] = globalMax - fabs(A[index]);
            }
        }
    }

    // Εκτύπωση του πίνακα B
    if (rank == 0) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                printf("%lf ", B[i * N + j]);
            }
            printf("\n");
        }
    }

    // Εύρεση του ελάχιστου στοιχείου του πίνακα B και της θέσης του
    if (rank * N < N * N) {
    minVal = B[rank * N * size];
    minPos = rank * N * size;
} else {
    minVal = INFINITY;
    minPos = 0;
}

for (i = rank; i < N; i += size) {
    for (j = 0; j < N; j++) {
        int index = i * N + j;
        if (B[index] < minVal) {
            minVal = B[index];
            minPos = index;
        }
    }
}

    // Συγχρονισμός των διεργασιών και εύρεση του ελάχιστου στοιχείου και της θέσης του
    double globalMin;
    int globalPos;
    MPI_Reduce(&minVal, &globalMin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&minPos, &globalPos, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    // Εκτύπωση του ελάχιστου στοιχείου και της θέσης του
    if (rank == 0) {
        printf("Minimum value: %lf at position (%d, %d)\n", globalMin, globalPos / N, globalPos % N);
    }

    // Απελευθέρωση της μνήμης και τερματισμός του MPI
    free(A);
    free(B);
    MPI_Finalize();
    return 0;
}