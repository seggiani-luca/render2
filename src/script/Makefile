.PHONY: run debug

OUT := test

run: $(OUT)
	./test

debug: $(OUT)
	gdb ./test

$(OUT): main.c script.c ../parse/parse.c arena/arena.c
	gcc main.c script.c ../parse/parse.c arena/arena.c -fsanitize=address -g -o test
