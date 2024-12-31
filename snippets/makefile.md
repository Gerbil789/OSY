# Makefile

A Makefile is a simple way to manage the build process for your project. It uses `make` to automate compiling and linking, ensuring that only the necessary parts of your code are rebuilt.

## Basic Structure

```make
# Variables
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =

# Targets
all: my_program

my_program: main.o utils.o
	$(CC) $(LDFLAGS) -o my_program main.o utils.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o my_program
```

### Explanation
- **Variables**:
  - `CC`: The compiler to use (e.g., `gcc`, `g++`).
  - `CFLAGS`: Flags for the compiler, like warnings and optimizations.
  - `LDFLAGS`: Flags for the linker.
- **Targets**:
  - `all`: Default target, usually depends on the main executable.
  - `my_program`: Links the object files into the final executable.
  - `main.o` and `utils.o`: Compile source files into object files.
  - `clean`: A utility target to remove build artifacts.

### Key Features
- **Automatic Dependency Tracking**: Files like `utils.h` are included as dependencies, so if `utils.h` changes, `utils.o` will be rebuilt.
- **Phony Targets**: Use `.PHONY` to mark targets like `clean` that don’t produce a file.

```make
.PHONY: all clean
```

## Tips
- Use `make -j` to enable parallel building, which speeds up compilation.
- Organize your Makefile by grouping similar targets and adding comments.
- Consider splitting large projects into subdirectories with separate Makefiles, using `include` to combine them.

