#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct treasure{
    int id;
    char username[32];
    float lat;
    float lon;
    char clue[100];
    int value;
} treasure;

void wlog(char *hunt_id, char *mesaj){
    char path[100];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, "huntlog");
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0777);
    if(fd == -1){
        perror("open");
        exit(-1);
    }
    dprintf(fd, "%s\n", mesaj);
    close(fd);
}

void csymlink(char *hunt_id){
    char target[100];
    snprintf(target, sizeof(target), "%s/%s", hunt_id, "huntlog");

    char link[100];
    snprintf(link, sizeof(link), "huntlog-%s", hunt_id);
    symlink(target, link);
}

void addtreasure(char* hunt_id){
    mkdir(hunt_id,0777);
    char path[100];
    snprintf(path, sizeof(path), "%s/%s",hunt_id, "treasure.dat");

    treasure t;
    printf("Introdu: ID Username Latitudine Longitudine Valoare\n");
    scanf("%d %31s %f %f %d", &t.id, t.username, &t.lat, &t.lon, &t.value);
    printf("\nIntrodu indiciul: ");
    getchar();
    fgets(t.clue, 100, stdin);
    t.clue[strcspn(t.clue, "\n")] = '\0';

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if(fd == -1){
        perror("open");
        exit(-1);
    }

    write(fd, &t, sizeof(treasure));
    close(fd);

    wlog(hunt_id, "Treasure added.");
    csymlink(hunt_id);
}

void listtreasure(char *hunt_id){
    char path[100];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, "treasure.dat");
    struct stat s;
    if(stat(path, &s) !=0){
        perror("stat");
        exit(-1);
    }

    printf("Hunt: %s, Size: %ld bytes, Last modified: %s", hunt_id, s.st_size, ctime(&s.st_mtime));

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(-1);
    }

    treasure t;
    while (read(fd, &t, sizeof(treasure)) == sizeof(treasure)) {
        printf("ID: %d, User: %s, Lat: %.6f, Lon: %.6f, Value: %d, Clue: %s\n",
               t.id, t.username, t.lat, t.lon, t.value, t.clue);
    }
    close(fd);
    wlog(hunt_id,"Treasures listed.");
}

void viewtreasure(char *hunt_id, char *treasure_id){
    char path[100];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, "treasure.dat");

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(-1);
    }

    int tid = atoi(treasure_id);
    treasure t;
    while (read(fd, &t, sizeof(treasure)) == sizeof(treasure)) {
        if (t.id == tid) {
            printf("ID: %d, User: %s, Latitudine: %.6f, Longitudine: %.6f, Clue: %s, Value: %d\n",
                   t.id, t.username, t.lat, t.lon, t.clue, t.value);
            break;
        }
    }
    close(fd);
    wlog(hunt_id, "Treasure viewed");
}

void removetreasure(char *hunt_id, char *treasure_id){
    char path[100];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, "treasure.dat");
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(-1);
    }

    char tmppath[100];
    snprintf(tmppath, sizeof(tmppath), "%s/temp.dat", hunt_id);
    int tmpfd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (tmpfd == -1) {
        perror("open temp");
        close(fd);
        exit(-1);
    }

    int tid = atoi(treasure_id);
    treasure t;
    while (read(fd, &t, sizeof(treasure)) == sizeof(treasure)) {
        if (t.id != tid) {
            write(tmpfd, &t, sizeof(treasure));
        }
    }

    close(fd);
    close(tmpfd);

    rename(tmppath, path);
    wlog(hunt_id, "Treasure removed.");
}

void removehunt(char *hunt_id){
    char path[100];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, "treasure.dat");
    unlink(path);

    snprintf(path, sizeof(path), "%s/%s", hunt_id, "huntlog");
    unlink(path);
    rmdir(hunt_id);

    char symlinkname[100];
    snprintf(symlinkname, sizeof(symlinkname), "huntlog-%s", hunt_id);
    unlink(symlinkname);

    printf("Hunt %s removed\n", hunt_id);
}

int main(int argc, char* argv[]){
    if(argc < 3){
        printf("Nu au fost introduse suficiente argumente\n");
        return 0;
    }

    if(strcmp(argv[1], "add") == 0){
        addtreasure(argv[2]);
    }
    else if(strcmp(argv[1], "list") == 0){
        listtreasure(argv[2]);
    }
    else if(strcmp(argv[1], "view") == 0 && argc >= 4){
        viewtreasure(argv[2],argv[3]);
    }
    else if(strcmp(argv[1], "remove_treasure") == 0 && argc >= 4){
        removetreasure(argv[2], argv[3]);
    }
    else if(strcmp(argv[1], "remove_hunt") == 0){
        removehunt(argv[2]);
    }

    return 0;
}
