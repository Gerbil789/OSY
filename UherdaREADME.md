# Usefull commands
- sscanf() / for getting values from string, input


# working with file
fgets(buf, sizeof(char), strlen(buf), f)

# Usefull snippets

### Formatting string and writing to socket
```
char errorInput[1000];
sprintf(errorInput, "Neplatny tah: %s", l_buf);
write(l_sock_clients[i], errorInput, strlen(errorInput));
```
notice strlen()  - returns how long is string in buffer in contrast of sizeof() which returns size of buffer




char *line_end = strchr(line_start, '\n');


### Forking child and execute command in there

```c
    int l_child = fork();
    if (l_child == 0)
    {

        dup2(l_sock_client, STDOUT_FILENO);
        if (sem_wait(g_sem_mutex) < 0)
        {
            log_msg(LOG_ERROR, "Unable to enter into critical section!");
            exit(1);
        }

        sleep(5);
        if (execvp(command, args) == -1)  // if execvp end, it will end process, it will take control of child processs
        {
            perror("execvp failed");
            exit(1);
        }
    }
    else
    {
        wait(nullptr); // wait for childer to die

        if (sem_post(g_sem_mutex) < 0)
        {
            log_msg(LOG_ERROR, "Unable to unlock critical section!");
            exit(1);
        }

        sprintf(justMessages, "</pre>----------</body></html>\n");
        write(l_sock_client, justMessages, strlen(justMessages));

        log_msg(STDOUT_FILENO, "Closing client, command successfully executed.");
        close(l_sock_client);
        exit(0);
    }
```


# Random notes

## Has to be '\n' (empty line) between http header and <!DOCTYPE html>, otherwise browsers loading is not working.
```
"HTTP/1.1 200 OK\n"
"Server: OSY/1.1.1 (Ubuntu)\n"
"Accept-Ranges: bytes\n"
"Vary: Accept-Encoding\n"
"Content-Type: text/html\n"
"\n"
"<!DOCTYPE html>\n"
"<html>\n"
"<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /></head>\n"
"<body><pre>\n";
```


```c

struct shm_data
{
  int num_of_process;
  int counter;
};

shm_data *g_glb_data = nullptr;

```