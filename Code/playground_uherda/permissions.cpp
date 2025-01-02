#include <stdio.h>
#include <sys/stat.h>

void print_permissions(mode_t mode);

int main() {
    const char *filename = "test.txt";
    struct stat fileStat;

    // Get file status
    if (stat(filename, &fileStat) == 0) {
        printf("File: %s\n", filename);

        // Print permissions for user, group, and others
        printf("Owner permissions: ");
        print_permissions(fileStat.st_mode & S_IRWXU);
        
        printf("Group permissions: ");
        print_permissions(fileStat.st_mode & S_IRWXG);
        
        printf("Others permissions: ");
        print_permissions(fileStat.st_mode & S_IRWXO);
        
    } else {
        perror("Failed to get file status");
    }

    return 0;
}

void print_permissions(mode_t mode) {
    // User permissions
    printf((mode & S_IRUSR) ? "r" : "-");  // Read for user
    printf((mode & S_IWUSR) ? "w" : "-");  // Write for user
    printf((mode & S_IXUSR) ? "x" : "-");  // Execute for user

    // Group permissions
    printf((mode & S_IRGRP) ? "r" : "-");  // Read for group
    printf((mode & S_IWGRP) ? "w" : "-");  // Write for group
    printf((mode & S_IXGRP) ? "x" : "-");  // Execute for group

    // Others permissions
    printf((mode & S_IROTH) ? "r" : "-");  // Read for others
    printf((mode & S_IWOTH) ? "w" : "-");  // Write for others
    printf((mode & S_IXOTH) ? "x" : "-");  // Execute for others

    printf("\n");
}
