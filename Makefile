PROJECT_NAME = VulkanEngine

BUILD_DIR = bin
OBJ_DIR = lib
SRC_DIR = src
DEPEND_DIR = dependencies
RESOURCE_DIR = resources
SHADER_DIR = shaders
INCLUDE_DIR = include

CC = clang++
C_FLAGS = -std=c++2a -I$(INCLUDE_DIR) -Wall # -Ofast
LINK_FLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

rwildcard = $(foreach d, $(wildcard $(1:=/*)), $(call rwildcard, $d, $2) $(filter $(subst *, %, $2), $d)) # Find all files in directory with specified pattern

SRCS = $(call rwildcard, $(SRC_DIR), *.cpp)
DEPEND_SRC := $(call rwildcard,$(DEPEND_DIR),*.cpp)
DEPEND_SRC += $(call rwildcard,$(DEPEND_DIR),*.c)

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS)) # SRC objects
OBJS += $(patsubst $(DEPEND_DIR)/%.cpp, $(OBJ_DIR)/$(DEPEND_DIR)/%.o,$(patsubst $(DEPEND_DIR)/%.c, $(OBJ_DIR)/$(DEPEND_DIR)/%.o, $(DEPEND_SRC))) # Dependency objects

SHADERS = $(patsubst $(RESOURCE_DIR)/$(SHADER_DIR)/%.GLSL, $(OBJ_DIR)/$(SHADER_DIR)/%.spv, $(call rwildcard, $(RESOURCE_DIR)/$(SHADER_DIR), *.GLSL) ) # Shaders

all: $(OBJS) $(BUILD_DIR)/$(PROJECT_NAME).bin
	@echo !-- Built $(PROJECT_NAME).bin --!

# Compile the individual cpp files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo !-- Compiling $^ --!
	@mkdir -p $(@D)
	@$(CC) -c $^ -o $@ $(C_FLAGS)

# Compile the individual dependency c files
$(OBJ_DIR)/$(DEPEND_DIR)/%.o: $(DEPEND_DIR)/%.c
	@echo !-- Compiling $^ --!
	@mkdir -p $(@D)
	@$(CC) -c $^ -o $@ $(C_FLAGS)

# Compile the individual dependency cpp files
$(OBJ_DIR)/$(DEPEND_DIR)/%.o: $(DEPEND_DIR)/%.cpp
	@echo !-- Compiling $^ --!
	@mkdir -p $(@D)
	@$(CC) -c $^ -o $@ $(C_FLAGS)

# Compile the individual shaders
$(OBJ_DIR)/$(SHADER_DIR)/%.spv: $(RESOURCE_DIR)/$(SHADER_DIR)/%.GLSL
	@echo !-- Compiling $^ --!
	@mkdir -p temp/$(^D)
	@cp -r $^ temp/$(basename $^)

	@mkdir -p $(@D)
	@glslc temp/$(basename $^) -o $@
	@rm -r temp

# Link the files together
$(BUILD_DIR)/$(PROJECT_NAME).bin: $(OBJS)
	@echo !-- Linking $^ --!
	@$(CC) -o $@ $^ $(LINK_FLAGS)

# Run the project as an executable
run: all $(SHADERS)
	@echo !-- Running --!
	@echo # Console padding
	@./$(BUILD_DIR)/$(PROJECT_NAME).bin

# Create the necessary folders
setup:
	@echo !-- Setting Up Environment --!
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(DEPEND_DIR)

# Remove unneeded files
clean:
	@echo !-- Cleaning Folders - Excluding Dependencies --!
	@rm -rf $(BUILD_DIR)/*

	@# @mkdir -p temp
	@# @cp -r $(OBJ_DIR)/$(DEPEND_DIR) temp
	@rm -rf $(OBJ_DIR)/*
	
	@# @cp -r temp/$(DEPEND_DIR) $(OBJ_DIR)
	@# @rm -r temp

# Remove unneeded files and dependency objects
full-clean:
	@echo !-- Cleaning Folders - Including Dependencies --!
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(OBJ_DIR)/*