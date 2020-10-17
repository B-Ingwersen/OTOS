#ifndef OTOS_CORE___C_STANDARD_LIBRARY___MATH_H
#define OTOS_CORE___C_STANDARD_LIBRARY___MATH_H

double fabs(double x);

double exp2(double x);

double exp(double x);

double expm1(double x);

double log2(double x);

double log(double x);

double log10(double x);

double log1p(double x);

int ilogb(double x);

double logb(double x);

double sqrt(double x);

double pow(double base, double exponent);

double cbrt(double x);

double hypot(double x, double y);

double divideBy2piRemainder(double x);

double sin(double x);

double cos(double x);

double tan(double x);

double asin(double x);

double acos(double x);

double atan(double x);

double atan2(double x, double y);

double sinh(double x);

double cosh(double x);

double tanh(double x);

double asinh(double x);

double acosh(double x);

double atanh(double x);

double erfc(double x);

double erf(double x);

double tgamma(double x);

double lgamma(double x);

double ceil(double x);

double floor(double x);

double trunc(double x);

double round(double x);

double nearbyint(double x);

double rint(double x);

double frexp( double x, int* exp );

double ldexp( double x, int exp );

double modf( double x, double * iptr );

double scalbn( double x, int exp );

#endif
