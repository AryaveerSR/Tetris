NAME = tetris

SRC_DIR = src
OUT_DIR = bin

CFLAGS = -Wall -Werror -Wextra -pedantic -fsanitize=address -lSDL2

build: clean
#	@gcc $(SRC_DIR)/*.c $(SRC_DIR)/*.h -o $(OUT_DIR)/$(NAME) $(CFLAGS)
	@gcc $(SRC_DIR)/*.c -o $(OUT_DIR)/$(NAME) $(CFLAGS)

clean:
	@rm -rf $(OUT_DIR)
	@mkdir $(OUT_DIR)

run: build
	@./$(OUT_DIR)/$(NAME)