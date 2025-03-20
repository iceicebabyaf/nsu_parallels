#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <vector>
#include <chrono>
#include <cmath>
#include <random>
#include <unordered_map>
#include <fstream>

using namespace std;

// ==== Глобальные переменные ====
queue<tuple<size_t, string, packaged_task<double()>>> task_queue;
unordered_map<size_t, pair<future<double>, string>> results;
mutex queue_mutex;
condition_variable queue_cv;
bool stop_server = false;
size_t task_id_counter = 0;

// ==== Генерация случайных чисел ====
double GetRandArgument(double from, double to) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<double> dis(from, to);
    return dis(gen);
}

// ==== Разные типы задач ====
tuple<size_t, string, packaged_task<double()>> GetSinTask() {
    double arg = GetRandArgument(-M_PI, M_PI);
    size_t id = task_id_counter++;
    string params = "sin(" + to_string(arg) + ")";
    packaged_task<double()> task([arg]() { return sin(arg); });
    return {id, params, move(task)};
}

tuple<size_t, string, packaged_task<double()>> GetSqrtTask() {
    double arg = GetRandArgument(0, 1000);
    size_t id = task_id_counter++;
    string params = "sqrt(" + to_string(arg) + ")";
    packaged_task<double()> task([arg]() { return sqrt(arg); });
    return {id, params, move(task)};
}

tuple<size_t, string, packaged_task<double()>> GetPowTask() {
    double base = GetRandArgument(1, 10);
    double power = GetRandArgument(0, 5);
    size_t id = task_id_counter++;
    string params = to_string(base) + "^" + to_string(power);
    packaged_task<double()> task([base, power]() { return pow(base, power); });
    return {id, params, move(task)};
}

// ==== Поток сервера ====
void server_thread() {
    while (true) {
        unique_lock<mutex> lock(queue_mutex);
        queue_cv.wait(lock, [] { return !task_queue.empty() || stop_server; });

        if (stop_server && task_queue.empty()) {
            break;
        }

        auto [task_id, params, task] = move(task_queue.front());
        task_queue.pop();
        lock.unlock();

        auto result = task.get_future(); // ✅ Сервер получает `future`
        cout << "Processing Task " << task_id << endl;
        task(); // Выполняем задачу

        lock.lock();
        results[task_id] = {move(result), params}; // ✅ Сохраняем `future` + параметры
        lock.unlock();
    }
}

// ==== Клиенты ====
void client_job(std::function<std::tuple<size_t, string, std::packaged_task<double()>>() > get_task, const std::string& filename) {
    ofstream fout(filename);
    vector<size_t> task_ids;

    for (int i = 0; i < 100; ++i) {
        auto [id, params, task] = get_task();
        {
            lock_guard<mutex> lock(queue_mutex);
            task_queue.push({id, params, move(task)});
            this_thread::sleep_for(chrono::milliseconds(5));  // Задержка между задачами

        }
        queue_cv.notify_one();
        task_ids.push_back(id);
    }

    for (size_t id : task_ids) {
        double result;
        string params;
        while (true) {
            {
                unique_lock<mutex> lock(queue_mutex);  // Используем unique_lock
                auto it = results.find(id);
                if (it != results.end()) {
                    result = it->second.first.get();
                    params = it->second.second;
                    results.erase(it);
                    break;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        fout << "Task " << id << ": " << params << " = " << result << endl;
    }

    fout.close();
}

// ==== Главная функция ====
int main() {
    thread server(server_thread);

    thread sin_client([] { client_job(GetSinTask, "sins.txt"); });
    thread sqrt_client([] { client_job(GetSqrtTask, "sqrts.txt"); });
    thread pow_client([] { client_job(GetPowTask, "pows.txt"); });

    sin_client.join();
    sqrt_client.join();
    pow_client.join();

    this_thread::sleep_for(chrono::seconds(1));  // Даем серверу доработать

    {
        lock_guard<mutex> lock(queue_mutex);
        stop_server = true;
    }
    queue_cv.notify_all();
    server.join();
    
    cout << "Server stopped." << endl;
    return 0;
}
