#include "CStandardLibrary/string.h"
#include "Definitions.h"

void * memchr(const void * string, int c, size_t n) {
    unsigned char character = (unsigned char)c;

    unsigned int i;
    for (i = 0; i < n; i++) {
        if ( ((unsigned char*)string)[i] == character ) {
            return (void*)( (unsigned char*)string + i );
        }
    }
    return NULL;
}

int memcmp(const void * string1, const void * string2, size_t n) {
    unsigned char * s1 = (unsigned char*)string1;
    unsigned char * s2 = (unsigned char*)string2;

    unsigned int i;
    for (i = 0; i < n; i++) {
        if ( s1[i] != s2[i] ) {
            if ( s1[i] > s2[i] ) {
                return 1;
            }
            else {
                return -1;
            }
        }
    }
    return 0;
}

void * memcpy ( void * restrict dest, const void * restrict source, size_t num ) {
    asm volatile(
        "mov ecx, %0\n\t"\
        "mov esi, %1\n\t"\
        "mov edi, %2\n\t"\
        "cld        \n\t"\
        "rep movsb  \n\t"\
        :
        : "g" (num), "g" (source), "g" (dest)
        : "memory", "cc", "ecx", "esi", "edi"
    );
    return dest;
}

void * memcpy_reverse ( void * restrict dest, const void * restrict source, size_t num ) {
    unsigned char * sourceTop = (unsigned char*)source + num - 1;
    unsigned char * destTop = (unsigned char*)dest + num - 1;

    asm volatile(
        "mov ecx, %0\n\t"\
        "mov esi, %1\n\t"\
        "mov edi, %2\n\t"\
        "std        \n\t"\
        "rep movsb  \n\t"\
        "cld"
        :
        : "g" (num), "g" (sourceTop), "g" (destTop)
        : "memory", "cc", "ecx", "esi", "edi"
    );

    return dest;
}

void * memmove(void * dest, const void * source, size_t n) {
    unsigned char * d = (unsigned char*)dest;
    unsigned char * s = (unsigned char*)source;

    if (s > d) {
        memcpy(dest, source, n);
    }
    if (d > s) {
        memcpy_reverse(dest, source, n);
    }
    return dest;
}

void * memset(void * string, int c, size_t n) {
    asm volatile(
        "mov ecx, %0    \n\t"\
        "mov al, %1    \n\t"\
        "mov edi, %2    \n\t"\
        "cld            \n\t"\
        "rep stosb"
        :
        : "g" (n), "g" ((unsigned char)c), "g" (string)
        : "cc", "memory", "ecx", "eax", "edi"
    );
    return string;
}

char * strcpy(char * restrict dest, char * restrict source) {
    unsigned int i = 0;
    while (true) {
        char c = source[i];
        dest[i] = c;
        if (c == 0) {
            break;
        }
        i++;
    }
    return dest;
}

char * strncpy(char * restrict dest, char * restrict source, size_t n) {
    unsigned int i = 0;
    while (i < n) {
        char c = source[i];
        dest[i] = c;
        if (c == 0) {
            break;
        }
        i++;
    }
    return dest;
}

size_t strlen( const char * string ) {
    unsigned int i = 0;
    while (string[i] != 0) {
        i++;
    }
    return i;
}

char * strcat(char * dest, const char * source) {
    unsigned int destLength = strlen(dest);
    strcpy(dest + destLength, (char * restrict)source);
    return dest;
}

char * strncat(char * dest, const char * source, size_t n) {
    unsigned int destLength = strlen(dest);
    strncpy(dest + destLength, (char * restrict)source, n);
    return dest;
}

char * strchr(const char * string, int c) {
    unsigned char * s = (unsigned char *) string;
    unsigned char ch = (unsigned char)c;

    unsigned int i = 0;
    while (true) {
        if (s[i] == c) {
            return (char*)(s + i);
        }
        else if (s[i] == 0) {
            return NULL;
        }
        i++;
    }
}

int strcmp(const char * string1,  const char * string2) {
    unsigned int i = 0;
    unsigned char c1;
    unsigned char c2;
    while (true) {
        c1 = string1[i];
        c2 = string2[i];

        if (c1 != c2 || c1 == 0 || c2 == 0) {
            break;
        }
        i++;
    }

    if (c1 == c2) {
        return 0;
    }
    else if (c1 > c2) {
        return 1;
    }
    else {
        return -1;
    }
}

int strncmp(const char * string1,  const char * string2, size_t n) {
    unsigned int i;
    unsigned char c1;
    unsigned char c2;
    for (i = 0; i < n; i++) {
        c1 = string1[i];
        c2 = string2[i];

        if (c1 != c2 || c1 == 0 || c2 == 0) {
            break;
        }
    }
    if (i == n) {
        return 0;
    }

    if (c1 == c2) {
        return 0;
    }
    else if (c1 > c2) {
        return 1;
    }
    else {
        return -1;
    }
}

size_t strspn(const char * dest, const char * source) {
    unsigned int i = 0;
    const unsigned int source_len = strlen(source);
    while (dest[i] != 0) {
        unsigned int j;
        for (j = 0; j < source_len; j++) {
            if (source[j] == dest[i]) {
                break;
            }
        }
        if (j == source_len) {
            break;
        }
        i++;
    }
    return i;
}

size_t strcspn(const char * dest, const char * source) {
    unsigned int i = 0;
    const unsigned int source_len = strlen(source);
    while (dest[i] != 0) {
        unsigned int j;
        for (j = 0; j < source_len; j++) {
            if (source[j] == dest[i]) {
                return i;
            }
        }
        i++;
    }
    return i;
}

char * strpbrk(const char * dest, const char * breakset) {
    unsigned int i = 0;
    const unsigned int source_len = strlen(breakset);
    while (dest[i] != 0) {
        unsigned int j;
        for (j = 0; j < source_len; j++) {
            if (breakset[j] == dest[i]) {
                return (char*)(dest + i);
            }
        }
        i++;
    }
    return NULL;
}

char * strrchr(const char * string, int ch) {
    unsigned char c = (unsigned char)ch;

    char * returnPointer = NULL;

    unsigned int i = 0;
    for (i = 0; string[i] != 0; i++) {
        if (string[i] == ch) {
            returnPointer = (char*)&string[i];
        }
    }

    if (string[i] == ch) {
        returnPointer = (char*)&string[i];
    }

    return returnPointer;
}

char * strstr(const char * string, const char * substring) {
    unsigned int substr_len = strlen(substring);

    unsigned int i, j;
    for (i = 0; string[i] != 0; i++) {
        for (j = 0; j < substr_len; j++) {
            if (string[i + j] != substring[i]) {
                break;
            }
        }
        if (j == substr_len) {
            return (char*)(string + i);
        }
    }
    return NULL;
}

char * strtok_withStorage(char * restrict string, const char * restrict deliminators, char ** strtok_storage) {
    if (string != NULL) {
        char * start = (char*)string + strspn(string, deliminators);
        if (*start == 0) {
            return NULL;
        }
        char * end = (char*)start + strcspn(start, deliminators);
        *end = 0;
        *strtok_storage = end;
        return start;
    }
    else {
        char * start = *strtok_storage;
        char * end = (char*)start + strcspn(start, deliminators);
        *end = 0;
        *strtok_storage = end;
        return start;
    }
}

