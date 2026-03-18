#ifndef UTIL_H
#define UTIL_H

int SHA_1(char *string, char **SHA_1_string);
char* scrivi();
int file_exists(char *path);
void hash_object(char *nameFile, char **hashed_string, unsigned long long int *final_size);

#endif
