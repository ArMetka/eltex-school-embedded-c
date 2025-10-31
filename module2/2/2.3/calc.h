#ifndef CALC_H
#define CALC_H

#include <math.h>

typedef struct {
    char name[32];
    double (*op)(double, double);
} binary_op;

double add(double arg1, double arg2);
double multiply(double arg1, double arg2);
double subtract(double arg1, double arg2);
double divide(double arg1, double arg2);

#endif