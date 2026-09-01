CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g
INCLUDES = -I.

ifeq ($(OS),Windows_NT)
EXE := .exe
RUN :=
else
EXE :=
RUN := ./
endif

BASE = util.o syntax.o lexer.o parser.o

.PHONY: all test test-answer clean

all: whilec$(EXE) whilei$(EXE) test_whilelang$(EXE)

whilec$(EXE): $(BASE) virtual_stack.o emit_wasm.o whilec.o
	$(CC) $(CFLAGS) -o $@ $^

whilei$(EXE): $(BASE) interpret.o virtual_stack.o whilei.o
	$(CC) $(CFLAGS) -o $@ $^

test_whilelang$(EXE): $(BASE) interpret.o virtual_stack.o emit_wasm.o test_whilelang.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

answer/%.o: answer/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

test: test_whilelang$(EXE)
	$(RUN)test_whilelang$(EXE)

test-answer: $(BASE) interpret.o answer/virtual_stack.o answer/emit_wasm.o test_whilelang.o
	$(CC) $(CFLAGS) -o test_whilelang_answer$(EXE) $^
	$(RUN)test_whilelang_answer$(EXE)

ifeq ($(OS),Windows_NT)
clean:
	-del /Q *.o *.exe 2>NUL
	-del /Q answer\*.o 2>NUL
else
clean:
	rm -f *.o whilec whilei test_whilelang test_whilelang_answer answer/*.o
endif
