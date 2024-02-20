CC := g++

STD := -std=c++11

CFLAGS := -g -Wall -Wextra $(STD)

SRC_DIR := src
BUILD_DIR := build

FMT_INC := ./fmt/include
CFLAGS += -I $(FMT_INC)

BLOCK_SIZE ?= 1024

BIN := rush

# get all .cpp files from src dir
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

# generate corresponding .o files in build dir
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# default target
all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# compile each .cpp file into corresponding object file, depends on .cpp file AND build directory existing
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(BIN) *.bin