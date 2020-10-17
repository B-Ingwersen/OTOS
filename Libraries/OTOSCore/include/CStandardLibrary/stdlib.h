#ifndef OTOS_CORE___C_STANDARD_LIBRARY___STDLIB_H
#define OTOS_CORE___C_STANDARD_LIBRARY___STDLIB_H

#define RAND_MAX 32767

#define NULL ((void *)0)
typedef unsigned long int size_t;

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long int quot;
    long int rem;
} ldiv_t;

extern unsigned long int stdlib_RandomNextPointer;

double strtod(char * string, char ** endPointer);

long int strtol(char * string, char ** endPointer);

unsigned long int strtoul(char * string, char ** endPointer);

float atof(char * string);

int atoi(char * string);

long int atol(char * string);

int abs(int n);

div_t div(int numerator, int denominator);

long int labs(long int n);

ldiv_t ldiv(long int numerator, long int denominator);

int rand_NextValuePointer(unsigned long int * next);

void srand_NextValuePointer(unsigned int seed, unsigned long int * next);

int rand();

void srand(unsigned int seed);

void * malloc(size_t size);

void free(void * pointer);

void * calloc(size_t nItems, size_t size);

void * realloc(void * pointer, size_t size);

#endif
