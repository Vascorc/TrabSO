#include <stdlib.h>
#include <math.h>
#include "utils.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double generate_exponential(double lambda) {
    double u = ((double) rand()) / RAND_MAX;
    return -log(1 - u) / lambda;
}

double generate_poisson(double lambda) {
    double L = exp(-lambda);
    double p = 1.0;
    int k = 0;
    do {
        k++;
        p *= ((double) rand()) / RAND_MAX;
    } while (p > L);
    return k - 1;
}

double generate_normal(double mean, double std_dev) {
    double u1 = ((double) rand()) / RAND_MAX;
    double u2 = ((double) rand()) / RAND_MAX;
    double z = sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);
    return z * std_dev + mean;
}
