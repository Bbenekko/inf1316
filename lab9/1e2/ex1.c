#define _DEFAULT_SOURCE  // garante scandir/alphasort no glibc moderno
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

char pathname[PATH_MAX];

#define FALSE 0
#define TRUE 1

int file_select(const struct dirent *entry)
{
    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
    return (FALSE);
    else
    return (TRUE);
}

/* int file_select ( struct direct *entry)
{
 char *ptr;
 char *rindex(char *s, char c);

 if ((strcmp(entry->d_name, ".")== 0) || (strcmp(entry->d_name, "..") == 0))
 return (FALSE);
 //Check for filename extensions 
 ptr = rindex(entry->d_name, '.')
 if ((ptr != NULL) && ((strcmp(ptr, ".c") == 0) || (strcmp(ptr, ".h") == 0) ||
 (strcmp(ptr, ".o") == 0) ))
return (TRUE);
 else
return(FALSE);
}  */

int main() 
{
    struct dirent **files;
    struct stat stats;
    int count;
    time_t now = time(NULL);

    if (getcwd(pathname, sizeof(pathname)) == NULL ) 
    { 
        printf("Error getting pathname\n"); 
        exit(0);
    }
    printf("Current Working Directory = %s\n",pathname);
    
    count = scandir( pathname, &files, file_select, alphasort);

    /* If no files found, make a non-selectable menu item */
    if (count <= 0) 
    { 
        printf("No files in this directory\n"); 
        exit(0);
    }

    printf("Number of files = %d\n",count);

    for (int i = 0; i < count; ++i) 
    {
        char path[PATH_MAX];
        int n = snprintf(path, sizeof(path), "%s/%s", pathname, files[i]->d_name);
        if (n < 0 || n > sizeof(path))
        {
            printf("Error getting path\n"); 
            exit(0);
        }

        if (stat(path, &stats) != 0) 
        {           
            printf("Erro ao acessar os stats");
            exit(0);
        }

        // TODO converter time para dias
        long tempo_dias = (long)(difftime(now, stats.st_mtime) / (60 * 60 * 24));

        printf("%s     inode %ld      size: %ld    age: %ld\n", files[i]->d_name, files[i]->d_ino, stats.st_size, tempo_dias);
    }

    printf("\n"); /* flush buffer */
    
    for (int i = 0; i < count; ++i) free(files[i]);
    free(files);

    return 0;
}
