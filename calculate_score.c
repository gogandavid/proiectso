#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct treasure {
    int id;
    char username[32];
    int value;
} treasure;

void calculate_hunt_scores(const char *hunt_name) {
    char path[256];
    snprintf(path, sizeof(path), "treasure_hunts/%s/treasures", hunt_name);

    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    treasure t;
    int fd;
    int scores_count = 0;
    char users[100][32];
    int scores[100] = {0};

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
            fd = open(filepath, O_RDONLY);
            if (fd < 0) continue;

            if (read(fd, &t, sizeof(treasure)) == sizeof(treasure)) {
                int found = 0;
                for (int i = 0; i < scores_count; i++) {
                    if (strcmp(users[i], t.username) == 0) {
                        scores[i] += t.value;
                        found = 1;
                        break;
                    }
                }
                if (!found && scores_count < 100) {
                    strcpy(users[scores_count], t.username);
                    scores[scores_count] = t.value;
                    scores_count++;
                }
            }
            close(fd);
        }
    }
    closedir(dir);

    printf("Scores for hunt '%s':\n", hunt_name);
    for (int i = 0; i < scores_count; i++) {
        printf("%s: %d\n", users[i], scores[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <hunt_name>\n", argv[0]);
        return 1;
    }

    calculate_hunt_scores(argv[1]);
    return 0;
}
