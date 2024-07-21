NAME = tetris

SRC_DIR = src
OUT_DIR = bin
GCC_OUT_FILE = $(OUT_DIR)/$(NAME)

DEV_FLAGS = -ggdb3 -Wall -Wextra -pedantic -Wno-unknown-pragmas #-fsanitize=address
CFLAGS = -lSDL2 -lSDL2_ttf

BUILD_CMD = gcc $(SRC_DIR)/*.c -o $(GCC_OUT_FILE)
# $(SRC_DIR)/*.h

build: clean
	@$(BUILD_CMD) $(CFLAGS)

debug: clean
	@$(BUILD_CMD) $(CFLAGS) $(DEV_FLAGS)

clean:
	@rm -rf $(OUT_DIR)
	@mkdir $(OUT_DIR)

run: debug
	@./$(GCC_OUT_FILE)
