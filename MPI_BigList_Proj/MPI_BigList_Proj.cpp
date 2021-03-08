#include<stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <time.h>



#define NUM_ITEMS 1000
int high1; //for M: 1 to high1
int high2; //for N: 1 to high2

int array1[NUM_ITEMS];
int array2[NUM_ITEMS];
int array3[NUM_ITEMS];
int array4[NUM_ITEMS];
int array5[NUM_ITEMS];
int count = 0; //number of items in the array

void Usage(char* string) {
    printf("Usage: %s <search file> <pattern file>\n", string);
}

void diff(int a[], int c[], int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        c[i] = (a[i] / 4 - 3);
        if (c[i] < 0) {
            c[i] = 0;
        }
    }
}

void print(int a[], char s, int count)
{
    int i;
    printf("\n%c:  ", s);
    for (i = 0; i < count; i++)
    {
        printf("%d ", a[i]);
    }
}
void boss(int rank, int size, int count)
{
    //scatter the M and N to all the workers
    int M[count];
    int P[count];
    int work[count]; //small list that boss calculated
    MPI_Scatter(array1, count, MPI_INT, M, count, MPI_INT, 0, MPI_COMM_WORLD);

    //do the work for p array
    diff(M, work, count);

    //gather the work from everybody
    MPI_Gather(&work, count, MPI_INT, array3, count, MPI_INT, 0, MPI_COMM_WORLD);

    //print the array out
    print(array3, 'P', count);
}

void worker(int rank, int size, int count)
{
    //scatter the M and N to all the workers
    int M[count];
    int P[count];
    int work[count]; //small list that boss calculated
    MPI_Scatter(array1, count, MPI_INT, M, count, MPI_INT, 0, MPI_COMM_WORLD);

    //do the work for p array
    diff(M, work, count);
    //gather the work from everybody
    MPI_Gather(&work, count, MPI_INT, array3, count, MPI_INT, 0, MPI_COMM_WORLD);

}


int main(int argc, char* argv[])
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();
    // Find out rank, size
    int rank;
    int wsize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    if (argc < 1 || argc > 2) {
        Usage(argv[0]);
        return 0;
    }

    if (argc == 2) {
        FILE* file;

        file = fopen(argv[1], "r");

        if (!file) {
            printf("Problem with opening file\n");
            return 0;
        }

        while (count < NUM_ITEMS && fscanf(file, "%d", &array1[count]) == 1)
            count++;

        fclose(file);

    }

    if (rank == 0)
    {
        print(array1, 'M', count);
        boss(0, wsize, count);
    }
    else
    {
        worker(rank, wsize, count);
    }


    double end_time = MPI_Wtime();
    if (rank == 0)
    {
        printf("Time: %f\n", end_time - start_time);
    }
    MPI_Finalize();
    return 0;
}
