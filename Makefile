PROJECTNAME = VulkanEngine

CC = clang++
CFLAGS = -std=c++2a -Wall -O2
LINKFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

BUILDDIR := bin
OBJDIR := lib
SRCDIR := src
DEPENDDIR := dependencies
SHADERDIR := shaders

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC = $(call rwildcard,$(SRCDIR),*.cpp)
DEPENDSRC := $(call rwildcard,$(DEPENDDIR),*.cpp)
DEPENDSRC += $(call rwildcard,$(DEPENDDIR),*.c)

OBJS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC)) # SRC objects
OBJS += $(patsubst $(DEPENDDIR)/%.cpp, $(OBJDIR)/$(DEPENDDIR)/%.o,$(patsubst $(DEPENDDIR)/%.c, $(OBJDIR)/$(DEPENDDIR)/%.o, $(DEPENDSRC))) # Dependency objects

SHADERS := $(patsubst $(SHADERDIR)/%.GLSL, $(OBJDIR)/$(SHADERDIR)/%.spv, $(call rwildcard, $(SHADERDIR), *.GLSL) ) # Shaders

.PHONY: temp
temp:
	@echo $(SHADERS)

# Build the project
.PHONY: build
build: $(OBJS) $(BUILDDIR)/$(PROJECTNAME).bin
	@echo !-- Built $(PROJECTNAME).bin --!

# Create the necessary folders
.PHONY: setup
setup:
	@echo !-- Setting Up Environment --!
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(OBJDIR)
	@mkdir -p $(DEPENDDIR)

# Remove unneeded files
.PHONY: clean
clean:
	@echo !-- Cleaning Folders --!
	@rm -rf $(BUILDDIR)/*

	@# @mkdir -p temp
	@# @cp -r $(OBJDIR)/$(DEPENDDIR) temp
	@rm -rf $(OBJDIR)/*
	
	@# @cp -r temp/$(DEPENDDIR) $(OBJDIR)
	@# @rm -r temp

# Remove unneeded files and dependency objects
.PHONY: full-clean
full-clean:
	@echo !-- Cleaning Folders - Including Dependencies --!
	@rm -rf $(BUILDDIR)/*
	@rm -rf $(OBJDIR)/*

# Compile the individual cpp files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo !-- Compiling $^ --!
	@mkdir -p $(@D)
	@$(CC) -c $^ -o $@ $(CFLAGS)

# Compile the individual dependency c files
$(OBJDIR)/$(DEPENDDIR)/%.o: $(DEPENDDIR)/%.c
	@echo !-- Compiling $^ --!
	@mkdir -p $(@D)
	@$(CC) -c $^ -o $@ $(CFLAGS)

# Compile the individual dependency cpp files
$(OBJDIR)/$(DEPENDDIR)/%.o: $(DEPENDDIR)/%.cpp
	@echo !-- Compiling $^ --!
	@mkdir -p $(@D)
	@$(CC) -c $^ -o $@ $(CFLAGS)

# Compile the individual shaders
$(OBJDIR)/$(SHADERDIR)/%.spv: $(SHADERDIR)/%.GLSL
	@echo !-- Compiling $^ --!
	@mkdir -p temp/$(^D)
	@cp -r $^ temp/$(basename $^)

	@mkdir -p $(@D)
	@glslc temp/$(basename $^) -o $@
	@rm -r temp

# Link the files together
$(BUILDDIR)/$(PROJECTNAME).bin: $(OBJS)
	@echo !-- Linking $^ --!
	@$(CC) -o $@ $^ $(LINKFLAGS)

# Run the project as an executable
.PHONY: run
run: build $(SHADERS)
	@echo !-- Running --!
	@echo # Console padding
	@./$(BUILDDIR)/$(PROJECTNAME).bin
