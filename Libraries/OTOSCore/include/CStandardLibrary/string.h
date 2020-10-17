#ifndef OTOS_CORE___C_STANDARD_LIBRARY___STRING_H
#define OTOS_CORE___C_STANDARD_LIBRARY___STRING_H

#define NULL ((void *)0)
typedef unsigned long int size_t;

void * memchr(const void * string, int c, size_t n);

int memcmp(const void * string1, const void * string2, size_t n);

void * memcpy ( void * restrict dest, const void * restrict source, size_t num );

void * memcpy_reverse ( void * restrict dest, const void * restrict source, size_t num );

void * memmove(void * dest, const void * source, size_t n);

void * memset(void * string, int c, size_t n);

char * strcpy(char * restrict dest, char * restrict source);

char * strncpy(char * restrict dest, char * restrict source, size_t n);

size_t strlen( const char * string );

char * strcat(char * dest, const char * source);

char * strncat(char * dest, const char * source, size_t n);

char * strchr(const char * string, int c);

int strcmp(const char * string1,  const char * string2);

int strncmp(const char * string1,  const char * string2, size_t n);

size_t strspn(const char * dest, const char * source);

size_t strcspn(const char * dest, const char * source);

char * strpbrk(const char * dest, const char * breakset);

char * strrchr(const char * string, int ch);

char * strstr(const char * string, const char * substring);

char * strtok_withStorage(char * restrict string, const char * restrict deliminators, char ** strtok_storage);

#endif
