#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <omp.h>

#define EPSILON 1e-5 
#define TAU 0.00016   

void iterate(std::vector<double>& A, std::vector<double>& b, std::vector<double>& x, int N) {
    std::vector<double> x_new(N, 0.0);
    double norm;
    int iter = 0;
    double num = 0.0, den = 0.0;

    #pragma omp parallel
    {
        do {
            // обнов x_new параллельно
            #pragma omp for
            for (int i = 0; i < N; i++) {
                double sum = 0.0;
                for (int j = 0; j < N; j++) {
                    sum += A[i * N + j] * x[j];
                }
                x_new[i] = x[i] - TAU * (sum - b[i]);
            }

            // вычисление нормы
            num = 0.0, den = 0.0;
            #pragma omp for reduction(+:num, den)
            for (int i = 0; i < N; i++) {
                double sum = 0.0;
                for (int j = 0; j < N; j++) {
                    sum += A[i * N + j] * x_new[j];
                }
                num += (sum - b[i]) * (sum - b[i]);
                den += b[i] * b[i];
            }

            // вычисл нормы в одном потоке
            #pragma omp single
            norm = sqrt(num) / sqrt(den);

            // копирую x_new в x параллельно
            #pragma omp for
            for (int i = 0; i < N; i++) {
                x[i] = x_new[i];
            }

            // +счётчик итераций и выводим норму в одном потоке
            #pragma omp single
            {
                iter++;
                std::cout << norm << std::endl;
            }

        } while (norm > EPSILON);
    }

    std::cout << "Итераций: " << iter << ", Норма: " << norm << std::endl;
}

int main() {
    int N = 20000; 
    std::ofstream fpt("variant2.csv");

    std::vector<double> A(N * N, 0.0);
    std::vector<double> b(N, 1.0);
    std::vector<double> x(N, 0.0);


    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = (i == j) ? 2.0 : 0.1;
        }
    }

    for (int threads = 1; threads <= omp_get_num_procs(); threads++) {
        omp_set_num_threads(threads);
        double start_time = omp_get_wtime();
        iterate(A, b, x, N);
        double end_time = omp_get_wtime();

        std::cout << "Threads: " << threads << ", Time: " << (end_time - start_time) << " seconds\n";
        fpt << (end_time - start_time) << "\n";

        // cброс начального приближения
        std::fill(x.begin(), x.end(), 0.0);
    }

    fpt.close();
    return 0;
}
