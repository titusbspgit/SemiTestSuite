# Simple build: compile generated tests into a static library
CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99

TEST_DIR := Test_Output/GPIO/Test_Code
SRC := \
	$(TEST_DIR)/gpio_reg_wr_rd_test/gpio_reg_wr_rd_test.c
OBJ := $(SRC:.c=.o)

all: libgenerated_tests.a

libgenerated_tests.a: $(OBJ)
	@echo Archiving $@
	ar rcs $@ $(OBJ)

$(TEST_DIR)/%/%.o: $(TEST_DIR)/%/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) libgenerated_tests.a

.PHONY: all clean
