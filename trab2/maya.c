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


// operacoes em arquivos


//operacoes em diretorios

void dl_req(Message msg, char* allFilesAndDirNames)
{
    const char *path = msg.pathName;
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

        printf("%s:\n", name);
        strcat(allFilesAndDirNames, name);
    }

    closedir(dir);
}

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
            putNameInResponse(response, name, 1);
            Message newMsg;
            strcpy(newMsg.pathName, pathname); 
            dl(&newMsg, response);
        } 
        else 
        {
            putNameInResponse(response, name, 0);
        }
        
    }
    response->owner = msg->owner;
    closedir(dir);
}

int main() {
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
}

