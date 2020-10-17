#include "CStandardLibrary/locale.h"

extern void * _OTOSCore_LoadLocation;

char * setlocale( int category, const char * locale) {
	char * (*func)(int,const char *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 592);
	return func(category,locale);
}

struct lconv * localeconv() {
	struct lconv * (*func)() = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 624);
	return func();
}

