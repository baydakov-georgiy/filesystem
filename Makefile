CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
GTEST_FLAGS = -DGTEST_HAS_PTHREAD=1 -lgtest -lgtest_main -lpthread

SOURCES = Rope.cpp AVLHTree.cpp FileSystem.cpp
OBJECTS = $(SOURCES:.cpp=.o)
MAIN_OBJ = main.o
TEST_OBJ = tests.o

EXECUTABLE = main
TEST_EXECUTABLE = tests

.PHONY: all clean test run

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

clean:
	rm -f $(OBJECTS) $(MAIN_OBJ) $(TEST_OBJ) $(EXECUTABLE) $(TEST_EXECUTABLE)

run: $(EXECUTABLE)
	./$(EXECUTABLE)
