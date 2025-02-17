#ifndef STDIO_H_
#define STDIO_H_
#include <bits/types/FILE.h> 
#define NULL ((void *) 0)
#define SEEK_SET 0
#define SEEK_END 2

/* Standard streams.  */
extern FILE *stdin;		/* Standard input stream.  */
extern FILE *stdout;		/* Standard output stream.  */
extern FILE *stderr;		/* Standard error output stream.  */
/* C89/C99 say they're macros.  Make them happy.  */
#define stdin stdin
#define stdout stdout
#define stderr stderr

typedef unsigned long size_t;
int printf(const char *restrict format, ...);
int fprintf(FILE *restrict stream, const char *restrict format, ...);
FILE *fopen(const char *filename, const char *mode);
size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
int fclose(FILE *stream);
#endif // STDIO_H_
