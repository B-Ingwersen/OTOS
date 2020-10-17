
#include "Definitions.h"
#include "CStandardLibrary/stdlib.h"
#include "CStandardLibrary/ctype.h"
#include "CStandardLibrary/math.h"

double strtod(char * string, char ** endPointer) {
    double integer = 0;
    double numerator = 0;
    double denominator = 1.0;
    double exponent = 0.0;

    bool32 negative = false;
    bool32 decimal = false;
    bool32 hasExponent = false;
    bool32 exponentNegative = false;

    u32 i = 0;
    for (; isspace(string[i]); i++) {}
    if (string[i] == '+') {
        i++;
    }
    else if (string[i] == '-') {
        negative = true;
        i++;
    }

    for (; string[i] != 0; i++) {
        if (exponent) {
            if (string[i] >= '0' && string[i] <= '9') {
                exponent *= 10.0;
                exponent += (double)(string[i] - '0');
            }
            else {
                break;
            }
            continue;
        }
        else if (decimal) {
            if (string[i] >= '0' && string[i] <= '9') {
                numerator *= 10.0;
                numerator += (double)(string[i] - '0');
                denominator *= 10.0;
            }
            else {
                break;
            }
            continue;
        }
        else if (string[i] == '.') {
            decimal = true;
            continue;
        }
        else if (string[i] == 'e' || string[i] == 'E') {
            hasExponent = true;
            i++;
            if (string[i] == '+') {
                i++;
            }
            else if (string[i] == '-') {
                exponentNegative = true;
                i++;
            }
            continue;
        }
        else {
            if (string[i] >= '0' && string[i] <= '9') {
                integer *= 10.0;
                integer += (double)(string[i] - '0');
            }
            else {
                break;
            }
            continue;
        }
    }

    if (endPointer != NULL) {
        *endPointer = string + i;
    }

    double value = integer;
    if (decimal) {
        value += numerator / denominator;
    }
    if (hasExponent) {
        double exp = pow(10.0, exponent);
        value *= exp;
    }
    if (negative) {
        value = -value;
    }
    return value;
}
long int strtol(char * string, char ** endPointer) {
    long int value = 0;
    bool32 negative = false;

    u32 i = 0;
    for (; isspace(string[i]); i++) {}
    if (string[i] == '+') {
        i++;
    }
    else if (string[i] == '-') {
        negative = true;
        i++;
    }
    for (; string[i] != 0; i++) {
        if (string[i] >= '0' && string[i] <= '9') {
            value *= 10;
            value += (long int)(string[i] - '0');
        }
        else {
            return 0;
        }
    }

    if (endPointer != NULL) {
        *endPointer = string + i;
    }

    if (negative) {
        value = -value;
    }
    return value;
}
unsigned long int strtoul(char * string, char ** endPointer) {
    unsigned long int value = 0;
    bool32 negative = false;

    u32 i = 0;
    for (; isspace(string[i]); i++) {}
    if (string[i] == '+') {
        i++;
    }
    else if (string[i] == '-') {
        negative = true;
        i++;
    }
    for (; string[i] != 0; i++) {
        if (string[i] >= '0' && string[i] <= '9') {
            value *= 10;
            value += (unsigned long int)(string[i] - '0');
        }
        else {
            return 0;
        }
    }

    if (endPointer != NULL) {
        *endPointer = string + i;
    }

    if (negative) {
        value = -value;
    }
    return value;
}
float atof(char * string) {
    return (float)strtod(string, NULL);
}
int atoi(char * string) {
    int value = 0;
    bool32 negative = false;

    u32 i = 0;
    for (; isspace(string[i]); i++) {}
    if (string[i] == '+') {
        i++;
    }
    else if (string[i] == '-') {
        negative = true;
        i++;
    }
    for (; string[i] != 0; i++) {
        if (string[i] >= '0' && string[i] <= '9') {
            value *= 10;
            value += (int)(string[i] - '0');
        }
        else {
            break;
        }
    }

    if (negative) {
        value = -value;
    }
    return value;
}
long int atol(char * string) {
    return (long int)strtol(string, NULL);
}

int abs(int n) {
    if (n < 0) {
        return -n;
    }
    else {
        return n;
    }
}
div_t div(int numerator, int denominator) {
    div_t result;
    result.quot = numerator / denominator;
    result.rem = numerator % denominator;
    return result;
}
long int labs(long int n) {
    if (n < 0) {
        return -n;
    }
    else {
        return n;
    }
}
ldiv_t ldiv(long int numerator, long int denominator) {
    ldiv_t result;
    result.quot = numerator / denominator;
    result.rem = numerator % denominator;
    return result;
}

int rand_NextValuePointer(unsigned long int * next) {
    *next = (*next) * 1103515245 + 12345;
    return (unsigned int)((*next) / 65536) % 32768;
}
 
void srand_NextValuePointer(unsigned int seed, unsigned long int * next) {
    *next = seed;
}

