#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <boost/program_options.hpp>
#include <openacc.h>
#include <chrono>

namespace po = boost::program_options;

#define OFFSET(x, y, N) ((x) * (N) + (y))

void saveMatrixToFile(const double* U, int N, const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
        return;
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            outfile << U[OFFSET(i, j, N)] << " ";
        }
        outfile << std::endl;
    }
    outfile.close();
}

void initializeOnDevice(double* U1, double* U2, int N) {
    #pragma acc parallel loop collapse(2) deviceptr(U1, U2)
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            U1[OFFSET(i, j, N)] = 0.0;
            U2[OFFSET(i, j, N)] = 0.0;
        }
    }

    #pragma acc parallel loop deviceptr(U1, U2)
    for (int j = 0; j < N; ++j) {
        U1[OFFSET(0, j, N)] = 10.0 + (10.0 * j) / (N - 1);           // верх
        U1[OFFSET(j, N - 1, N)] = 20.0 + (10.0 * j) / (N - 1);       // право
        U1[OFFSET(N - 1, N - 1 - j, N)] = 30.0 - (10.0 * j) / (N - 1); // низ
        U1[OFFSET(N - 1 - j, 0, N)] = 20.0 - (10.0 * j) / (N - 1);   // лево

        U2[OFFSET(0, j, N)] = 10.0 + (10.0 * j) / (N - 1);           // верх
        U2[OFFSET(j, N - 1, N)] = 20.0 + (10.0 * j) / (N - 1);       // право
        U2[OFFSET(N - 1, N - 1 - j, N)] = 30.0 - (10.0 * j) / (N - 1); // низ
        U2[OFFSET(N - 1 - j, 0, N)] = 20.0 - (10.0 * j) / (N - 1);   // лево
    }
}

int solve_heat_equation(int N, double eps, int max_iter, double& error, double* U) {
    std::vector<double> U1(N * N);
    std::vector<double> U2(N * N);
    double* U1_host = U1.data();
    double* U2_host = U2.data();

    int iter = 0;         // Объявляем iter здесь
    double maxdiff = 1.0; // Объявляем maxdiff здесь

    #pragma acc data create(U1_host[0:N*N], U2_host[0:N*N])
    {
        double* U1_dev = (double*)acc_deviceptr(U1_host);
        double* U2_dev = (double*)acc_deviceptr(U2_host);
        initializeOnDevice(U1_dev, U2_dev, N);

        double* current_dev = U1_dev;
        double* next_dev = U2_dev;

        do {
            #pragma acc parallel loop tile(32,32) deviceptr(current_dev, next_dev)
            for (int i = 1; i < N - 1; ++i) {
                for (int j = 1; j < N - 1; ++j) {
                    int idx = OFFSET(i, j, N);
                    next_dev[idx] = 0.25 * (current_dev[OFFSET(i-1, j, N)] + current_dev[OFFSET(i+1, j, N)] +
                                            current_dev[OFFSET(i, j-1, N)] + current_dev[OFFSET(i, j+1, N)]);
                }
            }

            if (iter % 5000 == 0) {
                double sum_sq_diff = 0.0;
                #pragma acc parallel loop collapse(2) reduction(+:sum_sq_diff) deviceptr(current_dev, next_dev)
                for (int i = 1; i < N - 1; ++i) {
                    for (int j = 1; j < N - 1; ++j) {
                        int idx = OFFSET(i, j, N);
                        double diff = next_dev[idx] - current_dev[idx];
                        sum_sq_diff += diff * diff;
                    }
                }
                maxdiff = sqrt(sum_sq_diff / (double)((N-2) * (N-2)));
            }

            double* temp = current_dev;
            current_dev = next_dev;
            next_dev = temp;

            iter++;
        } while (maxdiff > eps && iter < max_iter);

        // Копирование с устройства на хост
        acc_memcpy_from_device(U, current_dev, N * N * sizeof(double));
    }

    error = maxdiff;
    return iter;
}

int main(int argc, char* argv[]) {
    po::options_description desc("Доступные опции");
    desc.add_options()
        ("help,h", "Показать справку")
        ("N", po::value<int>()->default_value(128), "Размер сетки")
        ("eps", po::value<double>()->default_value(1e-6), "Точность")
        ("max_iter", po::value<int>()->default_value(1000000), "Максимальное количество итераций");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    int N = vm["N"].as<int>();
    double eps = vm["eps"].as<double>();
    int max_iter = vm["max_iter"].as<int>();

    if (N < 3 || eps <= 0 || max_iter <= 0) {
        std::cerr << "Некорректные параметры: N >= 3, eps > 0, max_iter > 0\n";
        return 1;
    }

    std::vector<double> U(N * N);
    auto start = std::chrono::high_resolution_clock::now();
    double error;
    int iterations = solve_heat_equation(N, eps, max_iter, error, U.data());
    auto end = std::chrono::high_resolution_clock::now();
    double time = std::chrono::duration<double>(end - start).count();

    std::cout << "Итераций: " << iterations << "\n";
    std::cout << "Ошибка: " << error << "\n";
    std::cout << "Время (с): " << time << "\n";

    saveMatrixToFile(U.data(), N, "result_matrix.txt");

    if (N == 10 || N == 13) {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                std::cout << U[OFFSET(i, j, N)] << " ";
            }
            std::cout << std::endl;
        }
    }

    return 0;
}
