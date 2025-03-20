#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <atomic>  
#include <omp.h>

using namespace std;
using namespace std::chrono;

void matrix_vector_product_threaded(const vector<double>& a, const vector<double>& b, vector<double>& c, int n, int num_threads) {
    vector<thread> threads;
    threads.reserve(num_threads);  // Заранее выделяем память    
    
    atomic<int> row(0);

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&]() {  
            while (true) {
                int i = row.fetch_add(1);  // Атомарно увеличиваем индекс строки
                if (i >= n) break;  // Если выходим за пределы n — выходим
                c[i] = 0.0;
                for (int j = 0; j < n; j++) {
                    c[i] += a[i * n + j] * b[j];
                }
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }
}


void run_parallel(int n, const char* csvName, int num_threads) {
    FILE *fpt = fopen(csvName, "a+");
    
    vector<double> a(n * n);
    vector<double> b(n);
    vector<double> c(n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            a[i * n + j] = i + j;
    }
    for (int j = 0; j < n; j++)
        b[j] = j;
    
    double start = omp_get_wtime();
    matrix_vector_product_threaded(a, b, c, n, num_threads);
    double end = omp_get_wtime();
    double elapsed = end - start;
    
    fprintf(fpt, "%.2f\n", elapsed);
    fclose(fpt);
}

int main() {
    vector<int> sizes = {20000, 40000};
    vector<int> threads = {1, 2, 4, 7, 8, 16, 20, 40};

    for (int n : sizes) {
        for (int num_threads : threads) {
            run_parallel(n, "output_threads.csv", num_threads);
        }
    }
    return 0;
}
