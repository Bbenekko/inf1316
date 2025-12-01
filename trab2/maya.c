#include "structs.h"

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
#include <libgen.h>


// operacoes em arquivos

void rd(Message* msg, Message* response)
{
    FILE *file;
    char path[100];

    response->owner = msg->owner;

    strncpy(path, msg->pathName, msg->sizePathName);
    path[msg->sizePathName] = '\0';

    file = fopen(path, "r");
    if (!file) {
        response->payload[0] = '\0';
        return;
    }

    fseek(file, msg->offset * 16, SEEK_SET);
    size_t bytesRead = fread(response->payload, 1, PAYLOAD_MAX, file);

    if (bytesRead != PAYLOAD_MAX) {
        response->payload[bytesRead] = '\0';
    }
    fclose(file);
}

/* int main() {
    Message msg;
    Message rdResponse;

    strcpy(msg.pathName, "testfile.txt");
    msg.sizePathName = strlen(msg.pathName);
    msg.offset = 0;

    rd(&msg, &rdResponse);
    printf("Payload lido: %s\n", rdResponse.payload);

    msg.offset = 1;

    rd(&msg, &rdResponse);
    printf("Payload lido: %s\n", rdResponse.payload);

    msg.offset = 2;

    rd(&msg, &rdResponse);
    printf("Payload lido: %s\n", rdResponse.payload);

    msg.offset = 3;

    rd(&msg, &rdResponse);
    printf("Payload lido: %s\n", rdResponse.payload);

    msg.offset = 4;

    rd(&msg, &rdResponse);
    printf("Payload lido: %s\n", rdResponse.payload);

    return 0;
} */

//operacoes em diretorios

void putNameInResponse(DL_REP* response, const char* name, int isDir)
{
    if(response->nrnames == 0)
    {
        response->posicoes[response->nrnames].posInicial = 0;
    }
    else
    {
        response->posicoes[response->nrnames].posInicial = response->posicoes[response->nrnames - 1].posFinal + 1;
    }
    response->posicoes[response->nrnames].posFinal = response->posicoes[response->nrnames].posInicial + strlen(name) - 1;
    response->posicoes[response->nrnames].ehSubdiretorio = isDir;
    strcat(response->allfilesnames, name);
    printf("response nrnames antes: %d\n", response->nrnames);
    response->nrnames += 1;
    printf("response nrnames depois: %d\n", response->nrnames);
}

