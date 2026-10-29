CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -O2 -Iinclude
LDFLAGS  := -lGLEW -lglfw -lGL -ldl

TARGET_NAME := PyxionPlayerBeta
BUILD_DIR   := build
OBJ_DIR     := $(BUILD_DIR)/output
SRC_DIR     := src

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

TARGET := $(BUILD_DIR)/$(TARGET_NAME)

.PHONY: all clean Directories

all: Directories $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking executable: $@"
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling: $< -> $@"
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

Directories:
	@mkdir -p $(OBJ_DIR)

-include $(DEPS)

clean:
	@echo "Cleaning up build artifacts..."
	rm -rf $(BUILD_DIR)
