# L0 unit tests: plain C++, no ROS, no Godot.
# Requires only a C++17 compiler. Run:  make test
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra

TEST_SRC  = test_throttle.cpp
TEST_BIN  = test_throttle

.PHONY: all test clean

all: $(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) ../../src/throttle.hpp
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o $(TEST_BIN)

# Failure (exit != 0) is a real build/run failure, so fail the target too.
test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(TEST_BIN)
