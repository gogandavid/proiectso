#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t   sa_mask;
    int        sa_flags;
    void     (*sa_restorer)(void);
};

volatile sig_atomic_t monitor_pid = 0;
volatile sig_atomic_t monitor_exited = 0;

void sigchildHandler(int sig) {
    char buffer[128];
    int fd = open("/tmp/monitor_cmd.txt", O_RDONLY);
    if (fd >= 0) {
        read(fd, buffer, sizeof(buffer) - 1);
        close(fd);
        buffer[strcspn(buffer, "\n")] = 0;

        if (sig == SIGUSR1) {
            execl("./treasure_manager", "treasure_manager", "list", "hunt1", NULL);
        } else if (sig == SIGUSR2) {
            execl("./treasure_manager", "treasure_manager", "list", "hunt1", NULL);
        } else if (sig == SIGTERM) {
            char *token = strtok(buffer, " ");
            token = strtok(NULL, " ");
            char *hunt_id = token;
            token = strtok(NULL, " ");
            char *treasure_id = token;
            execl("./treasure_manager", "treasure_manager", "view", hunt_id, treasure_id, NULL);
        } else if (sig == SIGCONT) {
            execl("./treasure_manager", "treasure_manager", "details", "hunt1", NULL);
        } else if (sig == SIGINT) {
            printf("Stopping monitor...\n");
            exit(0);
        }
    }
    monitor_exited = 1;
    printf("Monitor process handled signal %d\n", sig);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchildHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGCONT, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    char comanda[128];
    int monitor_merge = 0;
    while (1) {
        printf("treasure_hub> ");
        fflush(stdout);

        if (!fgets(comanda, sizeof(comanda), stdin)) {
            break;
        }

        comanda[strcspn(comanda, "\n")] = 0;

        if (strcmp(comanda, "start_monitor") == 0) {
            if (monitor_merge) {
                printf("Monitor already running.\n");
                continue;
            }

            pid_t pid = fork();
            if (pid == 0) {
                while (1) {
                    pause();
                }
                exit(0);
            } else if (pid > 0) {
                monitor_pid = pid;
                monitor_merge = 1;
                printf("Monitor started with PID %d\n", pid);
            } else {
                perror("fork");
            }
        } else if (strcmp(comanda, "stop_monitor") == 0) {
            if (!monitor_merge) {
                printf("Monitor not running.\n");
                continue;
            }
            kill(monitor_pid, SIGINT);
            usleep(1000000);
            monitor_merge = 0;
        } else if (strcmp(comanda, "exit") == 0) {
            if (monitor_merge) {
                printf("Cannot exit while monitor is running. Use stop_monitor first.\n");
                continue;
            }
            break;
        } else {
            if (!monitor_merge) {
                printf("Monitor is not running. Please start it first.\n");
                continue;
            }
            int fd = open("/tmp/monitor_cmd.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd < 0) {
                perror("open");
                continue;
            }
            dprintf(fd, "%s\n", comanda);
            close(fd);

            if (strcmp(comanda, "list_hunts") == 0) {
                kill(getpid(), SIGUSR1);
            } else if (strcmp(comanda, "list_treasures") == 0) {
                kill(getpid(), SIGUSR2);
            } else if (strncmp(comanda, "view_treasure", 14) == 0) {
                kill(getpid(), SIGTERM);
            } else if (strcmp(comanda, "details") == 0) {
                kill(getpid(), SIGCONT);
            }
        }
    }
    return 0;
}

