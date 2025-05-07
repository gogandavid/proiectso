#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

volatile sig_atomic_t monitor_pid = 0;
volatile sig_atomic_t monitor_exited = 0;

void sigusr1Handler(int sig) {
    char cmd[128] = {0};
    int fd = open("/tmp/monitor_cmd.txt", O_RDONLY);
    if (fd >= 0) {
        read(fd, cmd, sizeof(cmd) - 1);
        close(fd);
        cmd[strcspn(cmd, "\n")] = 0;

        if (strcmp(cmd, "list_hunts") == 0) {
            printf("[Monitor] Listing all hunts\n");
            exec("./treasure_manager()")
        } else if (strcmp(cmd, "list_treasures") == 0) {
            printf("[Monitor] Listing all treasures in this hunt\n");
        } else if (strcmp(cmd, "view_treasure") == 0) {
            printf("[Monitor] Viewing a specific treasure\n");
        } else {
            printf("[Monitor] Unknown command: %s\n", cmd);
        }
        usleep(500000);
    } else {
        perror("Monitor failed to read command file");
    }
}

void sigchildHandler(int sig) {
    int status;
    wait(&status);
    monitor_exited = 1;
    printf("[Hub] Monitor process exited with status %d\n", WEXITSTATUS(status));
}

int main() {
    struct sigaction sausr1, sachild;

    sausr1.sa_handler = sigusr1Handler;
    sigemptyset(&sausr1.sa_mask);
    sausr1.sa_flags = 0;
    sigaction(SIGUSR1, &sausr1, NULL);

    sachild.sa_handler = sigchildHandler;
    sigemptyset(&sachild.sa_mask);
    sachild.sa_flags = 0;
    sigaction(SIGCHLD, &sachild, NULL);

    char comanda[128];
    int monitor_running = 0;

    while (1) {
        printf("treasure_hub> ");
        fflush(stdout);

        if (!fgets(comanda, sizeof(comanda), stdin)) {
            break;
        }
        comanda[strcspn(comanda, "\n")] = '\0'; 

        if (strcmp(comanda, "start_monitor") == 0) {
            if (monitor_running) {
                printf("Monitor already running.\n");
                continue;
            }

            pid_t pid = fork();
            if (pid == 0) {
                struct sigaction sa;
                sa.sa_handler = sigusr1Handler;
                sigemptyset(&sa.sa_mask);
                sa.sa_flags = 0;
                sigaction(SIGUSR1, &sa, NULL);

                while (1) {
                    pause();
                }
            } else if (pid > 0) {
                monitor_pid = pid;
                monitor_running = 1;
                monitor_exited = 0;
                printf("Monitor started with PID %d\n", pid);
            } else {
                perror("fork");
            }

        } else if (strcmp(comanda, "stop_monitor") == 0) {
            if (!monitor_running) {
                printf("Monitor not running.\n");
                continue;
            }

            kill(monitor_pid, SIGTERM);
            printf("Stopping monitor.\n");

            while (!monitor_exited) {
                usleep(100000);
            }

            monitor_running = 0;

        } else if (strcmp(comanda, "exit") == 0) {
            if (monitor_running) {
                printf("Cannot exit while monitor is running. Use stop_monitor first.\n");
                continue;
            }
            break;

        } else {
            if (!monitor_running) {
                printf("Monitor is not running.\n");
                continue;
            }
            
            int fd = open("/tmp/monitor_cmd.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd < 0) {
                perror("open");
                continue;
            }
            dprintf(fd, "%s\n", comanda);
            close(fd);
            kill(monitor_pid, SIGUSR1);
        }
    }
    return 0;
}
