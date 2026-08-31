CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g
INCLUDES = -I.

BASE = util.o syntax.o lexer.o parser.o

.PHONY: all test test-answer clean

all: whilec whilei test_day

whilec: $(BASE) virtual_stack.o emit_wasm.o whilec.o
	$(CC) $(CFLAGS) -o $@ $^

whilei: $(BASE) interpret.o virtual_stack.o whilei.o
	$(CC) $(CFLAGS) -o $@ $^

test_day: $(BASE) interpret.o virtual_stack.o emit_wasm.o test_day.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

answer/%.o: answer/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

test: test_day
	./test_day

test-answer: $(BASE) interpret.o answer/virtual_stack.o answer/emit_wasm.o test_day.o
	$(CC) $(CFLAGS) -o test_day_answer $^
	./test_day_answer

clean:
	rm -f *.o whilec whilei test_day test_day_answer answer/*.o
