import matplotlib.pyplot as plt


executeTime = []
with open("parallels/task_2/output.csv", "r") as f:
    for line in f:
        try:
            value = float(line.strip())
            executeTime.append(value)
        except ValueError:
            continue  #
print(executeTime)
s = []
for i in range(1,len(executeTime)):
    s.append(round(executeTime[0]/executeTime[i],2))
print(s)
p = [2, 4, 7, 8, 16, 20, 40]

plt.figure(figsize=(10, 6))
plt.plot(p, s, label=r"$S_{20k}$", marker='o', linestyle='-')
plt.plot(p, p, label=r"$S = p$", color='black', linestyle=':')

plt.xlabel("Количество потоков (p)")
plt.ylabel("Ускорение (S)")
plt.title("Зависимость ускорения от количества потоков")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("output.png", dpi=300)
plt.show()