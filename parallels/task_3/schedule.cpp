#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <omp.h>

#define EPSILON 1e-5  // Точность
#define TAU 0.00016   // Параметр метода

void iterate(std::vector<double>& A, std::vector<double>& b, std::vector<double>& x, int N, std::string schedule_type, int chunk_size) {
    std::vector<double> x_new(N, 0.0);
    double norm;
    int iter = 0;

    double start_time = omp_get_wtime();  // Засекаем время
    double num, den;
    #pragma omp parallel
    {
        do {
            #pragma omp for schedule(static, chunk_size)
            for (int i = 0; i < N; i++) {
                double sum = 0.0;
                for (int j = 0; j < N; j++) {
                    sum += A[i * N + j] * x[j];
                }
                x_new[i] = x[i] - TAU * (sum - b[i]);
            }

            num = 0.0, den = 0.0; // <-- Теперь объявлены внутри параллельной области

            #pragma omp for reduction(+:num, den) schedule(static, chunk_size)
            for (int i = 0; i < N; i++) {
                double sum = 0.0;
                for (int j = 0; j < N; j++) {
                    sum += A[i * N + j] * x_new[j];
                }
                num += (sum - b[i]) * (sum - b[i]);
                den += b[i] * b[i];
            }

            #pragma omp single
            norm = sqrt(num) / sqrt(den);

            #pragma omp for schedule(static, chunk_size)
            for (int i = 0; i < N; i++) {
                x[i] = x_new[i];
            }

            iter++;
        } while (norm > EPSILON);
    }

    double end_time = omp_get_wtime();  // Засекаем время
    std::cout << "Schedule: " << schedule_type << ", Chunk: " << chunk_size 
              << ", Итераций: " << iter << ", Время: " << (end_time - start_time) << " сек" << std::endl;
}

int main() {
    int N = 200;  // Размерность системы
    int num_threads = omp_get_num_procs();  // Количество потоков
    std::ofstream fpt("schedule_test.csv");

    std::vector<double> A(N * N, 0.0);
    std::vector<double> b(N, 1.0);
    std::vector<double> x(N, 0.0);

    // Инициализация матрицы A (диагонально доминирующая)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = (i == j) ? 2.0 : 0.1;
        }
    }

    std::vector<std::string> schedules = {"static", "dynamic", "guided"};
    std::vector<int> chunk_sizes = {1, 10, 100, 1000};  // Размеры чанков

    fpt << "Array size: " << N << "x" << N << "\n" << "Threads: " << num_threads << "\n";
    fpt << "Schedule,Chunk,Time\n";

    for (const auto& sched : schedules) {
        for (int chunk : chunk_sizes) {
            omp_set_num_threads(num_threads);
            std::fill(x.begin(), x.end(), 0.0);
            iterate(A, b, x, N, sched, chunk);
            fpt << sched << "," << chunk << "," << omp_get_wtime() << "\n";
        }
    }

    fpt.close();
    return 0;
}
