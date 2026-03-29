#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>

typedef struct Node
{
    char name[256];
    struct Node *next;
}Node;
typedef Node *StringList;

void compress_and_save(unsigned char* uncompressed_data, unsigned long uncompressed_len, const char *hash_value);
unsigned char *decompress(char *path, size_t *len_out);
int file_exists(char *path);
int dir_exists(char *dir_name);
int create_dir(char *dir_name);
void nuke_directory(char *path);
unsigned long long int get_file_size(FILE *file);
char *returnFileType(char *text);
int returnTypeLen(char *type);
int returnFileLen(char *text);
char *ironWrite();
char *allocateString(size_t size);
int reallocString(char **s, size_t size_nuova);
unsigned char *allocateUString(size_t size);
int reallocUString(unsigned char **s, size_t size_nuova);
void add_node(StringList *head, char *name);
void merge_sort(StringList *headRef);

#endif
