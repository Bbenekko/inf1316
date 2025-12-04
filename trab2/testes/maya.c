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


void verifyPath(char path[], int sizePath, int owner)
{
    char temp[100];

    printf("Caminho inicio: %s\n", path);

    // Só um exemplo, ajuste a condição se precisar
    printf("2 e 3 caractere: %c %c %c\n", path[2], path[3], path[4]);
    if (path[2] == 'A' && path[3] == '0' && path[4] == '/')
    {
         // Monta o caminho correto em temp
        snprintf(temp, sizeof(temp), "./root/%s", path + 2);

        // Copia já com '\0'
        strcpy(path, temp);

        printf("Caminho final: %s\n", path);
        return;
    }
    
    // Monta o caminho correto em temp
    snprintf(temp, sizeof(temp), "./root/A%d/%s", owner, path + 2);

    // Copia já com '\0'
    strcpy(path, temp);

    printf("Caminho final: %s\n", path);
}

// operacoes em arquivos

void rd(Message* msg, Message* response)
{
    FILE *file;
    char path[100];

    response->owner = msg->owner;

    strncpy(path, msg->pathName, msg->sizePathName);
    path[msg->sizePathName] = '\0';

    verifyPath(path, msg->sizePathName, msg->owner);

    file = fopen(path, "r");
    if (!file) {
        perror("arquivo não foi aberto\n");
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

    printf("Listando o diretorio: %s\n", path);

    dir = opendir(path);
    if (!dir) {
        perror(path);
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
            perror(pathname);
            response->nrnames = -1;
            continue;
        }

        if (S_ISDIR(st.st_mode)) 
        {
            printf("%s:\n", pathname);
            putNameInResponse(response, name, 1);
            Message newMsg;
            strcpy(newMsg.pathName, pathname); 
            newMsg.sizePathName = strlen(newMsg.pathName) + 1;  
            dl(&newMsg, response);
        } 
        else 
        {
            putNameInResponse(response, name, 0);
        }
    }
    closedir(dir);
}

void list_directory(Message* msg, DL_REP* response)
{ 
    char path[100];

    response->owner = msg->owner;
    response->allfilesnames[0] = '\0';
    response->nrnames = 0;

    printf("Listando o diretorio solicitado pelo usuario %s\n", msg->pathName);

    //copia o ./ para o path
    strncpy(path, msg->pathName, msg->sizePathName);

    //verifica se tem a0 ou nao
    verifyPath(msg->pathName, msg->sizePathName, msg->owner);
    printf("path do diretorio a ser listado: %s\n", msg->pathName);

    /* strncpy(msg->pathName, path, strlen(path));*/
    msg->sizePathName = strlen(msg->pathName); 

    dl(msg, response);
    printf("Numero de nomes listados: %d\n", response->nrnames);
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

void exclude_diretory(Message* msg, Message* response)
{ 
    char path[100];

    response->owner = msg->owner;
    response->sizePathName = 0;

    //copia o ./ para o path
    strncpy(path, msg->pathName, msg->sizePathName);

    //verifica se tem a0 ou nao
    verifyPath(path, msg->sizePathName, msg->owner);
    printf("nome diretorio: %s\n", msg->dirName);
    printf("path do diretorio a ser removido: %s\n", path);
    
    //coloca o nome do diretorio a ser removido no path
    strncat(path, msg->dirName, msg->sizeDirName);
    path[strlen(path)] = '\0';
    printf("path do diretorio a ser removido: %s\n", path);

    strcpy(msg->pathName, path);
    msg->sizePathName = strlen(msg->pathName);

    printf("Iniciando remocao recursiva do diretorio: %s\n", path);

    removeRecursivo(msg, response);
    printf("path do diretorio a ser removido: %s\n", msg->pathName);
    int ret = rmdir(path);
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
    strcpy(msg.pathName, "./A0/subdir11");
    msg.pathName[strlen(msg.pathName)] = '\0';
    printf("Caminho passado: %s\n", msg.pathName);

    msg.sizePathName = strlen(msg.pathName) + 1;

    msg.owner = 2;
    dlResponse.allfilesnames[0] = '\0';
    dlResponse.nrnames = 0;
    dlResponse.owner = msg.owner;

    list_directory(&msg, &dlResponse);
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

    strcpy(msg.pathName, "./A0/subdir11/subdir2/testfile.txt");
    msg.sizePathName = strlen(msg.pathName);
    msg.offset = 0;
    msg.owner = 2;

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

/* int main() {
    Message msg;
    Message drResponse;

    strcpy(msg.pathName, "./A0/");
    msg.sizePathName = strlen(msg.pathName);
    strcpy(msg.dirName, "subdir11");
    msg.sizeDirName = strlen(msg.dirName) + 1;
    msg.owner = 2;

    drResponse.pathName[0] = '\0';
    drResponse.sizePathName = -1;

    exclude_diretory(&msg, &drResponse);
    if (drResponse.sizePathName == -1) {
        printf("Erro ao remover o diretório.\n");
    } else {
        printf("Diretório removido com sucesso. Caminho: %s\n", drResponse.pathName);
    }

    return 0;
}
 */
