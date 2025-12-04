#include "teste.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    Message msg;
    Message drResponse;

    strcpy(msg.pathName, "");
    msg.sizePathName = strlen(msg.pathName);
    strcpy(msg.dirName, "subdir1");
    msg.sizeDirName = strlen(msg.dirName);
    msg.owner = 1;

    /*
    create_subDirectory(&msg, &drResponse);
    if (drResponse.sizePathName == -1) {
        printf("Erro ao remover o diretório.\n");
    } else {
        printf("Diretório removido com sucesso. Caminho: %s\n", drResponse.pathName);
    }
    */

    Message in, out;

    in.owner = 1;
    strcpy(in.pathName, "subdir1/teste.txt");
    in.sizePathName = sizeof(in.pathName);
    strcpy(in.payload, "Abluble");
    in.offset = 32;

    write_file(&in, &out);

    return 0;
}