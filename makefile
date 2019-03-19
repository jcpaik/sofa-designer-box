################################################################################
# Start of: Directory set-up
################################################################################

# Names of executables
EXEC_TARGET = exec
TEST_TARGET = test

# Relative paths for source and object files
# $(TARGET) will be compiled from $(SRC_DIR)/$(TARGET)
# to object file $(BLD_DIR)/$(TARGET)
SRC_DIR = src
BLD_DIR = build
USR_FLAGS = -O2 -std=c++11 -Wall

# Entry points for targets
# Relative path from $(SRC_DIR)/$(TARGET)
EXEC_SRC_MAIN = main.cpp
TEST_SRC_MAIN = main.cpp

################################################################################
# End of: Directory set-up
################################################################################

# Setting up which utility to use
MKDIR_P ?= mkdir -p
ifeq ($(shell uname -s), Linux)
	CC = gcc
	CXX = g++
else ifeq ($(shell uname -s), Darwin)
	CC = clang
	CXX = clang++
endif

FIND_SRC_FLAGS = -name *.cpp -or -name *.c -or -name *.s
SRCS = $(shell (cd $(SRC_DIR) && find * $(FIND_SRC_FLAGS)))
OBJS = $(SRCS:%=$(BLD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

EXEC_SRCS = $(shell (cd $(SRC_DIR) && find $(EXEC_TARGET) $(FIND_SRC_FLAGS)))
EXEC_OBJS = $(EXEC_SRCS:%=$(BLD_DIR)/%.o)

EXEC_OBJ_MAIN = $(BLD_DIR)/$(EXEC_TARGET)/$(EXEC_SRC_MAIN).o
TEST_OBJ_MAIN = $(BLD_DIR)/$(TEST_TARGET)/$(TEST_SRC_MAIN).o

INC_FLAGS = -I$(SRC_DIR)/$(EXEC_TARGET) -I$(SRC_DIR)/$(TEST_TARGET)

# flags used in implicit rules
LDFLAGS = -lgmp -lgmpxx -pthread
# flags used for generating dependency when compiling codes
DEP_FLAGS = -MMD -MP
CPPFLAGS = $(DEP_FLAGS) $(USR_FLAGS) # $(INC_FLAGS)

# Linking

# Use only exec objects for linking exec target
$(EXEC_TARGET): $(EXEC_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Use exec objects and test objects except main exec obj 
# for linking test target
$(TEST_TARGET): $(filter-out $(EXEC_OBJ_MAIN), $(OBJS))
	$(CXX) -o $@ $^ $(LDFLAGS)

# assembly
# $(BLD_DIR)/%.s.o: $(SRC_DIR)/%.s
# 	$(MKDIR_P) $(dir $@)
# 	$(AS) $(ASFLAGS) -c $< -o $@

# c source
# $(BLD_DIR)/%.c.o: $(SRC_DIR)/%.c
# 	$(MKDIR_P) $(dir $@)
# 	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BLD_DIR)/$(EXEC_TARGET)/%.cpp.o: $(SRC_DIR)/$(EXEC_TARGET)/%.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BLD_DIR)/$(TEST_TARGET)/%.cpp.o: $(SRC_DIR)/$(TEST_TARGET)/%.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@ -I$(SRC_DIR)/$(EXEC_TARGET)

# maybe TODO: leave src/test/main.o at clean
.PHONY: clean
clean:
	$(RM) -r $(BLD_DIR)
	$(RM) $(EXEC_TARGET) $(TEST_TARGET)

-include $(DEPS)

