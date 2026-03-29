#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include <string.h>

#define THROW_UP 1
#define GOOD_BOY 0
#define MAX_DIR_SIZE 21  // Max size of name directory

void clean(char **s1, char **s2)
{
  free(*s1);
  free(*s2);
}

int init(char *main_path)
{
    char *path = malloc(strlen(main_path) + MAX_DIR_SIZE);
    char *main_path_tmp = malloc(strlen(main_path) + MAX_DIR_SIZE);
    int res = GOOD_BOY;

    strcpy(main_path_tmp, main_path);
    strcat(main_path_tmp, "/.ironGit");

    if(dir_exists(main_path_tmp)){ nuke_directory(main_path_tmp); res = 1; }

    if(!dir_exists(main_path_tmp)){
        if(!create_dir(main_path_tmp)){ clean(&path, &main_path_tmp); return THROW_UP; }

        strcpy(path, main_path_tmp);
        strcat(path, "/objects");
        if(!create_dir(path)){ clean(&path, &main_path_tmp); return THROW_UP; }

        strcpy(path, main_path_tmp);
        strcat(path, "/refs");
        if(!create_dir(path)){ clean(&path, &main_path_tmp); return THROW_UP; }

        strcat(path, "/heads");
        if(!create_dir(path)){ clean(&path, &main_path_tmp); return THROW_UP; }

        strcpy(path, main_path_tmp);
        strcat(path, "/HEAD.txt");
        FILE *ptr = fopen(path, "w");
        if(ptr == NULL){ clean(&path, &main_path_tmp); return THROW_UP; }
        fclose(ptr);
        clean(&path, &main_path_tmp);
        return res;
    }
    clean(&path, &main_path_tmp);
    return 2;  // <-- Very silly
}