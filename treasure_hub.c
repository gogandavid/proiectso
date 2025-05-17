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
int pipefd[2];

void sigchildHandler(int sig) {
    char buffer[1024];
    close(pipefd[1]);
    int n = read(pipefd[0], buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }
    close(pipefd[0]);
    wait(NULL);
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

            pipe(pipefd);
            pid_t pid = fork();
            if (pid == 0) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                pause();
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
        } else if (strcmp(comanda, "calculate_score") == 0) {
            DIR *dir = opendir("treasure_hunts");
            if (!dir) {
                perror("opendir");
                continue;
            }

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    int score_pipe[2];
                    pipe(score_pipe);
                    pid_t pid = fork();
                    if (pid == 0) {
                        close(score_pipe[0]);
                        dup2(score_pipe[1], STDOUT_FILENO);
                        execl("./calculate_score", "calculate_score", entry->d_name, NULL);
                        perror("execl");
                        exit(1);
                    } else {
                        close(score_pipe[1]);
                        char buffer[1024];
                        int n = read(score_pipe[0], buffer, sizeof(buffer) - 1);
                        if (n > 0) {
                            buffer[n] = '\0';
                            printf("%s", buffer);
                        }
                        close(score_pipe[0]);
                        waitpid(pid, NULL, 0);
                    }
                }
            }
            closedir(dir);
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
            } else {
                kill(getpid(), SIGCONT);
            }
        }
    }
    return 0;
}


