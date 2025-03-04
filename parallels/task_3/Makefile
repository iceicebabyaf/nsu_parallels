CXX = g++
CXXFLAGS = -Wall -O2 -fopenmp -lm

# Исходные файлы и объектные файлы
SRCS = main1.cpp main2.cpp schedule.cpp
OBJS = main1.o main2.o schedule.o
TARGETS = main1 main2 schedule

all: $(TARGETS)

# Компиляция объектных файлов
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Компоновка исполняемых файлов
main1: main1.o
	$(CXX) $(CXXFLAGS) $< -o $@

main2: main2.o
	$(CXX) $(CXXFLAGS) $< -o $@

schedule: schedule.o
	$(CXX) $(CXXFLAGS) $< -o $@

# Запуск
run1: main1
	./main1

run2: main2
	./main2

run3: schedule
	./schedule


# Очистка объектных файлов и исполняемых файлов
clean:
	rm -f $(OBJS) $(TARGETS) variant1.csv variant2.csv schedule_test.csv