void dl(Message* msg, DL_REP* response)
{
    DIR *dir;
    struct dirent *entry;
    char path[100];

    response->owner = msg->owner;

    strncpy(path, msg->pathName, msg->sizePathName);
    path[msg->sizePathName] = '\0';

    dir = opendir(path);
    if (!dir) {
        //perror(path);
        printf("Retorno da funcao recursiva\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        const char *name = entry->d_name;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        char pathname[MAXPATHLEN];
        snprintf(pathname, sizeof(pathname), "%s/%s", path, name);
        printf("name: %s\n", name);

        struct stat st;
        if (stat(pathname, &st) == -1) 
        {
            //perror(pathname);
            response->nrnames = -1;
            continue;
        }

        if (S_ISDIR(st.st_mode)) 
        {
            printf("%s:\n", pathname);
            Message newMsg;
            strcpy(newMsg.pathName, pathname); 
            dl(&newMsg, response);
        } 
        else 
        {
            putNameInResponse(response, name, 0);
        }
    }
    closedir(dir);
}

void removeRecursivo(Message* msg, Message* response)
{    
    DIR *dir;
    struct dirent *entry;

    char path[100];

    strncpy(path, msg->pathName, msg->sizePathName);
    path[msg->sizePathName] = '\0';

    printf("Removendo recursivamente o diretorio: %s\n", path);

    dir = opendir(path);
    if (!dir) {
        printf("Saindo da recursao\n");
        perror(path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        printf("arquivo ou dir atual da remocao: %s\n", name);

        char pathname[MAXPATHLEN];
        snprintf(pathname, sizeof(pathname), "%s/%s", path, name);

        struct stat st;
        if (stat(pathname, &st) == -1) {
            perror(pathname);
            continue;
        }

        int ret = 0;
        if (S_ISDIR(st.st_mode)) {
            Message newMsg;
            strcpy(newMsg.pathName, pathname); 
            newMsg.sizePathName = strlen(newMsg.pathName);
            removeRecursivo(&newMsg, response);
            ret = rmdir(pathname);
        } 
        else {
            ret = remove(pathname);
        }
        printf("Retorno da remocao de %s: %d\n", pathname, ret);
        if (ret == -1) {
            response->sizePathName = -1;
            return;
        }
    }
    closedir(dir);
}

void dr(Message* msg, Message* response)
{ 
    char path[100];

    response->owner = msg->owner;
    response->sizePathName = 0;

    strncpy(path, msg->pathName, msg->sizePathName);
    path[msg->sizePathName] = '\0';

    printf("Iniciando remocao recursiva do diretorio: %s\n", path);

    removeRecursivo(msg, response);
    int ret = rmdir(msg->pathName);
    printf("Retorno do rmdir final: %d\n", response->sizePathName);
    if (response->sizePathName != -1 && ret != -1)
    {
        char* newPath = dirname(msg->pathName);
        response->pathName[0] = '\0';
        strcpy(response->pathName, newPath);
        response->sizePathName = strlen(response->pathName);
    }
}


/* int main() {
    Message msg;
    DL_REP dlResponse;
    strcpy(msg.pathName, "../lab9");
    msg.sizePathName = strlen(msg.pathName);
    dlResponse.allfilesnames[0] = '\0';
    dlResponse.nrnames = 0;

    dl(&msg, &dlResponse);
    printf("Nomes arquivos: %s\n", dlResponse.allfilesnames);
    printf("Numero de nomes: %d\n", dlResponse.nrnames);

    for(int i = 0; i < dlResponse.nrnames; i++)
    {
        char temp[100];
        temp[0] = '\0';
        printf("Posicao Inicial: %d\n", dlResponse.posicoes[i].posInicial);
        printf("Posicao Final: %d\n", dlResponse.posicoes[i].posFinal);
        strncpy(temp, 
                &dlResponse.allfilesnames[dlResponse.posicoes[i].posInicial],
                dlResponse.posicoes[i].posFinal - dlResponse.posicoes[i].posInicial + 1);
        temp[dlResponse.posicoes[i].posFinal - dlResponse.posicoes[i].posInicial + 1] = '\0';
        printf("Nome %d: %s, ehDir: %d\n", i,
                temp,
               dlResponse.posicoes[i].ehSubdiretorio);
    }
    return 0;
} */

/* int main() {
    Message msg;
    Message rdResponse;

    strcpy(msg.pathName, "testfile.txt");
    msg.sizePathName = strlen(msg.pathName);
    msg.offset = 0;

    rd(&msg, &rdResponse);
    printf("Payload lido: %s\n", rdResponse.payload);

    msg.offset = 1;

    rd(&msg, &rdResponse);
    printf("Payload lido: %s\n", rdResponse.payload);

    msg.offset = 2;

    rd(&msg, &rdResponse);
    printf("Payload lido: %s\n", rdResponse.payload);

    msg.offset = 3;

    rd(&msg, &rdResponse);
    printf("Payload lido: %s\n", rdResponse.payload);

    msg.offset = 4;

    rd(&msg, &rdResponse);
    printf("Payload lido: %s\n", rdResponse.payload);

    return 0;
} */

int main() {
    Message msg;
    Message drResponse;

    strcpy(msg.pathName, "./testdir/subdir1");
    msg.sizePathName = strlen(msg.pathName);
    strcpy(msg.dirName, "subdir1");
    msg.sizeDirName = strlen(msg.dirName);

    dr(&msg, &drResponse);
    if (drResponse.sizePathName == -1) {
        printf("Erro ao remover o diretório.\n");
    } else {
        printf("Diretório removido com sucesso. Caminho: %s\n", drResponse.pathName);
    }

    return 0;
}

