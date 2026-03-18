#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>

char* scrivi();
int file_exists(char *path);
void hash_object(char *nameFile, char **hashed_string, unsigned long long int *final_size);
unsigned long long int get_file_size(FILE *file);
void nuke_directory(const char *path);
int dir_exists(char *dir_name);
int create_dir(char *dir_name);
void compress_and_save( unsigned char* uncompressed_data, unsigned long uncompressed_len, const char *path);

#endif
