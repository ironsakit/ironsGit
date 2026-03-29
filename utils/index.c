#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include "sha1.h"
#include "utils.h"
#include <winsock2.h>
#include "hash-object.h"
#include <dirent.h>
#include <errno.h>

#define COMMAND_OK 0
#define COMMAND_NOT_OK 1

typedef struct header  // The header which tells info about the index file and the files written on it
{
    char signature[4];  // must contain always the word "DIRC"
    uint32_t version;  // Format version (we use the version 2)
    uint32_t entries;  // Total number of file added
}header;

typedef struct fileInfo  // Will contain file information to be store inside of index file and finally be hashed to sign
{
    uint32_t ctime_sec;  // Creation (Seconds)
    uint32_t ctime_nsec;  // Creation (nanoseconds)
    uint32_t mtime_sec;  // Modified (seconds)
    uint32_t mtime_nsec;  // Modified (nanoseconds)
    uint32_t dev;        // Devide ID
    uint32_t ino;    // inode number
    uint32_t mode;   // permissions
    uint32_t uid;  // User id
    uint32_t gid;  // Group id
    uint32_t size;   // Size of file
    unsigned char sha1[20];  // 20 byte of my sha1
    uint16_t flags;  // length of the name file
}fileInfo;

char *create_index(char *filename, FILE *index, int *offset);
void readList(StringList *head, int numFile);
int add_index(int argc, char *argv[]);
char *create_index(char *filename, FILE *index, int *offset);
void scand_dir(char *path, StringList *nameList, StringList *irongitignore, int *numFile);
void readIronGitIgnore(StringList *irongitignore);
int checkIronGitIgnore(StringList *irongitignore, char *filename);

void readIronGitIgnore(StringList *irongitignore)
{
    if (file_exists(".irongitignore.txt"))
    {
        FILE *ignore = fopen(".irongitignore.txt", "r");
        char filename[256];
        while (fgets(filename, 256, ignore) != NULL)
        {
            if (filename[strlen(filename) - 1] == '\n') filename[strlen(filename) - 1] = '\0';  // fgets takes the '\n' in this way we are deleting it
            add_node(irongitignore, filename);  // let's add to our list
        }
        fclose(ignore);
    }
}

void readList(StringList *head, int numFile)
{
    header hdr;
    memcpy(hdr.signature, "DIRC", 4);  // we dont need htonl because we are coping single bytes
    hdr.version = htonl(2);
    hdr.entries = htonl(numFile);

    FILE *index = fopen(".ironGit/index", "wb");
    fwrite(&hdr, 12, 1, index);

    int sizeBuffer = 12;  // bytes of header (hdr)
    char *buffer = malloc(sizeBuffer);
    memcpy(buffer, &hdr, 12);  // Let's copy the header in the buffer

    while(*head != NULL)
    {
        int offset = 0;
        char *bufferTMP = create_index((*head)->name, index, &offset);
        if(bufferTMP == NULL)
        {  // we are creating file structure for each file
            fprintf(stderr, "Error: impossible to add %s files.\n", (*head)->name);
        }else
        {
            sizeBuffer+=offset;  // Increasing the space required
            reallocString(&buffer, sizeBuffer);  // allocating new memory for new files
            memcpy(buffer + (sizeBuffer - offset), bufferTMP, offset);  // copy
            free(bufferTMP);
        }
        *head = (*head)->next;
    }
    unsigned char raw_sha1[20];
    SHA_1(buffer, sizeBuffer, raw_sha1);  // and create the Sha1 to sign everything
    fwrite(raw_sha1, 20, 1, index);  // write it down everything
    free(buffer);
    fclose(index);
}

int checkIronGitIgnore(StringList *irongitignore, char *filename)
{
    StringList nameList = *irongitignore;
    char *cleanName = filename;

    if (strncmp(cleanName, "./", 2) == 0) cleanName+=2;  // If it begins with "./" we can further

    while (nameList != NULL)
    {
        if (nameList->name[0] == '*')  // We want to exclude a family of files (*.txt, *.exe)
        {
            char *extension = nameList->name + 1;  // .exe .txt etc..
            size_t cleanNameLen = strlen(cleanName);
            size_t extensioneLen = strlen(extension);
            if (cleanNameLen >= extensioneLen && strcmp(cleanName + cleanNameLen - extensioneLen, extension) == 0)
            {
                return 1;  // Ignored
            }
        }else if (strcmp(nameList->name, cleanName) == 0)
        {
            return 1;  // Ignored
        }
        nameList = nameList->next;  // let's take next
    }
    return 0;  // Not ignored
}

