import matplotlib.pyplot as plt

# Чтение данных из файлов
def read_data(file_path):
    data = []
    with open(file_path, "r") as f:
        for line in f:
            try:
                value = float(line.strip())
                data.append(value)
            except ValueError:
                continue  # Пропускаем строки, которые нельзя преобразовать в число
    return data

# Пути к файлам (замените на свои)
file1 = "/Users/xd/Desktop/python_PAK/parallels/task_3/output1.csv"
file2 = "/Users/xd/Desktop/python_PAK/parallels/task_3/output2.csv"

var1 = read_data(file1)  # Последовательное выполнение
var2 = read_data(file2)  # Параллельное выполнение

cnt = 0
for i in range(len(var1)):
    if var2[i]/var1[i] > 1:
        cnt += 1
        print(var1[i], var2[i])
print(round(cnt/len(var1), 4))
num_cores = list(range(1, len(var1) + 1))  # Количество ядер

# Вычисляем ускорение и эффективность
speedup = [var2[i] / var1[i] for i in range(len(var1))]
# efficiency = [speedup[i] / num_cores[i] for i in range(len(var1))]

# Построение графиков
plt.figure(figsize=(8, 4))

# График времени работы
plt.subplot(1, 2, 1)
plt.plot(num_cores, var1, marker='o', label="Вариант №1")
plt.plot(num_cores, var2, marker='o', label="Вариант №2")
plt.xlabel("Количество ядер")
plt.ylabel("Время работы (сек)")
plt.title("Время работы")
plt.legend()
plt.grid(True)

# График ускорения
plt.subplot(1, 2, 2)
plt.plot(num_cores, speedup, marker='o', color='r')
plt.xlabel("Количество ядер")
plt.ylabel("Ускорение")
plt.title("Ускорение (Speedup)")
plt.grid(True)

# График эффективности
# plt.subplot(1, 3, 3)
# plt.plot(num_cores, efficiency, marker='o', color='g')
# plt.xlabel("Количество ядер")
# plt.ylabel("Эффективность")
# plt.title("Эффективность (Efficiency)")
# plt.grid(True)

plt.tight_layout()
plt.savefig("output.png", dpi=300)
plt.show()
