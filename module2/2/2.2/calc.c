#include "./calc.h"

double add(int n, ...) {
    double result = 0;

    va_list args;
    va_start(args, n);

    for (int i = 0; i < n; i++) {
        result += va_arg(args, double);
    }

    va_end(args);

    return result;
}

double multiply(int n, ...) {
    double result = 0;

    va_list args;
    va_start(args, n);

    for (int i = 0; i < n; i++) {
        double tmp = va_arg(args, double);
        if (i == 0) {
            result = tmp;
        } else {
            result *= tmp;
        }
    }

    va_end(args);

    return result;
}

double subtract(double arg1, double arg2) {
    return arg1 - arg2;
}

double divide(double arg1, double arg2) {
    return arg1 / arg2;
}
