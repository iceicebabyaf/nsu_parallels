# Компилятор
CXX = pgc++

# Флаги компиляции
CXXFLAGS = -fast -Minfo=all

# Пути для заголовочных файлов
INCLUDE_DIRS = -I/usr/include -I/usr/include/boost

# Пути для библиотек
LIBRARY_DIRS = -L/usr/lib -L/usr/lib/x86_64-linux-gnu

# Библиотеки
# LIBS = -lboost_program_options -acc
LIBS = -lboost_program_options
# Исходный файл
# SRC = matrix_new.cpp
SRC = main.cpp

# Имя исполняемого файла
TARGET_GPU = heat_solver_gpu
TARGET_CPU = heat_solver_cpu

# Флаги для GPU и CPU
ACC_FLAGS_GPU = -acc -gpu=cc70
ACC_FLAGS_CPU = -acc=multicore
# ACC_FLAGS_CPU = -acc=host


# Цель по умолчанию
all: $(TARGET_GPU) $(TARGET_CPU)

# Сборка для GPU
$(TARGET_GPU): $(SRC)
	$(CXX) $(CXXFLAGS) $(ACC_FLAGS_GPU) $(INCLUDE_DIRS) $(LIBRARY_DIRS) $(LIBS) $(SRC) -o $(TARGET_GPU)

# Сборка для CPU
$(TARGET_CPU): $(SRC)
	$(CXX) $(CXXFLAGS) $(ACC_FLAGS_CPU) $(INCLUDE_DIRS) $(LIBRARY_DIRS) $(LIBS) $(SRC) -o $(TARGET_CPU)

# Очистка
clean:
	rm -f $(TARGET_GPU) $(TARGET_CPU) *.o

.PHONY: all clean