void scand_dir(char *path, StringList *nameList, StringList *irongitignore, int *numFile)
{
    DIR *directory = opendir(path);
    if(directory){
        struct dirent *entry = NULL;
        while((entry = readdir(directory)) != NULL)
        {
            if(strcmp(entry->d_name, ".") != 0 &&
               strcmp(entry->d_name, "..") != 0 &&
               strcmp(entry->d_name, ".ironGit") != 0 &&
               strcmp(entry->d_name, ".git") != 0)
            {
                char fullpath[1024];
                if (strcmp(path, ".") == 0)
                {
                    snprintf(fullpath, 1024, "./%s", entry->d_name);
                }else
                {
                    snprintf(fullpath, 1024, "%s/%s", path, entry->d_name);
                }

                if (checkIronGitIgnore(irongitignore, fullpath) != 1)
                {
                    struct stat fileInfo;  // we check if this is file or not
                    if (stat(fullpath, &fileInfo) == 0)
                    {
                        if (S_ISDIR(fileInfo.st_mode))
                        {  // It's a directory
                            scand_dir(fullpath, nameList, irongitignore, numFile);  // recursive call
                        }else if (S_ISREG(fileInfo.st_mode))
                        {  // It's a file
                            add_node(nameList, fullpath);  // We finally got the file
                            (*numFile)++;
                        }
                    }
                }
            }
        }
        closedir(directory);
    }else if(ENOENT == errno)
    {
        fprintf(stderr, "Error: impossible to open the directory.\n");
    }
}

int add_index(int argc, char *argv[])
{
    StringList nameList = NULL, irongitignore = NULL;
    readIronGitIgnore(&irongitignore);  // Let's read the irongitignore if it exists
    int numFile = 0;  // irongit add file1 file2 file3  |argc = 5| |numFile = 5 -2|

    for (int i = 2; i < argc; i++)
    {
        if (checkIronGitIgnore(&irongitignore, argv[i]) != 1 || strcmp(argv[i], ".") == 0)  // The user wants to add a file that is not ignored
        {
            struct stat fileInfo;  // we check if this is file or not
            if (stat(argv[i], &fileInfo) == 0)
            {
                if (S_ISDIR(fileInfo.st_mode))
                {  // It's a directory
                    scand_dir(argv[i], &nameList, &irongitignore, &numFile);  // recursive call
                }else if (S_ISREG(fileInfo.st_mode))
                {  // It's a file
                    char cleanPath[256];
                    if (strncmp(argv[i], "./", 2) != 0)
                    {
                        snprintf(cleanPath, 256, "./%s", argv[i]);
                        add_node(&nameList, cleanPath);  // We finally got the file
                    }else
                    {
                        add_node(&nameList, argv[i]);  // We finally got the file
                    }
                    numFile++;
                }
            }else
            {
                fprintf(stderr, "Error: \"%s\" is impossible to find.\n", argv[i]);
            }
        }
    }

    if (numFile > 0)
    {
        merge_sort(&nameList);
        readList(&nameList, numFile);
    }else
    {
        fprintf(stderr, "No file added.\n");
    }

    return COMMAND_OK;
}

char *create_index(char *filename, FILE *index, int *offset)
{
    char *filenameTMP = filename;
    char *hash = NULL;
    size_t size = 0;
    unsigned char raw_sha1[20];
    hash_object(&filenameTMP, &hash, raw_sha1, &size);
    compress_and_save((unsigned char*)filenameTMP, size, hash);
    // Creted the hash and saved the compressed file

    free(hash);

    struct stat file;
    fileInfo file_info;

    if(stat(filename, &file) == 0)
    {  // It takes the file's METADATA
        // Temporal data
        file_info.ctime_sec = htonl(file.st_ctime);
        file_info.ctime_nsec = 0;  // in Windows we don't have nanoseconds :(
        file_info.mtime_sec = htonl(file.st_mtime);
        file_info.mtime_nsec = 0;
        // File system metadata
        file_info.dev = htonl(file.st_dev);
        file_info.ino = htonl(file.st_ino);
        file_info.uid = htonl(file.st_uid);
        file_info.gid = htonl(file.st_gid);
        file_info.size = htonl(file.st_size);

        file_info.mode = htonl(0100644);  // It's used 0100644 for normal file and 0100755 for executable
        memcpy(file_info.sha1, raw_sha1, 20);

        int name_len = strlen(filename);
        file_info.flags = htons(name_len & 0x0FFF);  // We use htons for 16 bits and the mask 0X0FFF because GIT uses the last 12 bits for length and the first 4 bits for flags

        if(index != NULL)
        {
            fwrite(&file_info, 62, 1, index);
            fwrite(filename, name_len, 1, index);
            int padding = 8 - ((62 + name_len) % 8);
            for(int i = 0; i < padding; i++)
            {
                fputc(0, index);
            }
            char *buffer = (char *)malloc(1024);   // Buffer which will contain all my structure just to be hashed by Sha1
            memset(buffer, 0, 1024);
            int n = 0;
            memcpy(buffer + n, &file_info, 62);
            n+=62;
            memcpy(buffer + n, filename, name_len);
            n+=name_len;
            memset(buffer + n, 0, padding);
            n+=padding;
            *offset = n;
            return buffer;
        }
        printf("Error: it's impossible to create index file");
        return NULL;
    }
    fprintf(stderr, "Error: it's impossible to recover file info.\n");
    return NULL;
}