# Компилятор
CXX = pgc++

# Флаги компиляции
CXXFLAGS = -O3 -std=c++17 -acc -Minfo=all -I/usr/include/boost -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.11/comm_libs/12.3/openmpi4/openmpi-4.1.5/include

# Флаги линковки
LDFLAGS = -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.11/comm_libs/12.3/openmpi4/openmpi-4.1.5/lib -lmpi -lboost_program_options

# Для OpenACC
ACC_FLAGS_HOST = -acc=host
ACC_FLAGS_GPU = -acc=gpu
ACC_FLAGS_MULTICORE = -acc=multicore

# Файлы проекта
SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = heat_solver

# Сборка по умолчанию
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Сборка под GPU
gpu: clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(ACC_FLAGS_GPU)" $(TARGET)

# Сборка под CPU
cpu: clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(ACC_FLAGS_HOST)" $(TARGET)

# Сборка под multicore
multicore: clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(ACC_FLAGS_MULTICORE)" $(TARGET)

# Профилирование
profile: clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -g -O0" $(TARGET)

# Очистка
clean:
	rm -f $(OBJ) $(TARGET)

# Проверка
print:
	@echo "Компилятор: $(CXX)"
	@echo "Флаги компиляции: $(CXXFLAGS)"
	@echo "Флаги линковки: $(LDFLAGS)"
