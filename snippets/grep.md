# `grep` Command

The `grep` command is a powerful tool in Unix-like operating systems used for searching text. It looks for patterns within files or standard input and prints lines that match the given pattern.

## Syntax

```bash
grep [OPTIONS] PATTERN [FILE...]
```

### Parameters
- `PATTERN`: The regular expression or string to search for.
- `FILE`: One or more files to search. If omitted, `grep` reads from standard input.

### Common Options
- `-i`: Ignore case distinctions.
- `-v`: Invert the match, showing lines that do not match the pattern.
- `-r` or `-R`: Recursively search directories.
- `-l`: Print only the names of files with matching lines.
- `-n`: Prefix each matching line with its line number.
- `-c`: Count the number of matching lines.
- `--color`: Highlight matches in the output.

## Examples

### Basic Usage
Search for the word "error" in `log.txt`:

```bash
grep "error" log.txt
```

### Ignore Case
Search for "error" in a case-insensitive manner:

```bash
grep -i "error" log.txt
```

### Recursive Search
Search for "TODO" in all files within the current directory and subdirectories:

```bash
grep -r "TODO" .
```

### Count Matches
Count the number of lines containing "warning":

```bash
grep -c "warning" log.txt
```

### Invert Match
Find lines that do not contain "debug":

```bash
grep -v "debug" log.txt
```

## Using `grep` in C/C++
While `grep` is primarily a shell command, you can invoke it programmatically in C/C++ using system calls, `exec` family functions, or by processing text manually.

### Example: Using `exec`

The `exec` family of functions replaces the current process with a new process. To use `grep`, you can call `execlp`, `execvp`, or related functions.

#### Example with `execlp`

```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Arguments for grep
    const char *grep = "grep";
    const char *pattern = "pattern";
    const char *filename = "filename.txt";

    // Execute grep
    execlp(grep, grep, pattern, filename, NULL);

    // If execlp fails
    perror("execlp");
    exit(EXIT_FAILURE);
}
```

#### Example with `execvp`

```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    char *args[] = {"grep", "pattern", "filename.txt", NULL};

    // Execute grep
    execvp("grep", args);

    // If execvp fails
    perror("execvp");
    exit(EXIT_FAILURE);
}
```


## Notes
- `grep` is efficient for searching text and supports advanced pattern matching with regular expressions.
- Using system calls like `exec` functions is efficient and avoids creating a new shell process.