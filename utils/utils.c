#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <errno.h>
#include <dirent.h>
#include <zlib.h>

#define THROW_UP 1
#define GOOD_BOY 0
#define SIZE_MINIMA_STRINGA 5
#define OFFSET_STRINGA 10

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

void compress_and_save(unsigned char* uncompressed_data, unsigned long uncompressed_len, const char *hash_value)
{
  // Creating the path
  char path[256] = ".irongit/objects/";
  strncat(path, hash_value, 2);
  if(!dir_exists(path)) create_dir(path);  // If the directory doesn't exist, it cretes it
  strcat(path, "/");
  strncat(path, hash_value + 2, 38);  // Path: .../b6/awdmoawkdkdlsmdlaldaòwdlòl (38 chars)

  unsigned long int compressed_len = compressBound(uncompressed_len);  // It calculates the MAX length of the compressed data will ever be
  unsigned char *compressed_data = allocateUString(compressed_len);
  
  if(compressed_data == NULL)
  {
    fprintf(stderr, "Error compressing data.");
    exit(THROW_UP);
  }
  
  z_stream stream = {0};  // Structure that traces the compression process (is better to set to 0 because we do not want personalized functions with zalloc ecc...)
    
  stream.avail_in = uncompressed_len;  // We say to the structure that we have this space
  stream.next_in = uncompressed_data;  // It will read from uncompressed_data
    
  stream.avail_out = compressed_len;  // The compressed file will have this space
  stream.next_out = compressed_data;  // We want the compressed data inside this variable compressed_daa

  if (deflateInit(&stream, Z_BEST_SPEED) != Z_OK)
  {  // Prepares the process (Z_BEST_SPEED is level 1, is very fast but compress less)
    fprintf(stderr, "Error initializing zlib.\n");
    free(compressed_data);
    exit(THROW_UP);
  }

  if (deflate(&stream, Z_FINISH) != Z_STREAM_END)
  {   // compressing (Z_FINISH says that we are giving all data at once and Z_STREAM_END is the success flag)
    fprintf(stderr, "Error compressing data.\n");
    deflateEnd(&stream);
    free(compressed_data);
    exit(THROW_UP);
  }
  
  unsigned long int final_compressed_len = stream.total_out;  // Tells us the actual size of the compressed data
  deflateEnd(&stream);  // Free the memory used to make the calculus

  FILE *file = fopen(path, "wb");  // Let's open a binary file
  fwrite(compressed_data, 1, final_compressed_len, file);  // and write the compressed data
  fclose(file);
  free(compressed_data);
}

unsigned char *decompress(char *path, size_t *len_out)
{
    if(file_exists(path))
    {
        unsigned char compressed_data[4096], buffer[4096], *uncompressed_data = NULL;
        z_stream stream = {0};
        int byte_read, len = 0;

        if (inflateInit(&stream) != Z_OK)  // Start the engine of Z_LIB
        {  // Prepares the process
            fprintf(stderr, "Error initializing zlib.\n");
            exit(THROW_UP);
        }

        FILE *fin = fopen(path, "rb");

        while((byte_read = fread(compressed_data, sizeof(char), 4096, fin)) > 0)
        {
            stream.avail_in = byte_read;  // How much data we want to decompress
            stream.next_in = compressed_data;  // What data are we uncompressing
            do
            {
                stream.avail_out = 4096;  // How much we can decompress
                stream.next_out = buffer;  // Where we want the decompressed data
                int status = inflate(&stream, Z_NO_FLUSH);  // decompressing data
                if (status == Z_DATA_ERROR || status == Z_MEM_ERROR) {  // If it gives error
                    inflateEnd(&stream);  // Free the memory allocated for the z_stream structure
                    free(uncompressed_data);
                    fclose(fin);
                    return NULL;
                }

                int uncompressed_byte = 4096 - stream.avail_out;  // Calculates the number of bytes we have decompressed
                if(reallocUString(&uncompressed_data, uncompressed_byte + len + 1)){
                    memcpy(uncompressed_data + len, buffer, uncompressed_byte);  // Concatenates the new uncompressed data
                    len += uncompressed_byte;  // Increases the length
                }
            } while (stream.avail_out == 0);
        }
        fclose(fin);
        inflateEnd(&stream);

        if(uncompressed_data != NULL)
        {
            uncompressed_data[len] = '\0';
        }
        if(len_out != NULL)
        {
            *len_out = len;
        }

        return uncompressed_data;
    }
    return NULL;
}

void nuke_directory(char *path)
{
#ifdef _WIN32
  char command[256];
  sprintf(command, "rmdir /s /q \"%s\"", path);
  system(command); // Runs the Windows CMD command to force delete
#else
  char command[256];
  sprintf(command, "rm -rf \"%s\"", path);
  system(command); // Runs the Unix terminal command to force delete
#endif
}

int file_exists(char *path)
{
    FILE *ptr = fopen(path, "r");
    if(ptr != NULL)
    {
        fclose(ptr);
        return 1;
    }
    return 0;
}

