#include <math.h>
#include <stdio.h>
#include "declarations.h"

// Тестовые функции
static double test_sin(double x) { return sin(x); }
static double test_cos(double x) { return cos(x); }
static double test_x2(double x) { return x * x; }
static double test_2x(double x) { return 2 * x; }
static double test_exp(double x) { return exp(x); }

void test_root(RootFinder* rf, int f1_idx, int f2_idx, double a, double b, double eps, double expected) {
    Function test_funcs[] = {
        create_function(test_sin, test_cos, "sin"),
        create_function(test_x2, test_2x, "x^2"),
        create_function(test_exp, test_exp, "exp")
    };
    
    int iterations;
    double result = rf->solve(&test_funcs[f1_idx], &test_funcs[f2_idx], a, b, eps, &iterations);
    double abs_error = fabs(result - expected);
    double rel_error = expected != 0.0 ? abs_error / fabs(expected) : abs_error;
    
    printf("Result: %.6f, Abs Error: %.6f, Rel Error: %.6f\n", result, abs_error, rel_error);
}

void test_integral(Integrator* integ, int f_idx, double a, double b, double eps, double expected) {
    Function test_funcs[] = {
        create_function(test_sin, test_cos, "sin"),
        create_function(test_x2, test_2x, "x^2"),
        create_function(test_exp, test_exp, "exp")
    };
    
    double result = integ->integrate(&test_funcs[f_idx], a, b, eps);
    double abs_error = fabs(result - expected);
    double rel_error = expected != 0.0 ? abs_error / fabs(expected) : abs_error;
    
    printf("Result: %.6f, Abs Error: %.6f, Rel Error: %.6f\n", result, abs_error, rel_error);
}