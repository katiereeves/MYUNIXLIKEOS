/* Base standard, XSI extensions excluded */

#ifndef _STDIO_H_
#define _STDIO_H_

#include "stddef.h"
#include "sys/types.h"

typedef struct file_t{
    int flags; /* -r, -r, -rw... */
    int fileno;
    unsigned char *buf;
    unsigned char *bptr; /* file cursor */
    int len; /* buffer length in bytes */
    char free; /* flag for fclose */
    char iobf; /* io buffer flush: _IOFBF, _IOLBF, or _IONBF */
    char eof; /* flag if EOF is present */
} FILE;

#define _IOFBF 0x1 /* Input/output fully buffered. */
#define _IOLBF 0x2 /* Input/output line buffered. */
#define _IONBF 0x3 /* Input/output unbuffered. */

#define SEEK_CUR 0x1
#define SEEK_END 0x2
#define SEEK_SET 0x3

#define EOF -1

#define FILENAME_MAX 256 /* arbatrally set to _POSIX_PATH_MAX */ 
#define FOPEN_MAX 20
#define TMP_MAX 25

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

/* printing */
int putchar(int c);
int putc(int c, FILE *stream);
int getchar();
int getc(FILE *stream);
int printf(const char *restrict, ...);

/* file operations */
FILE* fopen(const char *path, const char *mode);
int fclose(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void rewind(FILE *stream);
ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);

static inline int feof(FILE *stream) { return stream ? (int)stream->eof : 1; }

#endif /* _STDIO_H_ */