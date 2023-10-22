CC=clang
CFLAGS=-Wall -Ivendor/datatype99 -Ivendor/metalang99/include/

default: run

format:
	find src/ \( -name '*.c' -o -name '*.h' \) -exec clang-format -i {} \;

src/lex/lex.yy.c: src/lex/lex.l
	cd src/lex && lex lex.l

build/lex.yy.o: src/lex/lex.yy.c
	$(CC) $(CFLAGS) -w -c src/lex/lex.yy.c -o build/lex.yy.o

build/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

compiler: build/main.o build/lex.yy.o
	$(CC) $(CFLAGS) build/lex.yy.o build/main.o -o compiler

clean:
	rm -rf build/*
	rm src/lex/lex.yy.c

run: compiler
	./compiler
