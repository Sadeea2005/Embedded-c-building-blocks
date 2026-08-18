# Host build: compile the modules + tests with strict warnings and run them.
CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Werror -O2 -g
SRCS    = ring_buffer.c mem_pool.c cmd_parser.c state_machine.c tests.c
TARGET  = tests

.PHONY: all test clean
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
