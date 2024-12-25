# Usefull commands
- sscanf() / for getting values from string, input


# Usefull snippets

### Formatting string and writing to socket
```
char errorInput[1000];
sprintf(errorInput, "Neplatny tah: %s", l_buf);
write(l_sock_clients[i], errorInput, strlen(errorInput));
```
notice strlen()  - returns how long is string in buffer in contrast of sizeof() which returns size of buffer