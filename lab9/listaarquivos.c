#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>


extern int alphasort();
char pathname[MAXPATHLEN];

#define FALSE 0
#define TRUE 1

/* int file_select(struct direct *entry)
{
    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
    return (FALSE);
    else
    return (TRUE);
} */

int file_select ( struct direct *entry)
{
    return TRUE;
} 

int alphasort_new(const void *a, const void *b) {
    struct direct *da = *(struct direct **)a;
    struct direct *db = *(struct direct **)b;

    int a_ponto = (da->d_name[0] == '.');
    int b_ponto = (db->d_name[0] == '.');

    if (a_ponto != b_ponto) {
        // quem começa com '.' vai pro FINAL
        return a_ponto ? 1 : -1;
    }
    return alphasort(a, b);
}



int main() {
    int count,i;
    struct direct **files;
    int file_select();

    if (getwd(pathname) == NULL ) { printf("Error getting path\n"); exit(0);
    }
        printf("Current Working Directory = %s\n",pathname);
        count = scandir( pathname, &files, file_select, alphasort_new);

        /* If no files found, make a non-selectable menu item */
        if (count <= 0) { printf("No files in this directory\n"); exit(0);
    }
    printf("Number of files = %d\n",count);
    for (i=1;i<count+1;++i) printf("%s ", files[i-1]->d_name);
    printf("\n"); /* flush buffer */
    return 0;
}