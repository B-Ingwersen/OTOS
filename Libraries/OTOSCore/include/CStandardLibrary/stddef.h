#ifndef OTOS_CORE___C_STANDARD_LIBRARY___STDDEF_H
#define OTOS_CORE___C_STANDARD_LIBRARY___STDDEF_H

#define offsetof(type, member) __builtin_offsetof(type, member)

typedef unsigned long int size_t;

#endif
