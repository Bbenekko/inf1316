#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "teste.h"

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

        snprintf(aux, sizeof(aux), ".%s%s/%s", directory, dc_req->pathName, dc_req->dirName);
    }
    
    if (strlen(aux) >= sizeof(aux))
    {
        // path estourou o buffer ou snprintf inválido

        perror("ERRO: create_subDirectory - Path estourou o buffer\n");

        dc_rep->sizePathName = -1;
        return;
    }
    

    if(mkdir(aux, 0777) == 0) 
    {
        dc_rep->owner = dc_req->owner;

        printf("Criação de diretório bem sucedida!\n");

        strncpy(dc_rep->pathName, aux, sizeof(aux) - 1);
        dc_rep->pathName[sizeof(aux) - 1] = '\0'; // Garantir terminação

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
            // path estourou o buffer ou snprintf inválido

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

    // caso de deletar arquivo (é completamente diferente de limpar os primeiros 16 caracteres ????)
    if(wr_req->offset == 0 && strcmp(wr_req->payload, "") == 0)
    {
        if(remove(aux) != 0) // path relativo faltando  
        {
            // TODO erro ao deletar
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

    FILE *original;
    char flag_ja_inseriu = 0;
    int size_original;
    char ch;

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

    int current_size = ftell(original); // ftell retorna a posição atual
    printf("\n\n%d\n\n", current_size);

    if (current_size == -1L) {
        perror("ERRO: write_file - Erro ao obter o tamanho do arquivo (ftell)");
        fclose(original);
        wr_rep->offset = -1;
        return;
    }

    // Se o offset requisitado é maior que o tamanho atual, preenchemos
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
        // Após o loop, o ponteiro está AGORA no wr_req->offset
    }
    else
    {
        // Reposiciona o ponteiro para o offset do usuário para sobrescrever
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

    // 6. Finalização e Resposta
    fclose(original);

    wr_rep->owner = wr_req->owner;
    strncpy(wr_rep->pathName, aux, PATH_MAX - 1);
    wr_rep->pathName[PATH_MAX - 1] = '\0';
    wr_rep->sizePathName = (int)strlen(wr_rep->pathName);
    wr_rep->offset = wr_req->offset; // Retorna o offset original

    printf("write file bem sucedido. Escrito %zu bytes em offset %ld no arquivo %s\n", 
           written, wr_req->offset, aux);
}