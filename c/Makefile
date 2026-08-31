CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g
INCLUDES = -I.

FRONT = util.o syntax.o lexer.o parser.o interpret.o

.PHONY: all test test-answer clean

all: whilec test_day

whilec: $(FRONT) virtual_stack.o emit_wasm.o whilec.o
	$(CC) $(CFLAGS) -o $@ $^

test_day: $(FRONT) virtual_stack.o emit_wasm.o test_day.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

answer/%.o: answer/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

test: test_day
	./test_day

test-answer: $(FRONT) answer/virtual_stack.o answer/emit_wasm.o test_day.o
	$(CC) $(CFLAGS) -o test_day_answer $^
	./test_day_answer

clean:
	rm -f *.o whilec test_day test_day_answer answer/*.o
