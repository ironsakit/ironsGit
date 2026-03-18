#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <errno.h>
#include <dirent.h>
#include <zlib.h>

#define THROW_UP 1
#define GOOD_BOY 0
#define SIZE_MINIMA_STRINGA 5
#define OFFSET_STRINGA 10

void compress_and_save(unsigned char* uncompressed_data, unsigned long uncompressed_len, const char *path){
  unsigned long int compressed_len = compressBound(uncompressed_len);
  unsigned char *compressed_data = (unsigned char*) malloc(compressed_len);
  
  if(compressed_data == NULL){
    fprintf(stderr, "Error compressing data.");
    exit(THROW_UP);
  }
  
  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
    
  stream.avail_in = uncompressed_len;
  stream.next_in = uncompressed_data; // Cast necessario perché zlib si aspetta dati non-const
    
  stream.avail_out = compressed_len;
  stream.next_out = compressed_data;
    
  // level 1 of zlib
  if (deflateInit(&stream, Z_BEST_SPEED) != Z_OK) {
    fprintf(stderr, "Error initializing zlib.\n");
    free(compressed_data);
    exit(THROW_UP);
  }
    
  // compressing
  if (deflate(&stream, Z_FINISH) != Z_STREAM_END) {
    fprintf(stderr, "Error compressing data.\n");
    deflateEnd(&stream);
    free(compressed_data);
    exit(THROW_UP);
  }
  
  unsigned long int final_compressed_len = stream.total_out;
  deflateEnd(&stream);

  FILE *file = fopen(path, "wb");
  fwrite(compressed_data, 1, final_compressed_len, file);
  fclose(file);
  free(compressed_data);
}

// Cross-platform compatibility
void nuke_directory(const char *path) {
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

int dir_exists(char *dir_name){
  DIR *directory = opendir(dir_name);
  if(directory){
    closedir(directory);
    return 1;
  }else if(ENOENT == errno){  // When open(), read(), exec() fail errno is set to ENOENT (ERROR: No Entry)
    return 0;
  }else{
    return 0;  // It failed for other reasons (understood from StackOverflow...)
  }
}

int create_dir(char *dir_name){
  if(!CreateDirectory(dir_name, NULL)){
    printf("Error creating the directory.");
    return 0;
  }else{
    if(!SetFileAttributes(dir_name, FILE_ATTRIBUTE_HIDDEN)){
      printf("Error to set hidden attribute.");
      return 0;
    }
    return 1;
  }
}

int file_exists(char *path){
  FILE *ptr = fopen(path, "r");
  if(ptr != NULL){
    fclose(ptr);
    return 1;
  }else{
    return 0;
  }
}

unsigned long long int get_file_size(FILE *file){
  if(fseek(file, 0, SEEK_END) != 0) return 0;  // error
  long len = ftell(file);
  if(len == -1L) return 0;  // error
  rewind(file);
  return (unsigned long long int)len;
}

char *alloca_Stringa(size_t size){
  return (char *) calloc(size, sizeof(char));
}

int rialloca_stringa(char **s, size_t size_nuova){
  char *tmp = (char *)realloc(*s, sizeof(char) * size_nuova);  // Rialloco la stringa s con una nuova dimensione
  if(tmp != NULL){  // Se l'operazione e' andata a buon fine realloc resituisce il puntatore alla nuova stringa
    *s = tmp;  // Quindi possiamo tranquillamente puntare a quello locazione di memoria
    return 1;
  }
  return 0;
}

char* scrivi(){
  int c;
  size_t i = 0, len = SIZE_MINIMA_STRINGA;
  char *s = alloca_Stringa(len);  // Alloco spazio minimo per la stringa

  if(s == NULL){  // Controllo se la malloc e' andata a buon fine
    fprintf(stderr, "Error: function scrivi() does not work.\n");
    exit(1);
  }

  while((c = getchar()) != '\n' && c != EOF){
    if(i >= len - 1){
      len += OFFSET_STRINGA;
      if(!rialloca_stringa(&s, len)){  // Rialloco una nuova dimensione (+10 caratteri) per la stringa
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
