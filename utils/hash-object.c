#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "sha1.h"

char *create_new_string(char *type, unsigned long long int *size, FILE *file){
  char buffer[256];
  int len_header = sprintf(buffer, "%s %llu", type, *size);
  
  char *result = calloc(len_header + *size + 1, sizeof(char));
  if(!result) return NULL;

  memcpy(result, buffer, len_header);

  result[len_header] = '\0';
  rewind(file);
  fread(result + len_header + 1, 1, *size, file);

  fclose(file);
  *size += len_header + 1;
  return result;
}

void hash_object(char *nameFile, char **hashed_string, unsigned long long int *final_size){
  if(file_exists(nameFile)){
    FILE *file = fopen(nameFile, "rb");
    *final_size = get_file_size(file);
    char *newString = create_new_string("blob", final_size, file);
    *hashed_string = SHA_1(newString, *final_size);
    free(newString);
  }
}
