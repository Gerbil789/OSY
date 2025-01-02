#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// Obslužná funkce pro SIGUSR1
void handle_usr1(int signum) {
    printf("Obdržel jsem SIGUSR1!\n");
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_usr1;  // Nastavení obsluhy signálu
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);  // Vyprázdnění masky signálů

    // Nastavení obsluhy pro SIGUSR1
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Čekám na SIGUSR1... (PID: %d)\n", getpid());
    while (1) {
        pause();  // Čeká na signál
    }

    return 0;
}
