# Config
CXX := g++
CXXFLAGS := -std=c++17 -pthread

FILE_NAME := monty
SRC_FILE := src/monty.cpp
BIN_DIR := ./bin
TARGET := $(BIN_DIR)/monty

# Build
.PHONY: monty clean non cat test16 test100 test2000 test10000

monty: $(TARGET)

$(TARGET): $(SRC_FILE) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Commands
none: monty
	-./$(TARGET)

cat: monty
	-./$(TARGET) cat

test16: monty
	./$(TARGET) 16

test100: monty
	./$(TARGET) 100

test2000: monty
	./$(TARGET) 2000

test10000: monty
	./$(TARGET) 10000

clean:
	rm -rf $(BIN_DIR)