#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <winsock2.h>

#define CONTINUE 1
#define STOP 0
#define DIM_BLOCK 64  // 64 byte = 512 bit
#define DIM_PADDING 8 // 8 byte = 64 bit
#define DIM_MAX 56  // 56 byte = 448 bit
#define DIM_WORDS 16
#define DIM_WORDS_CREATED 80
#define SHA_1_LEN 41
#define LEFT_ROTATE(value, bits) (((unsigned int)(value) <<(bits)) | ((unsigned int)(value) >> (32 - (bits))))  // Fast macro implementation (cool idea) to manipulate the left circular ratation

typedef struct Sha_struct Sha_struct;

struct Sha_struct  // This struct will help
{
  unsigned char buffer[DIM_BLOCK];  // 64 byte = 512 bit
  unsigned long long int byteread;  // Number of byte read
  unsigned int sha_state[5]; // Sha state (H1, H2, H3, H4, H5)
  void (*change_sha_state)(Sha_struct *s, unsigned a, unsigned int b, unsigned int c, unsigned int d, unsigned int e);
};

void change_sha_state(Sha_struct *s, unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int e)
{
  s->sha_state[0] += a;
  s->sha_state[1] += b;
  s->sha_state[2] += c;
  s->sha_state[3] += d;
  s->sha_state[4] += e;
}

int create_padding(Sha_struct *sha_struct, char *s, long long int len, unsigned long long int total_len)
{
  if(len >= DIM_BLOCK)
  {
    memcpy(sha_struct->buffer, s, DIM_BLOCK);
    return CONTINUE;  // We need more blocks
  }
  if(len >= DIM_MAX)
  {
    memcpy(sha_struct->buffer, s, len);
    sha_struct->buffer[len] = 0x80;  // 10000000
    memset(sha_struct->buffer + len + 1, 0x00, DIM_BLOCK - len - 1);
    return CONTINUE;  // We need more blocks
  }
  memset(sha_struct->buffer, 0x00, DIM_BLOCK);  // Initialize to 0x00 (0) the buffer (in case we are in the case that len is negative)
  if(len >= 0)
  {
    memcpy(sha_struct->buffer, s, len);
    sha_struct->buffer[len] = 0x80; // We need a 0x80 (1) before the set of 0x00 (0)
  }
  total_len*=8;  // Calculate the bit dimension
  int offset = 56;
  int pos = DIM_BLOCK - DIM_PADDING;  // Let's start from 56 (64-56 = 8 byte = 64 bit)
  while(offset >= 0)
  {
    sha_struct->buffer[pos] = (total_len >> offset) & 0xFF;  // it starts from the most significant byte and ends with least significant (& 0xFF is best practice but C does this automatically)
    offset-=8;
    pos++;
  }
  return STOP;
}
/* Initial SHA_STATE taken from https://rst.im/magic/sha1 */
void initializeMyShaStruct(Sha_struct *s)
{
  memset(s, 0, sizeof(Sha_struct));
  s->byteread = 0;
  s->sha_state[0] = 0x67452301;  // Conta in avanti
  s->sha_state[1] = 0xEFCDAB89;  // Continua a contare fino a F
  s->sha_state[2] = 0x98BADCFE;  // Conta all'indietro
  s->sha_state[3] = 0x10325476;  // Conta fino a 0
  s->sha_state[4] = 0xC3D2E1F0;  // BOOOOHH
  s->change_sha_state = change_sha_state;  // Assigning the function
}

unsigned int *create_16_words(const Sha_struct *s)
{
  /* Cretes my 16 words array (4 bytes each) */
  unsigned int *w = (unsigned int*) calloc(DIM_WORDS, sizeof(unsigned int));
  int j = 0;
  /* Using bitwise operation to fit perfectly each unsigned char (1 bytes) inside of unsigned int (4 bytes)*/
  for(int i = 0; i < DIM_WORDS; i++)
  {  // Big Endian
    w[i] |= (unsigned int)s->buffer[j] << 24;  // Most significant  ( 11111111 | 00000000 | 00000000 | 00000000 )
    w[i] |= (unsigned int)s->buffer[j+1] << 16;   // ( 00000000 | 11111111 | 00000000 | 00000000 )
    w[i] |= (unsigned int)s->buffer[j+2] << 8;    // // ( 00000000 | 00000000 | 11111111 | 00000000 )
    w[i] |= (unsigned int)s->buffer[j+3];  // Least significant ( 00000000 | 00000000 | 00000000 | 11111111 )
    j+=4;
  }
  return w;
}

