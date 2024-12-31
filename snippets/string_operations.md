# Working with Strings in C

This document summarizes the most important functions for working with strings in the C language.
The functions are part of the standard library `<string.h>` and cover copying, comparing, formatting, and manipulating text.

---

## 1. Copying and Assignment

### **`strcpy`** - copies a string
```c
char src[] = "Hello";
char dest[10];
strcpy(dest, src);  // dest now contains "Hello"
```

### **`strncpy`** - copies n characters
```c
char src[] = "Long text";
char dest[5];
strncpy(dest, src, 4);  // dest contains "Long"
dest[4] = '\0';  // Null terminator must be added manually.
```

---

## 2. String Comparison

### **`strcmp`** - compares strings

```c
char a[] = "Hello";
char b[] = "World";

if (strcmp(a, b) == 0) {
    printf("Strings are equal.\n");
} else {
    printf("Strings are different.\n");
}
```

### **`strncmp`** - compares n characters
```c
if (strncmp(a, b, 3) == 0) {
    printf("The first 3 characters are equal.\n");
}
```

### strncasecmp, strcasecmp - CASE insensitive

---

## 3. String Length

### **`strlen`** - returns the length of a string
```c
char text[] = "Programming";
printf("Length: %lu\n", strlen(text));  // Length: 11
```

---

## 4. Concatenating Strings

### **`strcat`** - appends a string to another
```c
char dest[20] = "Hello, ";
char src[] = "world!";
strcat(dest, src);  // dest: "Hello, world!"
```

### **`strncat`** - appends up to n characters
```c
strncat(dest, " How are you?", 4);  // dest: "Hello, worl How"
```

---

## 5. Searching for Substrings or Characters

### **`strchr`** - searches for a character in a string
```c
char *a = "Programming";
char *ptr = strchr(a, 'g');
printf("%s\n", ptr);  // gramming ("grejming")
```

### **`strstr`** - searches for a substring
```c
char *a = "Hello world";
char *ptr = strstr(a, "world");
printf("%s\n", ptr);  // world
```

---

## 6. Formatting Strings

### **`sprintf`** - formats text into a string
```c
char buffer[50];
int number = 42;
sprintf(buffer, "number is %d", number);
printf("%s\n", buffer);  // number is 42
```

### **`snprintf`** - safer variant with maximum length
```c
snprintf(buffer, sizeof(buffer), "Length: %d", strlen("Test"));
```


### Formatting with %c, %s, %d, %f

```c
char ch = 'A';
char str[] = "Hello";
int num = 42;
double pi = 3.14159;
printf("Character: %c, String: %s, Integer: %d, Float: %.2f\n", ch, str, num, pi);
// Character: A, String: Hello, Integer: 42, Float: 3.14
```


### Converting Numbers to Strings
```c
int num = 1234;
char buffer[20];
sprintf(buffer, "%d", num);
printf("String: %s\n", buffer);  // String: 1234

float f = 56.78;
sprintf(buffer, "%.2f", f);
printf("String: %s\n", buffer);  // String: 56.78
```

---

## 7. Splitting Strings

### **`strtok`** - splits a string into tokens
```c
char text[] = "C;Java;Python";
char *token = strtok(text, ";");

while (token != NULL) {
    printf("%s\n", token);
    token = strtok(NULL, ";");
}
// Output: C, Java, Python
```

---

## 8. Converting to Numbers

### **`atoi`** - converts a string to int
```c
int number = atoi("1234");
printf("%d\n", number);  // 1234
```

### **`atof`** - converts to double
```c
double d = atof("3.14");
```

---

## 9. Replacing Characters
```c
char text[] = "Testing text";
for (int i = 0; i < strlen(text); i++) {
    if (text[i] == 't') {
        text[i] = 'T';
    }
}
printf("%s\n", text);  // TesTing Text
```

---

## 10. Changing Letter Case

