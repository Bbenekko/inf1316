#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"

void write_file(Message* wr_req, Message* wr_rep)
{
    // TODO função checa validade do path

    // TODO talvez tenhamos que adicionar um path relativo ou algo do tipo

    // checa offset
    if(wr_req->offset % 16 != 0)
    {
        // TODO printa erro 
        wr_rep->offset = -1;
        return;
    }

    // caso de deletar arquivo (é completamente diferente de limpar os primeiros 16 caracteres ????)
    if(wr_req->offset == 0 && strcmp(wr_req->payload, ""))
    {
        if(remove(wr_req->pathName)) // path relativo faltando  
        {
            // TODO erro ao deletar
            wr_rep->offset = -1;
            return;
        }
        // TODO retorno de select
    }

    FILE *original, *temporario;
    char flag_ja_inseriu = 0;
    int size_original;
    char ch;

    if ((original = fopen(wr_req->pathName, "r")) == NULL) {
        perror("Error opening original file");
        wr_rep->offset = -1;
        return;
    }

    // pegando tamanho
    fseek(original, 0, SEEK_END); // seek to end of file
    size_original = ftell(original); // get current file pointer
    fseek(original, 0, SEEK_SET);

    if(wr_req->payload < size_original)
    {
        if ((temporario = fopen("temp.txt", "a")) == NULL) 
        {
            perror("Error opening temporary file for writing");
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
        wr_rep->sizePathName = wr_req->pathName;
        strcpy(wr_rep->payload, "");
        wr_rep->offset = wr_req->offset;

        fclose(original);
        fclose(temporario);

        remove(wr_req->pathName); // remove o arquivo original
        rename("temp.txt", wr_req->pathName);

        return;

    }
    /*
    else if(wr_req->payload == size_original)
    {
        fclose(original);
        if ((original = fopen(wr_req->pathName, "w")) == NULL) 
        {
            perror("Error opening original file for appending");
            wr_rep->offset = -1;
            return;
        }

        fputs(wr_req->payload, original);

        fclose(original);

        wr_rep->owner = wr_req->owner;
        strcpy(wr_rep->pathName, wr_req->pathName);
        wr_rep->sizePathName = wr_req->pathName;
        strcpy(wr_rep->payload, "");
        wr_rep->offset = wr_req->offset;

        return;
    }
    */
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
        strcpy(wr_rep->pathName, wr_req->pathName);
        wr_rep->sizePathName = wr_req->pathName;
        strcpy(wr_rep->payload, "");
        wr_rep->offset = wr_req->offset;

        return;
    }
    
}