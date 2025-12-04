#include "trabalho.h"

void verifyPath(char path[], int sizePath, int owner)
{
    char temp[100];
 
    if (path[2] == 'A' && path[3] == '0' && path[4] == '/')
    {
         // Monta o caminho correto em temp
        snprintf(temp, sizeof(temp), "./root/%s", path + 2);

        // Copia já com '\0'
        strcpy(path, temp);

        return;
    }
    
    snprintf(temp, sizeof(temp), ROOT_DIRECTORY"/A%d/%s", owner, path + 2);

    strcpy(path, temp);

    printf("Caminho final: %s\n", path);
}

// operacoes em arquivos

void read_directory(Message* msg, Message* response)
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

    //copia o ./ para o path
    strncpy(path, msg->pathName, msg->sizePathName);

    //verifica se tem a0 ou nao
    verifyPath(msg->pathName, msg->sizePathName, msg->owner);

    /* strncpy(msg->pathName, path, strlen(path));*/
    msg->sizePathName = strlen(msg->pathName); 

    dl(msg, response);
}

void removeRecursivo(Message* msg, Message* response)
{    
    DIR *dir;
    struct dirent *entry;

    char path[100];

    strncpy(path, msg->pathName, msg->sizePathName);
    path[msg->sizePathName] = '\0';

    dir = opendir(path);
    if (!dir) {
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
    
    //coloca o nome do diretorio a ser removido no path
    strncat(path, msg->dirName, msg->sizeDirName);
    path[strlen(path)] = '\0';

    strcpy(msg->pathName, path);
    msg->sizePathName = strlen(msg->pathName);

    removeRecursivo(msg, response);
    int ret = rmdir(path);
    if (response->sizePathName != -1 && ret != -1)
    {
        char* newPath = dirname(msg->pathName);
        response->pathName[0] = '\0';
        strcpy(response->pathName, newPath);
        response->sizePathName = strlen(response->pathName);
    }
}

int temA0(char* string)
{
    if(strncmp("/A0", string, 3) == 0)
    {
        return 1; // possui A0
    }
    else return 0; // nao possui A0
}

void create_subDirectory(Message* dc_req, Message* dc_rep)
{
    char directory[4];
    char aux[PATH_MAX];

    if (temA0(dc_req->pathName))
    {
        snprintf(aux, sizeof(aux), "%s/%s", dc_req->pathName, dc_req->dirName);
    }
    else
    {
        snprintf(directory, 4, "/A%d", dc_req->owner);

        snprintf(aux, sizeof(aux), ROOT_DIRECTORY"%s%s/%s", directory, dc_req->pathName, dc_req->dirName);
    }
    
    if (strlen(aux) >= sizeof(aux))
    {
        perror("ERRO: create_subDirectory - Path estourou o buffer\n");

        dc_rep->sizePathName = -1;
        return;
    }
    

    if(mkdir(aux, 0777) == 0) 
    {
        dc_rep->owner = dc_req->owner;

        printf("Criação de diretório bem sucedida!\n");

        strncpy(dc_rep->pathName, aux, sizeof(aux) - 1);
        dc_rep->pathName[sizeof(aux) - 1] = '\0'; 

        dc_rep->sizePathName = (int)strlen(aux);

        return;
    }
    else
    {
        // erro de criação de diretório
        dc_rep->sizePathName = -1;

        perror("ERRO: create_subdirectory - Erro na criação de diretório!\n");
        printf("\nPath: %s\n\n", aux);

        return;
    }
}

void write_file(Message* wr_req, Message* wr_rep)
{
    char aux[PATH_MAX];
    FILE *original;

    if (temA0(wr_req->pathName))
    {
        strncpy(aux, wr_req->pathName, sizeof(aux) -1);
        aux[sizeof(aux) - 1] = '\0';
    }
    else
    {
        char directory[4];
        snprintf(directory, sizeof(directory), "/A%d", wr_req->owner);

        int n = snprintf(aux, sizeof(aux), ".%s/%s", directory, wr_req->pathName);

        if (n > sizeof(aux) || n < 0 ) 
        {
            perror("ERRO: write_file - Path estourou o buffer ou snprintf invalido");

            wr_rep->offset = -1;
            return;
        }
    }

    // checa offset
    if(wr_req->offset % 16 != 0)
    {
        perror("ERRO: write_file - Offset inválido, não é múltiplo de 16");
        wr_rep->offset = -1;
        return;
    }

    if(wr_req->offset == 0 && strcmp(wr_req->payload, "") == 0)
    {
        if(remove(aux) != 0) // path relativo faltando  
        {
            perror("ERRO: write_file - erro ao deletar arquivo!");
            wr_rep->offset = -1;
            return;
        }

        printf("Arquivo deletado com sucesso!\n");

        wr_rep->owner = wr_req->owner;
        strcpy(wr_rep->pathName, ""); // retorna path vazio
        wr_rep->sizePathName = 0;
        strcpy(wr_rep->payload, "");
        wr_rep->offset = 0;

        return;
    }

    if ((original = fopen(aux, "r+b")) == NULL) 
    {
        // Se o arquivo não existe, tenta criar para leitura/escrita. O modo 'w+b' cria e trunca.
        if ((original = fopen(aux, "w+b")) == NULL)
        {
            wr_rep->offset = -1;
            perror("ERRO: write_file - Erro ao abrir/criar arquivo\n");

            printf("\nPath: %s\n\n", aux);

            return;
        }
    }

    if (fseek(original, 0, SEEK_END) != 0) {
        perror("ERRO: write_file - Erro ao posicionar ponteiro (SEEK_END)\n");
        fclose(original);
        wr_rep->offset = -1;
        return;
    }

    int current_size = ftell(original); 
    //printf("\n\n%d\n\n", current_size);

    if (current_size == -1L) {
        perror("ERRO: write_file - Erro ao obter o tamanho do arquivo (ftell)");
        fclose(original);
        wr_rep->offset = -1;
        return;
    }

    if (wr_req->offset > current_size) {
        int bytes_to_fill = (int)wr_req->offset - current_size;

        // Caractere de preenchimento: espaço em branco (' ')
        char fill_char = ' '; 
        
        printf("Preenchendo %ld bytes com espaços (' ') antes da escrita.\n", bytes_to_fill);
        
        // Escreve o espaço byte a byte
        for (long i = 0; i < bytes_to_fill; i++) {
            if (fwrite(&fill_char, 1, 1, original) != 1) {
                perror("ERRO: write_file - Erro ao preencher vazio com espaços");
                fclose(original);
                wr_rep->offset = -1;
                return;
            }
        }
    }
    else
    {
        if (fseek(original, wr_req->offset, SEEK_SET) != 0) {
            perror("ERRO: write_file - Erro ao posicionar ponteiro (fseek SEEK_SET)");
            fclose(original);
            wr_rep->offset = -1;
            return;
        }
    }

    size_t payload_len = strlen(wr_req->payload);
    size_t written = fwrite(wr_req->payload, 1, payload_len, original); 

    if (written != payload_len) {
        perror("ERRO: write_file - Erro durante a escrita (fwrite)");
        fclose(original);
        wr_rep->offset = -1;
        return;
    }

    fclose(original);

    wr_rep->owner = wr_req->owner;
    strncpy(wr_rep->pathName, aux, PATH_MAX - 1);
    wr_rep->pathName[PATH_MAX - 1] = '\0';
    wr_rep->sizePathName = (int)strlen(wr_rep->pathName);
    wr_rep->offset = wr_req->offset; 

    printf("write file bem sucedido. Escrito %zu bytes em offset %ld no arquivo %s\n", 
           written, wr_req->offset, aux);
}