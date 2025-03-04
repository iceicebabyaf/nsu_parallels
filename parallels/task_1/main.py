import matplotlib.pyplot as plt
import numpy as np

data = []
omp20k = []
omp40k = []
cnt = 1
with open("/Users/xd/Desktop/python_PAK/parallels/task_1/output.csv", "r") as f:
    for line in f:
        try:
            value = float(line.strip())
            if cnt <= 7:
                omp20k.append(value)
            if cnt > 7 and cnt < 15:
                omp40k.append(value) 
            if cnt == 15:
                single20k = value
            if cnt == 16:
                single40k = value
            cnt += 1
        except ValueError:
            continue


print("OMP 20k:",  omp20k)
print("OMP 40k:", omp40k)
print("T1 20k:", single20k)
print("T2 40k:", single40k)
s20k = []
s40k = []
for i in range(len(omp20k)):
    s20k.append(round(single20k / omp20k[i], 2))
    s40k.append(round(single40k / omp40k[i], 2))
print("S 20k:", s20k)
print("S 40k:", s40k)

p = [2, 4, 7, 8, 16, 20, 40]


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
plt.savefig("output.png", dpi=300)
plt.show()