int dir_exists(char *dir_name)
{
  DIR *directory = opendir(dir_name);
  if(directory)
  {
    closedir(directory);
    return 1;
  }
  if(ENOENT == errno)
  {  // When open(), read(), exec() fail errno is set to ENOENT (ERROR: No Entry)
      return 0;
  }
  return 0;  // It failed for other reasons (understood from StackOverflow...)
}

int create_dir(char *dir_name){
  if(!CreateDirectory(dir_name, NULL))
  {
    printf("Error creating the directory.");
    return 0;
  }
  if(!SetFileAttributes(dir_name, FILE_ATTRIBUTE_HIDDEN))
  {
      printf("Error to set hidden attribute.");
      return 0;
  }
  return 1;
}

unsigned long long int get_file_size(FILE *file)
{
  if(fseek(file, 0, SEEK_END) != 0) return 0;  // error
  long len = ftell(file);
  if(len == -1L) return 0;  // error
  rewind(file);
  return (unsigned long long int)len;
}

char *allocateString(size_t size)
{
  return (char *) calloc(size, sizeof(char));
}

unsigned char *allocateUString(size_t size)
{
    return (unsigned char *) calloc(size, sizeof(unsigned char));
}

int reallocString(char **s, size_t size_nuova)
{
  char *tmp = (char *)realloc(*s, sizeof(char) * size_nuova);  // Rialloco la stringa s con una nuova dimensione
  if(tmp != NULL)
  {  // Se l'operazione e' andata a buon fine realloc resituisce il puntatore alla nuova stringa
    *s = tmp;  // Quindi possiamo tranquillamente puntare a quello locazione di memoria
    return 1;
  }
  return 0;
}

int reallocUString(unsigned char **s, size_t size_nuova)
{
    unsigned char *tmp = (unsigned char *)realloc(*s, sizeof(unsigned char) * size_nuova);  // Rialloco la stringa s con una nuova dimensione
    if(tmp != NULL)
    {  // Se l'operazione e' andata a buon fine realloc resituisce il puntatore alla nuova stringa
        *s = tmp;  // Quindi possiamo tranquillamente puntare a quello locazione di memoria
        return 1;
    }
    return 0;
}

char* ironWrite()
{
  int c;
  size_t i = 0, len = SIZE_MINIMA_STRINGA;
  char *s = allocateString(len);  // Alloco spazio minimo per la stringa

  if(s == NULL)
  {  // Controllo se la malloc e' andata a buon fine
    fprintf(stderr, "Error: function scrivi() does not work.\n");
    exit(1);
  }

  while((c = getchar()) != '\n' && c != EOF)
  {
    if(i >= len - 1)
    {
      len += OFFSET_STRINGA;
      if(!reallocString(&s, len))
      {  // Rialloco una nuova dimensione (+10 caratteri) per la stringa
	    fprintf(stderr, "Error: function scrivi() does not work.\n");
	    exit(1);
      }
    }
    s[i] = (char)c;
    i++;
  }
  s[i] = '\0';  // Chiudiamo la stringa
  return s;
}

char *returnFileType(char *text)
{
    if(strncmp(text, "blob", 4) == 0) return "blob";
    if(strncmp(text, "tree", 4) == 0) return "tree";
    if(strncmp(text, "commit", 6) == 0) return "commit";
    return NULL;
}

int returnTypeLen(char *type)
{
    if(strcmp(type, "blob") == 0) return 4;
    if(strcmp(type, "tree") == 0) return 4;
    if(strcmp(type, "commit") == 0) return 6;
    return 0;
}

int returnFileLen(char *text)
{
    return atoi(text);
}

typedef struct Node
{
    char name[256];
    struct Node *next;
}Node;
typedef Node *StringList;

StringList create_node()
{
    return (StringList)malloc(sizeof(Node));
}

void add_node(StringList *head, char *name)
{
    StringList newNode = create_node();
    strncpy(newNode->name, name, 255);
    newNode->name[255] = '\0';
    newNode->next = *head;
    *head = newNode;
}

void split_list(StringList source, StringList *front, StringList *back)
{
    if (source == NULL || source->next == NULL)
    {
        *front = source;
        *back = NULL;
        return;
    }
    StringList slow = source;
    StringList fast = source->next;
    while (fast != NULL)
    {
        fast = fast->next;
        if (fast != NULL)
        {
            slow = slow->next;
            fast = fast->next;
        }
    }
    *front = source;
    *back = slow->next;
    slow->next = NULL;
}

StringList merge_sorted(StringList a, StringList b)
{
    if (a == NULL) return b;
    if (b == NULL) return a;
    StringList result = NULL;
    if (strcmp(a->name, b->name) <= 0)
    {
        result = a;
        result->next = merge_sorted(a->next, b);
    } else
    {
        result = b;
        result->next = merge_sorted(a, b->next);
    }
    return result;
}

void merge_sort(StringList *headRef)
{
    StringList head = *headRef;
    if (head == NULL || head->next == NULL) return;
    StringList a;
    StringList b;
    split_list(head, &a, &b);
    merge_sort(&a);
    merge_sort(&b);
    *headRef = merge_sorted(a, b);
}