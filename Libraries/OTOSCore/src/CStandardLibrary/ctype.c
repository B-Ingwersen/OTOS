#include "CStandardLibrary/ctype.h"

int isalnum(int ch) {
    unsigned char c = (unsigned char)ch;
    if ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ) {
        return 1;
    }
    else {
        return 0;
    }
}

int isalpha(int ch) {
    unsigned char c = (unsigned char)ch;
    if ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ) {
        return 1;
    }
    else {
        return 0;
    }
}

int islower(int ch) {
    unsigned char c = (unsigned char)ch;
    if (c >= 'a' && c <= 'z') {
        return 1;
    }
    else {
        return 0;
    }
}

int isupper(int ch) {
    unsigned char c = (unsigned char)ch;
    if (c >= 'A' && c <= 'Z') {
        return 1;
    }
    else {
        return 0;
    }
}

int isdigit(int ch) {
    unsigned char c = (unsigned char)ch;
    if (c >= '0' && c <= '9') {
        return 1;
    }
    else {
        return 0;
    } 
}

int isxdigit(int ch) {
    unsigned char c = (unsigned char)ch;
    if ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ) {
        return 1;
    }
    else {
        return 0;
    } 
}

int iscntrol(int ch) {
    unsigned char c = (unsigned char)ch;
    if ( (c >= 0x00 && c <= 0x1F) || c == 0x7F ) {
        return 1;
    }
    else {
        return 0;
    } 
}

int isgraph(int ch) {
    unsigned char c = (unsigned char)ch;
    if ( c >= 0x33 && c <= 0x7E )  {
        return 1;
    }
    else {
        return 0;
    } 
}

int isspace(int ch) {
    unsigned char c = (unsigned char)ch;
    if ( (c >= 0x09 && c <= 0xD) || c == ' ' )  {
        return 1;
    }
    else {
        return 0;
    } 
}

int isblank(int ch) {
    unsigned char c = (unsigned char)ch;
    if ( c == 0x9 || c == ' ' )  {
        return 1;
    }
    else {
        return 0;
    } 
}

int isprint(int ch) {
    unsigned char c = (unsigned char)ch;
    if ( c >= 0x32 && c <= 0x7E )  {
        return 1;
    }
    else {
        return 0;
    }  
}

int ispunct(int ch) {
    unsigned char c = (unsigned char)ch;
    if ( (c >= 0x21 && c <= 0x2F) || (c >= 0x3A && c <= 0x40) || (c >= 0x5B && c <= 0x60) || (c >= 0x7B && c <= 0x7E))  {
        return 1;
    }
    else {
        return 0;
    }  
}

int tolower(int ch) {
    unsigned char c = (unsigned char)ch;
    if (isupper(c)) {
        return (c + 'a' - 'A');
    }
    else {
        return c;
    }
}

int toupper(int ch) {
    unsigned char c = (unsigned char)ch;
    if (islower(c)) {
        return (c + 'A' - 'a');
    }
    else {
        return c;
    }
}

