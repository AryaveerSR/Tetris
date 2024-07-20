NAME = tetris

SRC_DIR = src
OUT_DIR = bin

DEV_FLAGS = -g -Wall -Wextra -pedantic -Wno-unknown-pragmas -fsanitize=address
CFLAGS = -lSDL2

build: clean
#	@gcc $(SRC_DIR)/*.c $(SRC_DIR)/*.h -o $(OUT_DIR)/$(NAME) $(CFLAGS)
	@gcc $(SRC_DIR)/*.c -o $(OUT_DIR)/$(NAME) $(CFLAGS) $(DEV_FLAGS)

clean:
	@rm -rf $(OUT_DIR)
	@mkdir $(OUT_DIR)

run: build
	@./$(OUT_DIR)/$(NAME)