void create_80_words_from_16_words(unsigned int **words_16)
{
  unsigned int *new = realloc(*words_16, DIM_WORDS_CREATED * sizeof(unsigned int));
  if(new != NULL)
  {
    *words_16 = new;
  }else
  {
    fprintf(stderr, "ERROR: REALLOC FAILD.\n");
    free(*words_16);
    exit(1);
  }
  unsigned int X = 0;
  for(int i = 16; i < DIM_WORDS_CREATED; i++)
  {
    X = (*words_16)[i - 3] ^ (*words_16)[i - 8] ^ (*words_16)[i - 14] ^ (*words_16)[i - 16];  // New combination through XOR operations
    /* Circular rotation */
    (*words_16)[i] = LEFT_ROTATE(X, 1);
  }
}
/* Magic number for signatures taken from: https://rst.im/magic/sha1 */
unsigned int magic_function(unsigned int b, unsigned int c, unsigned int d, int i, unsigned int *k)
{
  if(i <= 19){
    *k = 0x5A827999;
    return (b & c) | ((~b) & d);
  }
  if(i <= 39){
    *k = 0x6ED9EBA1;
    return b ^ c ^ d;
  }
  if(i <= 59){
    *k = 0x8F1BBCDC;
    return (b & c) | (b & d) | (c & d);
  }
  *k = 0xCA62C1D6;
  return b ^ c ^ d;
}

void exchange(unsigned int *a, unsigned int *b, unsigned int *c, unsigned int *d, unsigned int *e, const unsigned int *TMP)
{
  *e = *d;
  *d = *c;
  *c = LEFT_ROTATE(*b ,30);  // Circular left rotation
  *b = *a;
  *a = *TMP;
}

void blender(Sha_struct *s, const unsigned int *words_80)
{
  unsigned int a = s->sha_state[0], b = s->sha_state[1], c = s->sha_state[2], d = s->sha_state[3], e = s->sha_state[4];  // Creating a copy of my sha state
  unsigned int TMP = 0;
  unsigned int k = 0;
  for(int i = 0; i < DIM_WORDS_CREATED; i++)
  {
    TMP = magic_function(b, c, d, i, &k);
    TMP += LEFT_ROTATE(a, 5) + e + words_80[i] + k;  // This combination will result in overflow (it's what we want)
    exchange(&a, &b, &c, &d, &e, &TMP);  // Circular Exchange
  }
  s->change_sha_state(s, a, b, c, d, e);  // Changing the sha_state after 80 rounds
}

char * SHA_1(char *string, const unsigned long long int total_len, unsigned char raw_sha1[20])
{
  Sha_struct myBuffer;
  initializeMyShaStruct(&myBuffer);
  unsigned int *words_16;
  int flag = CONTINUE;
  
  while(flag != STOP)
  {
    flag = create_padding(&myBuffer,string + myBuffer.byteread, (long long int)total_len - myBuffer.byteread, total_len);
    words_16 = create_16_words(&myBuffer);  // Creating 16 words from my buffer
    create_80_words_from_16_words(&words_16);  // creating new 80 words from 16 words
    blender(&myBuffer, words_16);
    free(words_16);
    myBuffer.byteread += DIM_BLOCK;
  }
  char *SHA_1_string = (char *) calloc(SHA_1_LEN, sizeof(char));
  /* Concatenates the sha_states creating the hashed string (40 characters in EXADECIMAL) */
  sprintf(SHA_1_string, "%08x%08x%08x%08x%08x", myBuffer.sha_state[0], myBuffer.sha_state[1], myBuffer.sha_state[2], myBuffer.sha_state[3], myBuffer.sha_state[4]);

  if(raw_sha1 != NULL)
  {
      for(int i = 0; i < 5; i++)
      {
          myBuffer.sha_state[i] = htonl(myBuffer.sha_state[i]);  // My sha_state must be in Big Endian
      }
      memcpy(raw_sha1, myBuffer.sha_state, 20);   // I can copy raw byte from sha1_state
  }

  return SHA_1_string;
}