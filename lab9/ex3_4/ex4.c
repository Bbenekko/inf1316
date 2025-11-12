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

void percorreDirs(const char *path,int nvl) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir) {
        perror(path);
        return;
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
            printf("%*s[%s]\n", nvl * 2, "", name);
            percorreDirs(pathname, nvl + 1);
        } 
        else {
            printf("%*s%s\n", nvl * 2, "", name);
        }
    }

    closedir(dir);
}

int main() {
    printf("[%s]\n", ".");
    percorreDirs(".", 0);
    return 0;
}