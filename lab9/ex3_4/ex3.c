#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

int percorreDirs(const char *path) {
    DIR *dir;
    struct dirent *entry;
    int total = 0;

    dir = opendir(path);
    if (!dir) {
        perror(path);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        char pathname[MAXPATHLEN];
        snprintf(pathname, sizeof(pathname), "%s/%s", path, name);

        struct stat st;
        if (stat(pathname, &st) == -1) {
            perror(pathname);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            printf("%s:\n", pathname);
            total += percorreDirs(pathname);
        } 
        else {
            printf("%s: %d bytes\n", pathname, (int)st.st_size);
            total += (int)st.st_size;
        }
    }

    closedir(dir);
    return total;
}

int main() {
    int total = percorreDirs(".");
    printf("Total de bytes = %d\n", total);
    return 0;
}