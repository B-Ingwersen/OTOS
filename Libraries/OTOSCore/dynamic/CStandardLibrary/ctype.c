#include "CStandardLibrary/ctype.h"

extern void * _OTOSCore_LoadLocation;

int isalpha(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 520);
	return func(ch);
}

int islower(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 224);
	return func(ch);
}

int isupper(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 448);
	return func(ch);
}

int isdigit(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 628);
	return func(ch);
}

int isxdigit(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 312);
	return func(ch);
}

int iscntrol(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 480);
	return func(ch);
}

int isgraph(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 540);
	return func(ch);
}

int isspace(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 284);
	return func(ch);
}

int isblank(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 176);
	return func(ch);
}

int isprint(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 548);
	return func(ch);
}

int ispunct(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 276);
	return func(ch);
}

int tolower(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 228);
	return func(ch);
}

int toupper(int ch) {
	int (*func)(int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 700);
	return func(ch);
}

