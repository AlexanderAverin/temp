#ifndef DECLARATIONS_H
#define DECLARATIONS_H

// Объявления функций из ассемблера
extern double f1(double x);
extern double f2(double x);
extern double f3(double x);
extern double df1(double x);
extern double df2(double x);
extern double df3(double x);

typedef double (*afunc)(double);

double root(afunc f, afunc g, afunc df, afunc dg, double a, double b, double eps1);
double integral(afunc f, double a, double b, double eps2);

// Function interface
typedef struct {
    afunc function;
    afunc derivative;
    char* name;
} Function;

// "main" abstract
typedef struct {
    Function f1, f2, f3;
    double a, b;
} Figure;

typedef struct {
    const char* name;
    double (*solve)(Function* f, Function* g, double a, double b, double eps, int* iterations);
} RootFinder;

typedef struct {
    const char* name;
    double (*integrate)(Function* f, double a, double b, double eps);
} Integrator;

// Function wrapper
Function create_function(afunc f, afunc df, const char* name);
double evaluate(Function* f, double x);
double evaluate_derivative(Function* f, double x);

// Figure wrapper
Figure create_figure(Function f1, Function f2, Function f3, double a, double b);
double calculate_area(Figure* fig, double eps, RootFinder* rf, Integrator* integ);
void find_intersection_points(Figure* fig, double eps1, RootFinder* rf, double* points, int* count);
void find_intersection_points_with_iterations(Figure* fig, double eps1, RootFinder* rf, double* points, int* count, int* iterations);

// Для работы с разностью функций
void set_difference_functions(Function* upper, Function* lower);
double function_difference(double x);

// RootFinder и Integrator
RootFinder create_combined_method(void);
RootFinder create_bisection_method(void);
Integrator create_simpson_method(void);

// Testing
void test_root(RootFinder* rf, int f1_idx, int f2_idx, double a, double b, double eps, double expected);
void test_integral(Integrator* integ, int f_idx, double a, double b, double eps, double expected);

#endif