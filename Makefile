CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
GTEST_FLAGS = -DGTEST_HAS_PTHREAD=1 -lgtest -lgtest_main -lpthread

SOURCES = Rope.cpp AVLHTree.cpp FileSystem.cpp
OBJECTS = $(SOURCES:.cpp=.o)
MAIN_OBJ = main.o
TEST_OBJ = tests.o
BENCH_OBJ = benchmark.o

EXECUTABLE = main
TEST_EXECUTABLE = tests
BENCH_EXECUTABLE = bench

.PHONY: all clean test run benchmark plot

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(OBJECTS) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(GTEST_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_OBJ): tests.cpp
	$(CXX) $(CXXFLAGS) -DGTEST_HAS_PTHREAD=1 -c $< -o $@

$(BENCH_EXECUTABLE): $(OBJECTS) $(BENCH_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

benchmark: $(BENCH_EXECUTABLE)
	./$(BENCH_EXECUTABLE)
	python3 plot_benchmarks.py

plot:
	python3 plot_benchmarks.py

clean:
	rm -f $(OBJECTS) $(MAIN_OBJ) $(TEST_OBJ) $(BENCH_OBJ) $(EXECUTABLE) $(TEST_EXECUTABLE) $(BENCH_EXECUTABLE)
	rm -f benchmark_*.csv benchmark_*.png benchmark_statistics.txt

run: $(EXECUTABLE)
	./$(EXECUTABLE)
