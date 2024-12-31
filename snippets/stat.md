# `stat` Command

The `stat` command in Unix-like operating systems is used to display detailed information about a file or a file system. It provides metadata such as file size, access permissions, modification time, and more.

## Syntax

```bash
stat [OPTIONS] FILE
```

### Parameters
- `FILE`: The file or directory whose metadata you want to retrieve.

### Common Options
- `-f`: Display information about the file system instead of the file.
- `-c FORMAT`: Use a custom format for the output.
- `--help`: Show help information.
- `--version`: Show the version of `stat`.

## Examples

### Basic Usage
Display detailed information about a file:

```bash
stat myfile.txt
```

### File System Information
Get information about the file system:

```bash
stat -f myfile.txt
```

### Custom Format
Display only the file size and permissions:

```bash
stat -c "%s %A" myfile.txt
```

## Using `stat` in C/C++
In C/C++, the `stat` system call provides similar functionality for retrieving file metadata programmatically. The `struct stat` is used to store the metadata.

### Syntax

```c
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

int stat(const char *pathname, struct stat *statbuf);
```

- `pathname`: Path to the file.
- `statbuf`: Pointer to a `struct stat` that will be filled with the file's metadata.

### Example: Basic File Metadata

```c
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    struct stat fileStat;

    if (stat(argv[1], &fileStat) < 0) {
        perror("stat");
        return 1;
    }

    printf("File: %s\n", argv[1]);
    printf("Size: %ld bytes\n", fileStat.st_size);
    printf("Permissions: %o\n", fileStat.st_mode & 0777);
    printf("Last modified: %ld\n", fileStat.st_mtime);

    return 0;
}
```

### Example: Check File Type

```c
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    struct stat fileStat;

    if (stat(argv[1], &fileStat) < 0) {
        perror("stat");
        return 1;
    }

    if (S_ISREG(fileStat.st_mode)) {
        printf("%s is a regular file.\n", argv[1]);
    } else if (S_ISDIR(fileStat.st_mode)) {
        printf("%s is a directory.\n", argv[1]);
    } else {
        printf("%s is of another type.\n", argv[1]);
    }

    return 0;
}
```

### Fields in `struct stat`
- `st_size`: File size in bytes.
- `st_mode`: File mode (permissions and type).
- `st_mtime`: Time of last modification.
- `st_atime`: Time of last access.
- `st_ctime`: Time of last status change.
- `st_uid`: User ID of owner.
- `st_gid`: Group ID of owner.

## Notes
- Use the `stat` system call to efficiently retrieve file metadata in C/C++.
- Combine `stat` with other system calls for more complex file operations.
- Always check the return value of `stat` to handle errors, such as non-existent files or insufficient permissions.
