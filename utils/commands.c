#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "init.h"
#include "hash-object.h"
#include <string.h>
#include "index.h"

#define COMMAND_OK 0
#define COMMAND_NOT_OK 1

int cmd_hash_object(int argc, char *argv[]);
int cmd_init(int argc, char *argv[]);
int cmd_cat_file(int argc, char *argv[]);
int cmd_add(int argc, char *argv[]);
int cmd_write_tree(int argc, char *argv[]);
int cmd_ls_tree(int argc, char *argv[]);

int cmd_hash_object(int argc, char *argv[])
{
  if(argc < 3)
  {
    printf("Usage: irongit hash-object <option> <file>\n");
    return COMMAND_NOT_OK;
  }
  char *option = NULL;
  char *hash_value = NULL;
  char *file_name = NULL;

  if(argc == 3)
  {
      file_name = argv[2];
  }
  if(argc == 4)
  {
      option = argv[2];
      file_name = argv[3]; // we are taking the file_name as the fourth argument from argv
  }

  if (file_exists(file_name))
  {
    unsigned long long int size = 0;
    char *name = file_name;
    hash_object(&name, &hash_value,NULL, &size);  // File_name will have the new header: blob size\0file_content
    printf("%s\n", hash_value); // Final hash
    if(option != NULL && option[1] == 'w')
    {  // it means we need to write the file
      compress_and_save((unsigned char*)name, size, hash_value);
    }
    free(file_name);
    free(hash_value);
  } else
  {
    fprintf(stderr, "Error: The file %s does not exist\n", file_name);
  }
  return COMMAND_OK;
}

int cmd_init(int argc, char *argv[])
{
  if(init(".") == COMMAND_OK)
  {
    printf("Initialized empty Git repository.\n");
  }else
  {
    printf("Reinitialized existing Git repository.\n");
  }
  return COMMAND_OK;
}

int cmd_cat_file(int argc, char *argv[]){
    if(argc < 4)
    {
        printf("Usage: irongit cat-file <option> <hash>\n");
        return COMMAND_NOT_OK;
    }
    char *option = argv[2];
    char *hash = argv[3];  // Takes the hash

    if(strlen(hash) < 40)
    {
        fprintf(stderr, "fatal: Not a valid object name \"%s\"", hash);
        exit(COMMAND_NOT_OK);
    }

    char path[59] = ".irongit/objects/";

    strncat(path, hash, 2);
    strcat(path, "/");
    strncat(path, hash+2, 38);
    path[58] = '\0';

    size_t len = 0;
    char *text = (char*) decompress(path, &len);
    if(text != NULL){
        switch (option[1])
        {
            case 'p':
                if(strncmp(text, "blob ", 5) == 0)
                {  // If it is a blob we can actually read its content
                    char *content = text + strlen(text) + 1; // It skips the header and the null byte (\0)
                    fwrite(content, 1, len - (strlen(text) + 1), stdout);  // In case we are printing text with '\0' inside this will never stops
                }
                break;
            case 't':
                printf("%s", returnFileType(text));
                break;
            case 's':
                printf("%d", returnFileLen(text + returnTypeLen(returnFileType(text))));  // es: text = "blob 5" with returnLen(text + 4) we get 5
                break;
            default:
                printf("Undefined command\n");
        }
        free(text);
    }else
    {
        printf("Fatal: Not a valid object name %s\n", hash);
        return COMMAND_NOT_OK;
    }
    return COMMAND_OK;
}

int cmd_add(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("Usage: irongit add <file>\n");
        return COMMAND_NOT_OK;
    }
    return add_index(argc, argv);
}

int cmd_write_tree(int argc, char *argv[])
{

    return COMMAND_OK;
}

int cmd_ls_tree(int argc, char *argv[]){

    return COMMAND_OK;
}