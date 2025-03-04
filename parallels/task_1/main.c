#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>



double wtime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1.0e6;
}

/* matrix_vector_product_omp: Compute matrix-vector product c[m] = a[m][n] * b[n] */
void matrix_vector_product_omp(double *a, double *b, double *c, int n)
{
    #pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = n / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);

        for (int i = lb; i <= ub; i++) {
            c[i] = 0.0;
            for (int j = 0; j < n; j++) {
                c[i] += a[i * n + j] * b[j];
            }
        }
    }
}
void matrix_vector_product(double *a, double *b, double *c, int n)
{
    #pragma omp parallel
    {
        for (int i = 0; i < n; i++) {
            c[i] = 0.0;
            for (int j = 0; j < n; j++) {
                c[i] += a[i * n + j] * b[j];
            }
        }
    }
}


void run_parallel(int n, char* csvName, int ompOrNot)
{

    FILE *fpt = fopen(csvName, "a+");
    int threads[] = {2, 4, 7, 8, 16, 20, 40};
    int num_tests = sizeof(threads) / sizeof(threads[0]);
    double *a, *b, *c;


    a = (double *)calloc(n * n, sizeof(double));
    b = (double *)calloc(n, sizeof(double));
    c = (double *)calloc(n, sizeof(double));


    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            a[i * n + j] = i + j;
    }
    for (int j = 0; j < n; j++)
        b[j] = j;
    if (ompOrNot){
        for (int i = 0; i < num_tests; i++) {
            omp_set_num_threads(threads[i]);
            double t = wtime();
            matrix_vector_product_omp(a, b, c, n);
            t = wtime() - t;
            fprintf(fpt, "%.2f\n", t);
    }
    }
    else{
        omp_set_num_threads(1);
        double t = wtime();
        matrix_vector_product(a, b, c, n);
        t = wtime() - t;
        fprintf(fpt, "%.2f\n", t);
    }
    
    fclose(fpt);

    free(a);
    free(b);
    free(c);
}

int main(int argc, char **argv)
{

    run_parallel(20000, "output.csv", 1);
    run_parallel(40000, "output.csv", 1);
    run_parallel(20000, "output.csv", 0);
    run_parallel(40000, "output.csv", 0);
    return 0;
}
