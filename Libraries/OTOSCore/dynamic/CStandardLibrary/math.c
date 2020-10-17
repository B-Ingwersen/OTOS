#include "CStandardLibrary/math.h"

extern void * _OTOSCore_LoadLocation;

double exp2(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 396);
	return func(x);
}

double exp(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 552);
	return func(x);
}

double expm1(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 96);
	return func(x);
}

double log2(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 668);
	return func(x);
}

double log(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 104);
	return func(x);
}

double log10(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 488);
	return func(x);
}

double log1p(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 680);
	return func(x);
}

int ilogb(double x) {
	int (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 648);
	return func(x);
}

double logb(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 132);
	return func(x);
}

double sqrt(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 108);
	return func(x);
}

double pow(double base, double exponent) {
	double (*func)(double,double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 464);
	return func(base,exponent);
}

double cbrt(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 496);
	return func(x);
}

double hypot(double x, double y) {
	double (*func)(double,double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 728);
	return func(x,y);
}

double divideBy2piRemainder(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 216);
	return func(x);
}

double sin(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 428);
	return func(x);
}

double cos(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 384);
	return func(x);
}

double tan(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 564);
	return func(x);
}

double asin(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 588);
	return func(x);
}

double acos(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 724);
	return func(x);
}

double atan(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 572);
	return func(x);
}

double atan2(double x, double y) {
	double (*func)(double,double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 440);
	return func(x,y);
}

double sinh(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 476);
	return func(x);
}

double cosh(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 116);
	return func(x);
}

double tanh(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 388);
	return func(x);
}

double asinh(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 120);
	return func(x);
}

double acosh(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 612);
	return func(x);
}

double atanh(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 484);
	return func(x);
}

double erfc(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 748);
	return func(x);
}

double erf(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 280);
	return func(x);
}

double tgamma(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 424);
	return func(x);
}

double lgamma(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 220);
	return func(x);
}

double ceil(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 164);
	return func(x);
}

double floor(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 172);
	return func(x);
}

double trunc(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 732);
	return func(x);
}

double round(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 136);
	return func(x);
}

double nearbyint(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 316);
	return func(x);
}

double rint(double x) {
	double (*func)(double) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 336);
	return func(x);
}

double frexp( double x, int* exp ) {
	double (*func)(double,int*) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 740);
	return func(x,exp);
}

double ldexp( double x, int exp ) {
	double (*func)(double,int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 240);
	return func(x,exp);
}

double modf( double x, double * iptr ) {
	double (*func)(double,double *) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 376);
	return func(x,iptr);
}

double scalbn( double x, int exp ) {
	double (*func)(double,int) = _OTOSCore_LoadLocation + *(unsigned int*)(_OTOSCore_LoadLocation + 600);
	return func(x,exp);
}

