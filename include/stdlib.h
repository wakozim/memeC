#ifndef STDLIB_H_
#define STDLIB_H_
#define RAND_MAX 2147483647
#define NULL ((void *) 0)
typedef unsigned long size_t;

int rand(void);
void srand(unsigned int seed);
void *_Nullable malloc(size_t size);
void free(void *_Nullable ptr);
void *_Nullable calloc(size_t n, size_t size);
void *_Nullable realloc(void *_Nullable ptr, size_t size);
void *_Nullable reallocarray(void *_Nullable ptr, size_t n, size_t size);

#endif // STDLIB_H_
