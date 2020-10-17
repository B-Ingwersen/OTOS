#include "CStandardLibrary/string.h"

extern void * _OTOSCore_LoadLocation;

void * memchr(const void * string, int c, size_t n) {
	void * (*func)(const void *,int,size_t) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 392);
	return func(string,c,n);
}

int memcmp(const void * string1, const void * string2, size_t n) {
	int (*func)(const void *,const void *,size_t) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 508);
	return func(string1,string2,n);
}

void * memcpy ( void * restrict dest, const void * restrict source, size_t num ) {
	void * (*func)(void * restrict,const void * restrict,size_t) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 204);
	return func(dest,source,num);
}

void * memcpy_reverse ( void * restrict dest, const void * restrict source, size_t num ) {
	void * (*func)(void * restrict,const void * restrict,size_t) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 576);
	return func(dest,source,num);
}

void * memmove(void * dest, const void * source, size_t n) {
	void * (*func)(void *,const void *,size_t) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 144);
	return func(dest,source,n);
}

void * memset(void * string, int c, size_t n) {
	void * (*func)(void *,int,size_t) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 532);
	return func(string,c,n);
}

char * strcpy(char * restrict dest, char * restrict source) {
	char * (*func)(char * restrict,char * restrict) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 92);
	return func(dest,source);
}

char * strncpy(char * restrict dest, char * restrict source, size_t n) {
	char * (*func)(char * restrict,char * restrict,size_t) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 472);
	return func(dest,source,n);
}

size_t strlen( const char * string ) {
	size_t (*func)(const char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 696);
	return func(string);
}

char * strcat(char * dest, const char * source) {
	char * (*func)(char *,const char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 356);
	return func(dest,source);
}

char * strncat(char * dest, const char * source, size_t n) {
	char * (*func)(char *,const char *,size_t) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 524);
	return func(dest,source,n);
}

char * strchr(const char * string, int c) {
	char * (*func)(const char *,int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 712);
	return func(string,c);
}

int strcmp(const char * string1,  const char * string2) {
	int (*func)(const char *,const char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 560);
	return func(string1,string2);
}

int strncmp(const char * string1,  const char * string2, size_t n) {
	int (*func)(const char *,const char *,size_t) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 452);
	return func(string1,string2,n);
}

size_t strspn(const char * dest, const char * source) {
	size_t (*func)(const char *,const char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 688);
	return func(dest,source);
}

size_t strcspn(const char * dest, const char * source) {
	size_t (*func)(const char *,const char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 580);
	return func(dest,source);
}

char * strpbrk(const char * dest, const char * breakset) {
	char * (*func)(const char *,const char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 756);
	return func(dest,breakset);
}

char * strrchr(const char * string, int ch) {
	char * (*func)(const char *,int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 340);
	return func(string,ch);
}

char * strstr(const char * string, const char * substring) {
	char * (*func)(const char *,const char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 420);
	return func(string,substring);
}

char * strtok_withStorage(char * restrict string, const char * restrict deliminators, char ** strtok_storage) {
	char * (*func)(char * restrict,const char * restrict,char **) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 156);
	return func(string,deliminators,strtok_storage);
}

