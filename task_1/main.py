import matplotlib.pyplot as plt

# Читаем данные из CSV
time20k = []
time40k = []

with open("/Users/xd/Desktop/nsu_2course/2_sem/python_PAK/parallels/lab_3/task_1/output6omp.csv", "r") as f:
    lines = [line.strip() for line in f if line.strip()]

# Проверяем, хватает ли данных
if len(lines) < 16:
    print("Ошибка: недостаточно данных в файле!")
    exit()

# Разделяем данные (первые 8 значений для 20k, остальные 8 для 40k)
time20k = list(map(float, lines[:8]))
time40k = list(map(float, lines[8:16]))

print("OMP 20k:", time20k)
print("OMP 40k:", time40k)

# Количество потоков
p = [1, 2, 4, 7, 8, 16, 20, 40]

# Вычисляем ускорение
s20k = [time20k[0] / t for t in time20k]
s40k = [time40k[0] / t for t in time40k]

print("S 20k:", s20k)
print("S 40k:", s40k)

# Строим график
plt.figure(figsize=(10, 6))
plt.plot(p, s20k, label=r"$S_{20k}$", marker='o', linestyle='-')
plt.plot(p, s40k, label=r"$S_{40k}$", marker='x', linestyle='--')
plt.plot(p, p, label=r"$S = p$", color='black', linestyle=':')

plt.xlabel("Количество потоков (p)")
plt.ylabel("Ускорение (S)")
plt.title("Зависимость ускорения от количества потоков")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("output6omp.png", dpi=300)
plt.show()
