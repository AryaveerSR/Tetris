NAME = tetris

SRC_DIR = src
OUT_DIR = bin
GCC_OUT_FILE = $(OUT_DIR)/$(NAME)
VALGRIND_OUT_FILE = $(OUT_DIR)/valgrind.out.txt

DEV_FLAGS = -ggdb3 -Wall -Wextra -pedantic -Wno-unknown-pragmas
CFLAGS = -lSDL2 -lSDL2_ttf

NO_VALGRIND_FLAGS = -fsanitize=address
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=$(VALGRIND_OUT_FILE) ./$(GCC_OUT_FILE)

BUILD_CMD = gcc $(SRC_DIR)/*.c -o $(GCC_OUT_FILE)
# $(SRC_DIR)/*.h

build: clean
	@$(BUILD_CMD) $(CFLAGS)

debug: clean
	@$(BUILD_CMD) $(CFLAGS) $(DEV_FLAGS) $(NO_VALGRIND_FLAGS)

clean:
	@rm -rf $(OUT_DIR)
	@mkdir $(OUT_DIR)

run: debug
	@./$(GCC_OUT_FILE)

check: clean
	@$(BUILD_CMD) $(CFLAGS) $(DEV_FLAGS)
	@valgrind $(VALGRIND_FLAGS)
	@echo Output at $(VALGRIND_OUT_FILE)