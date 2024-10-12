# Makefile for HL Interpreter

# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

# Target executable
TARGET = main.exe

# Source files
SRCS = src/main.cpp src/fetch_source_code.cpp

# Rule to build the target
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Rule to run the program with an input file
run: $(TARGET)
	@echo "Running the program with input file: $(filename)"
	@./$(TARGET) $(filename)

# Clean up generated files
clean:
	del $(TARGET)

# Phony targets
.PHONY: run clean
