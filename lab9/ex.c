#include <sys/types.h> 
#include <dirent.h>
#include <sys/dir.h>
#include <stdio.h>
int main(void)
{
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir("."))) return 1;
    if (!(entry = readdir(dir))) return 2;
    do {
        if (entry->d_type == DT_DIR) printf("%s é diretorio\n",entry->d_name);
        else if (entry->d_type == DT_REG) printf("%s is file \n",entry->d_name);
        else if (entry->d_type == DT_SOCK) printf("%s is socket\n",entry->d_name);
        else if (entry->d_type == DT_LNK) printf ("%s is link\n",entry->d_name);
        else if (entry->d_type == DT_FIFO) printf ("%s is named pipe\n",entry->d_name);
        else if (entry->d_type == DT_CHR) printf ("%s is char-device\n",entry->d_name);
        else if (entry->d_type == DT_BLK) printf ("%s is block-device\n",entry->d_name);
        else ("%s tem tipo default UNKNOWN\n", entry->d_name);
    } while (entry = readdir(dir));
 return 0; 
}