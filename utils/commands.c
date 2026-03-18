#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "utils.h"
#include "init.h"
#define COMMAND_OK 0
#define COMMAND_NOT_OK 1

int cmd_hash_object(int argc, char *argv[], char *flag){
  if(argc < 3){
    printf("Uso: irongit hash-object <file>\n");
    return COMMAND_NOT_OK;
  }
  char *hash_value = NULL;
  char *file_name = argv[2];
  if(flag != NULL){
    file_name = argv[3];
  }
  
  if (file_exists(file_name)) {
    unsigned long long int size = 0;
    hash_object(file_name, &hash_value, &size);
    printf("%s\n", hash_value); // Final hash
    if(flag != NULL){
      char path[256] = ".irongit/objects/";
      strncat(path, hash_value, 2);
      create_dir(path);
      strcat(path, "/");
      strncat(path, hash_value + 2, 38);
      compress_and_save((unsigned char*)hash_value, size, path);
    }
    free(hash_value); 
  } else {
    fprintf(stderr, "Error: The file %s does not exist\n", argv[2]);
  }
  
  return COMMAND_OK;
}

int cmd_init(int argc, char *argv[], char *flag){
  if(init(".") == COMMAND_OK){
    printf("Initialized empty Git repository.\n");
  }else{
    printf("Reinitialized empty Git repository.\n");
  }
  return COMMAND_OK;
}
