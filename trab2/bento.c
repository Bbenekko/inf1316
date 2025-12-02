#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

//TODO            LÓGICA DO A0 e testagem com a main da maya
//TODO            LÓGICA DO A0 e testagem com a main da maya
//TODO            LÓGICA DO A0 e testagem com a main da maya


#include "structs.h"

void create_subDirectory(Message* dc_req, Message* dc_rep)
{

    char aux[PATH_MAX];

    int n = snprintf(aux, sizeof(aux), "%s/%s", dc_req->pathName, dc_req->dirName);

    if (n > sizeof(aux) || n < 0 ) 
    {
        // path estourou o buffer ou snprintf inválido

        perror("ERRO: create_subDirectory - Path estourou o buffer ou snprintf invalido");

        dc_rep->sizePathName = -1;
        return;
    }
    

    if(mkdir(aux, 0777) == 0) 
    {
        dc_rep->owner = dc_req->owner;

        strncpy(dc_rep->pathName, aux, sizeof(aux) - 1);
        dc_rep->pathName[sizeof(aux) - 1] = '\0'; // Garantir terminação

        dc_rep->sizePathName = (int)strlen(aux);

        return;
    }
    else
    {
        // erro de criação de diretório
        dc_rep->sizePathName = -1;

        perror("ERRO: create_subdirectory - Erro na criação de diretório!");

        return;
    }
}

void write_file(Message* wr_req, Message* wr_rep)
{

    // checa offset
    if(wr_req->offset % 16 != 0)
    {
        perror("ERRO: write_file - Offset inválido, não é múltiplo de 16");
        wr_rep->offset = -1;
        return;
    }

    // caso de deletar arquivo (é completamente diferente de limpar os primeiros 16 caracteres ????)
    if(wr_req->offset == 0 && strcmp(wr_req->payload, ""))
    {
        if(remove(wr_req->pathName)) // path relativo faltando  
        {
            // TODO erro ao deletar
            perror("ERRO: write_file - erro ao deletar arquivo!");
            wr_rep->offset = -1;
            return;
        }
        wr_rep->owner = wr_req->owner;
        strcpy(wr_rep->pathName, ""); // retorna path vazio
        wr_rep->sizePathName = 0;
        strcpy(wr_rep->payload, "");
        wr_rep->offset = wr_req->offset;

        return;
    }

    FILE *original, *temporario;
    char flag_ja_inseriu = 0;
    int size_original;
    char ch;

    if ((original = fopen(wr_req->pathName, "rw")) == NULL) 
    {
        perror("ERRO: write_file - Error opening original file");
        wr_rep->offset = -1;
        return;
    }

    // pegando tamanho
    fseek(original, 0, SEEK_END); // seek to end of file
    size_original = ftell(original); // get current file pointer
    fseek(original, 0, SEEK_SET);

    if(wr_req->offset < size_original)
    {
        if ((temporario = fopen("temp.txt", "a")) == NULL) 
        {
            perror("ERRO: write_file - Error opening temporary file for writing");
            wr_rep->offset = -1;
            return;
        }

        for(int i = 0; i < wr_req->payload; i++)
        {
            ch = fgetc(original);
            fputc(ch, temporario);
        }

        fputs(wr_req->payload, original);
        fseek(original, 16, SEEK_CUR);
       
        for (int i = ftell(original); i < size_original; i++)
        {
            ch = fgetc(original);
            fputc(ch, temporario);
        }

        wr_rep->owner = wr_req->owner;
        strcpy(wr_rep->pathName, wr_req->pathName);
        wr_rep->sizePathName = wr_req->sizePathName;
        strcpy(wr_rep->payload, "");
        wr_rep->offset = wr_req->offset;

        fclose(original);
        fclose(temporario);

        remove(wr_req->pathName); // remove o arquivo original
        rename("temp.txt", wr_req->pathName);

        return;

    }
       else // offset maior ou igual que o do arquivo
    {
        fclose(original);
        if ((original = fopen(wr_req->pathName, "a")) == NULL) 
        {
            perror("Error opening original file for appending");
            wr_rep->offset = -1;
            return;
        }

        int qtd_pular = size_original - wr_req->offset;

        for(int i = 0; i < qtd_pular; i += 16)
        {
            fputs(wr_req->payload, "                ");
        }

        fputs(wr_req->payload, original);

        fclose(original);

        wr_rep->owner = wr_req->owner;
        strncpy(wr_rep->pathName, wr_req->pathName, wr_rep->sizePathName - 1);
        wr_rep->pathName[wr_rep->sizePathName - 1] = '\0'; // Garantir terminação
        wr_rep->sizePathName = wr_req->sizePathName;
        strcpy(wr_rep->payload, "");
        wr_rep->offset = wr_req->offset;

        return;
    }
    
}