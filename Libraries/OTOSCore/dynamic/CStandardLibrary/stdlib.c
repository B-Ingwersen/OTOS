#include "CStandardLibrary/stdlib.h"

extern void * _OTOSCore_LoadLocation;

double strtod(char * string, char ** endPointer) {
	double (*func)(char *,char **) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 344);
	return func(string,endPointer);
}

long int strtol(char * string, char ** endPointer) {
	long int (*func)(char *,char **) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 320);
	return func(string,endPointer);
}

unsigned long int strtoul(char * string, char ** endPointer) {
	unsigned long int (*func)(char *,char **) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 256);
	return func(string,endPointer);
}

float atof(char * string) {
	float (*func)(char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 352);
	return func(string);
}

int atoi(char * string) {
	int (*func)(char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 676);
	return func(string);
}

long int atol(char * string) {
	long int (*func)(char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 160);
	return func(string);
}

int abs(int n) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 708);
	return func(n);
}

div_t div(int numerator, int denominator) {
	div_t (*func)(int,int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 704);
	return func(numerator,denominator);
}

long int labs(long int n) {
	long int (*func)(long int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 180);
	return func(n);
}

ldiv_t ldiv(long int numerator, long int denominator) {
	ldiv_t (*func)(long int,long int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 608);
	return func(numerator,denominator);
}

int rand_NextValuePointer(unsigned long int * next) {
	int (*func)(unsigned long int *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 152);
	return func(next);
}

void srand_NextValuePointer(unsigned int seed, unsigned long int * next) {
	void (*func)(unsigned int,unsigned long int *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 432);
	func(seed,next);
}

