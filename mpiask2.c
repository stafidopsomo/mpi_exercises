#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int main(int argc, char *argv[]) {
    int rank, size, N, i, j, isStrictlyDominant = 1;
    double *A, *B, m, n, minVal = INFINITY;
    int minPos = -1;

    // Αρχικοποίηση του MPI
    MPI_Init(&argc, &argv);

    // Λήψη του rank και του size της τρέχουσας διεργασίας
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Έλεγχος για την ύπαρξη της παραμέτρου N
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Μετατροπή της παραμέτρου N από string σε ακέραιο
    N = atoi(argv[1]); 

    // Δέσμευση μνήμης για τους πίνακες A και B
    A = (double *)malloc(N * N * sizeof(double));
    B = (double *)malloc(N * N * sizeof(double));

    // Έλεγχος για επιτυχή δέσμευση μνήμης
    if (A == NULL || B == NULL) {
        printf("Memory allocation failed for array A or B\n");
        free(A);
        MPI_Finalize();
        return 1;
    }

    // Εισαγωγή στοιχείων στον πίνακα A από τον χρήστη (μόνο από την rank 0 διεργασία)
    if (rank == 0) {
        printf("Please enter the elements of the array\n");
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                scanf("%lf", &A[i * N + j]);
            }
        }
    }

    // Εκπομπή των στοιχείων του πίνακα A σε όλες τις διεργασίες
    MPI_Bcast(A, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Έλεγχος αυστηρής κυριαρχίας για τον πίνακα A
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

    // Συγχρονισμός διεργασιών και έλεγχος πίνακα για αν είναι αυστηρά διαγώνια δεσπόζων
    int globalDominant;
    MPI_Allreduce(&isStrictlyDominant, &globalDominant, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

    // Εκτύπωση αποτελέσματος για το αν ο πίνακας ειναι αυστηρά διαγώνια δεσπόζων
    if (rank == 0) {
        if (globalDominant) {
            printf("Yes, the matrix is strictly dominant\n");
        } else {
            printf("No, the matrix is not strictly dominant\n");
            MPI_Finalize();
            return 0;
        }
    }

    // Υπολογισμός μέγιστης απόλυτης τιμής στη διαγώνιο του πίνακα A
    m = fabs(A[rank * N + rank]);
    for (i = rank + size; i < N; i += size) {
        double temp = fabs(A[i * N + i]);
        if (temp > m) {
            m = temp;
        }
    }

    // Συγχρονισμός διεργασιών και εύρεση της μέγιστης απόλυτης τιμής
    double globalMax;
    MPI_Allreduce(&m, &globalMax, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

    // Εκτύπωση της μέγιστης απόλυτης τιμής της διαγωνίου
    if (rank == 0) {
        printf("The max absolute value of the diagonal is %lf\n", globalMax);
    }

    // Δημιουργία του πίνακα B βάσει της μέγιστης τιμής
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

    // Εκτύπωση του πίνακα B (μόνο από την rank 0 διεργασία)
    if (rank == 0){
        printf("The new matrix Β is:\n");
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                printf("%lf ", B[i * N + j]);
            }
            printf("\n");
        }
    } 

    // Υπολογισμός ελάχιστης τιμής του πίνακα B
    double localMin = B[rank * N];
    int localMinPos = rank * N;
    for (i = rank; i < N; i += size) {
        for (j = 0; j < N; j++) {
            int index = i * N + j;
            if (B[index] < localMin) {
                localMin = B[index];
                localMinPos = index;
            }
        }
    }

    // Συγχρονισμός διεργασιών και εύρεση της ελάχιστης τιμής
    double globalMin;
    int globalMinPos;
    MPI_Allreduce(&localMin, &globalMin, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&localMinPos, &globalMinPos, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

    // Εκτύπωση της ελάχιστης τιμής του πίνακα B
    if (rank == 0) {
        printf("The min value of matrix B is %lf at position (%d, %d)\n", globalMin, globalMinPos / N + 1, globalMinPos % N + 1);
    }

    // Απελευθέρωση μνήμης και τερματισμός του MPI
    free(A);
    free(B);
    MPI_Finalize();
    return 0;
}
