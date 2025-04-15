# Rogue Makefile -- remade by Pierce for Pierce Mod

SRC_DIR := src
OBJ_DIR := $(SRC_DIR)/obj
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
HDR := $(wildcard $(SRC_DIR)/*.h)
TARGET := rogue

CC := gcc
CFLAGS := -g -O2
LIBS := -lcurses
DEFS := -DHAVE_CONFIG_H

# Final executable target
$(TARGET): $(HDR) $(OBJ)
	$(CC) $(OBJ) $(LIBS) $(DEFS) -o $(TARGET)

# Rule to compile .c -> .o (ensures obj/ exists)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(LIBS) $(DEFS) -c $< -o $@

# Create the obj/ directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)