### toupper - converts a character to uppercase
```c
char c = 'a';
printf("%c\n", toupper(c));  // A
```
### tolower - converts a character to lowercase
```c
char c = 'Z';
printf("%c\n", tolower(c));  // z
```

---

## 11. Reading and Converting Numbers

### Reading and Printing Integers and Floats

```c
int number;
float decimal;
printf("Enter an integer: ");
scanf("%d", &number);
printf("Enter a float: ");
scanf("%f", &decimal);
printf("You entered integer: %d and float: %.2f\n", number, decimal);
```

### Formatting reading
```c
char param[100];
printf("Enter 'GET value HTTP' value: ");
if (scanf("GET %99s HTTP", param) == 0)
{
    printf("Bad formatting");
}
else
{
    printf("Received value: %s, length: %d\n", param, strlen(param));
}
```


### Table of parameters

| Format | Description                              | Example Input | Notes                                      |
|--------|------------------------------------------|---------------|--------------------------------------------|
| `%d`   | Read an **integer**                       | `42`          | Skips leading whitespace                   |
| `%f`   | Read a **floating-point number**          | `3.14`        | Accepts decimals                          |
| `%c`   | Read a **single character**               | `A`           | Does **not** skip whitespace               |
| `%s`   | Read a **string** (until whitespace)      | `hello`       | Buffer overflow risk without size limit    |
| `%99s` | Read a string, max **99 characters**      | `hello`       | Safer to prevent overflow                  |
| `%u`   | Read an **unsigned integer**              | `123`         |                                            |
| `%x`   | Read a **hexadecimal integer**            | `0x1A`        |                                            |
| `%o`   | Read an **octal integer**                 | `075`         |                                            |
| `%lf`  | Read a **double**                         | `3.1415`      |                                            |


## Note:
- All strings in C must be null-terminated (`\0`).
- Working with strings requires careful memory management and attention to potential buffer overflows.



## Some random examples

### Using a dynamic array
```c
char srd[] = "Hello";
char *dst[10];
dst[0] = (char*)malloc(10);
strcpy(dst[0], srd);
log_msg(LOG_INFO, dst[0]);
```


```c

char *http = strstr(buf, "HTTP");  // Find substring "HTTP" in 'buf'
char *get = strstr(buf, "GET");    // Find substring "GET" in 'buf'

if (!http || !get)  // If either "HTTP" or "GET" is not found in 'buf'
{
    log_msg(LOG_INFO, "Not valid format 'GET value HTTP'");  // Log invalid format
    return;  // Exit the function
}

int remove = strlen(buf) - strlen(http);  // Calculate position of "HTTP" by subtracting lengths

char command[15];  // Buffer to hold extracted command (max 14 characters + null terminator)

// Copy portion of 'buf' starting at position 5 (skipping 'GET ') for (remove - 5) characters
strncpy(command, buf + 5, remove - 5);

// Null-terminate the copied string at position (remove - 6) to properly end the command
command[remove - 6] = '\0';

// Check if the command is "favicon.ico" (case insensitive) and deny such requests
if (strncasecmp("favicon.ico", command, strlen("favicon.ico")) == 0)
{
    log_msg(LOG_INFO, "favicon request denied");  // Log denial of favicon request
    exit(1);  // Terminate the program
}

char *args[10];  // Array to store extracted command arguments
char *token = strtok(command, "*");  // Tokenize the command using '*' as delimiter
int index = 0;  // Initialize index to store tokens
while (token != NULL)  // Loop through all tokens
{
    args[index++] = token;  // Store each token in 'args' array
    token = strtok(NULL, "*");  // Get the next token
}
args[index] = NULL;  // Null-terminate the array of arguments

sprintf(justMessages, "Command: ");  // Initialize message to log
index = 0;  // Reset index for logging

// Append each argument to 'justMessages' string
while (args[index] != NULL)
{
    strcat(justMessages, args[index]);  // Concatenate the token to the message
    strcat(justMessages, " ");         // Add a space between tokens
    index++;
}

log_msg(LOG_INFO, justMessages);  // Log the full command with arguments

```