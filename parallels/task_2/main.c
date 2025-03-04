#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

double integrate_omp(double (*func)(double), double a, double b, int n)
{
    double h = (b - a) / n;
    double sum = 0.0;

    #pragma omp parallel
    {
        double local_sum = 0.0;
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = n / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);

        for (int i = lb; i <= ub; i++)
            local_sum += func(a + h * (i + 0.5));

        #pragma omp atomic
        sum += local_sum; // Атомарное сложение
    }

    return sum * h;
}


double func(double x) {
    return exp(-x * x);
}

double wtime()
{
    return omp_get_wtime();
}

const double PI = 3.14159265358979323846;
const double a = -4.0;
const double b = 4.0;
const int nsteps = 40000000;

double run_serial()
{
    double t = wtime();
    double res = integrate_omp(func, a, b, nsteps); // интеграл в одном потоке
    t = wtime() - t;
    printf("Result (serial): %.12f; error %.12f\n", res, fabs(res - sqrt(PI)));
    return t;
}

void run_parallel()
{
    FILE *fpt = fopen("output.csv", "w+");
    int threads[] = {1, 2, 4, 7, 8, 16, 20, 40};
    for (int i = 0; i<8; i++)
    {
        omp_set_num_threads(threads[i]);
        double t = wtime();
        double res = integrate_omp(func, a, b, nsteps);
        t = wtime() - t;
        printf("Result (parallel): %.12f; error %.12f\n", res, fabs(res - sqrt(PI)));
        fprintf(fpt, "%.2f\n", t);
    }

    fclose(fpt);
}

int main(int argc, char **argv) {
    printf("Integration f(x) on [%.12f, %.12f], nsteps = %d\n", a, b, nsteps);
    run_parallel();
    return 0;
}
