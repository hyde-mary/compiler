# Makefile
# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra

# Source files
SRCS = main.cpp

# Output executable name
TARGET = main.exe

# Default target
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

# Clean target
clean:
	rm -f $(TARGET)
