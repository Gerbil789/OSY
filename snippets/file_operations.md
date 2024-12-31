# File Operations in C/C++

File operations in C/C++ allow you to interact with the file system, such as creating, reading, writing, and closing files. These operations are essential for working with persistent data storage.

## File Handling in C (POSIX API)

### Common Functions

#### `open`
Opens a file and returns a file descriptor.

```c
#include <fcntl.h>
#include <unistd.h>

int open(const char *pathname, int flags, mode_t mode);
```

- `pathname`: Path to the file.
- `flags`: File access mode (e.g., `O_RDONLY`, `O_WRONLY`, `O_RDWR`).
- `mode`: Permissions (used when creating a file).

#### `read`
Reads data from a file.

```c
ssize_t read(int fd, void *buf, size_t count);
```

- `fd`: File descriptor.
- `buf`: Buffer to store the read data.
- `count`: Number of bytes to read.

#### `write`
Writes data to a file.

```c
ssize_t write(int fd, const void *buf, size_t count);
```

- `fd`: File descriptor.
- `buf`: Buffer containing data to write.
- `count`: Number of bytes to write.

#### `close`
Closes a file descriptor.

```c
int close(int fd);
```

- `fd`: File descriptor to close.

### Example: Reading and Writing Files

```c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    int fd = open("example.txt", O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    const char *data = "Hello, File!\n";
    if (write(fd, data, 13) < 0) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Reset file offset to the beginning
    lseek(fd, 0, SEEK_SET);

    char buffer[14] = {0};
    if (read(fd, buffer, 13) < 0) {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Read from file: %s", buffer);

    close(fd);
    return 0;
}
```

## File Handling in C (Standard Library API)

The C standard library provides a simpler API for file operations using the `FILE` structure.

### Common Functions

#### `fopen`
Opens a file and returns a pointer to a `FILE` structure.

```c
#include <stdio.h>

FILE *fopen(const char *filename, const char *mode);
```

- `filename`: Path to the file.
- `mode`: Mode string (e.g., `"r"` for reading, `"w"` for writing).

#### `fread`
Reads data from a file.

```c
size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
```

- `ptr`: Pointer to the buffer to store data.
- `size`: Size of each element to read.
- `count`: Number of elements to read.
- `stream`: File stream.

#### `fwrite`
Writes data to a file.

```c
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
```

- `ptr`: Pointer to the data to write.
- `size`: Size of each element to write.
- `count`: Number of elements to write.
- `stream`: File stream.

#### `fclose`
Closes a file.

```c
int fclose(FILE *stream);
```

- `stream`: File stream to close.

### Example: Reading and Writing Files

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file = fopen("example.txt", "w+");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    const char *data = "Hello, File!\n";
    if (fwrite(data, 1, 13, file) != 13) {
        perror("fwrite");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Reset file position to the beginning
    rewind(file);

    char buffer[14] = {0};
    if (fread(buffer, 1, 13, file) != 13) {
        perror("fread");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    printf("Read from file: %s", buffer);

    fclose(file);
    return 0;
}
```

## File Handling in C++ (High-Level API)

C++ provides the `<fstream>` library for high-level file operations.

### Common Classes

#### `std::ofstream`
Used for writing to files.

#### `std::ifstream`
Used for reading from files.

#### `std::fstream`
Used for both reading and writing.

### Example: Reading and Writing Files

```cpp
#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ofstream outFile("example.txt");
    if (!outFile) {
        std::cerr << "Failed to open file for writing\n";
        return 1;
    }
    outFile << "Hello, File!\n";
    outFile.close();

    std::ifstream inFile("example.txt");
    if (!inFile) {
        std::cerr << "Failed to open file for reading\n";
        return 1;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        std::cout << "Read from file: " << line << '\n';
    }
    inFile.close();

    return 0;
}
```

## Notes
- Always check the return values of file operations for error handling.
- In C, remember to close file descriptors or file streams to avoid resource leaks.
- In C++, ensure you close file streams or let RAII handle it.
- Be mindful of file permissions and concurrency issues when working with files.
