import math
import re


def check_sin(filename):
    with open(filename, 'r') as file:
        for line in file:
            match = re.match(r"Task (\d+): sin\(([-\d\.]+)\) = ([-\d\.]+)", line)
            if match:
                task_id, arg, expected = match.groups()
                arg = float(arg)
                expected = float(expected)
                actual = math.sin(arg)


                if not math.isclose(actual, expected, abs_tol=1e-2):
                    print(f"Ошибка в {filename}: Task {task_id} -> sin({arg}) = {actual:.5f}, а не {expected:.5f}")


def check_pow(filename):
    with open(filename, 'r') as file:
        for line in file:
            match = re.match(r"Task (\d+): ([\d\.]+)\^([\d\.]+) = ([-\d\.]+)", line)
            if match:
                task_id, base, power, expected = match.groups()
                base = float(base)
                power = float(power)
                expected = float(expected)
                actual = math.pow(base, power)


                if not math.isclose(actual, expected, abs_tol=1e-2):
                    print(f"Ошибка в {filename}: Task {task_id} -> {base}^{power} = {actual:.5f}, а не {expected:.5f}")


def check_sqrt(filename):
    with open(filename, 'r') as file:
        for line in file:
            match = re.match(r"Task (\d+): sqrt\(([\d\.]+)\) = ([-\d\.]+)", line)
            if match:
                task_id, arg, expected = match.groups()
                arg = float(arg)
                expected = float(expected)
                actual = math.sqrt(arg)


                if not math.isclose(actual, expected, abs_tol=1e-2):
                    print(f"Ошибка в {filename}: Task {task_id} -> sqrt({arg}) = {actual:.5f}, а не {expected:.5f}")


check_sin("sins.txt")
check_pow("pows.txt")
check_sqrt("sqrts.txt")

print("Проверка завершена.")
