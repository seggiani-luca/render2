.PHONY: $(EX) run

# -- sources
LIB := lib
LIB_C := $(shell find $(LIB) -name "*.c")
SRC := src
SRC_C := $(shell find $(SRC) -name "*.c")
SRC_H := $(shell find $(SRC) -name "*.h")

# -- objects

EX := exe 
OUT := out
SRC_OUT_C := $(subst $(SRC), $(OUT), $(SRC_C:.c=.o))
LIB_OUT_C := $(subst $(LIB), $(OUT), $(LIB_C:.c=.o))

# -- tools
C := gcc
L := gcc
CFLAGS := -Wall -Wextra -Wno-missing-braces -g
LFLAGS := -lglfw -lGL
OFLAGS :=

# -- targets
all: $(EX)

run:
	@./$(EX)

debug:
	@gdb ./$(EX)

asan:
	@./$(EX) 2> tst/asan.txt

$(EX): $(SRC_OUT_C) $(LIB_OUT_C)
	@echo "Linking objects ..."
	@$(L) $(SRC_OUT_C) $(LIB_OUT_C) $(LFLAGS) $(OFLAGS) -o $@

$(OUT)/%.o: $(SRC)/%.c | $(OUT)
	@echo "Compiling source $< ..."
	@mkdir -p $(dir $@)
	@$(C) $(CFLAGS) $(OFLAGS) -c $< -o $@

$(OUT)/%.o: $(LIB)/%.c | $(OUT)
	@echo "Compiling lib source $< ..."
	@mkdir -p $(dir $@)
	@$(C) $(CFLAGS) -c $< -o $@

$(OUT):
	@mkdir -p $(OUT)

clean:
	@echo "Cleaning sources ..."
	@rm -rf $(OUT)
	@rm -rf $(EX)
