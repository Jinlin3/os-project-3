CC = gcc
CFLAGS = -g -c -m32
LDFLAGS = -m32
AR = ar -rc
RANLIB = ranlib

# Object files
OBJ = my_vm.o test.o

# Targets
all: test

# Rule to make the test executable
test: test.o my_vm.a
	$(CC) $(LDFLAGS) -o $@ $^ -lm

# Rule to make the static library
my_vm.a: my_vm.o
	$(AR) $@ $^
	$(RANLIB) $@

# Generic rule for compiling object files from C source files
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJ) my_vm.a test

.PHONY: all clean
