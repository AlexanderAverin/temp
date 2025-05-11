#include <math.h>
#include "declarations.h"

// Метод Симпсона (парабол) для интегрирования
static double simpson_integrate(Function* f, double a, double b, double eps) {
    int n = 4;  // Начальное число интервалов (должно быть четным)
    int max_iterations = 20;  // Ограничение на число итераций
    int iterations = 0;
    double h, sum_prev = 0.0, sum = 0.0;
    
    do {
        sum_prev = sum;
        sum = evaluate(f, a) + evaluate(f, b);
        h = (b - a) / n;
        
        for (int i = 1; i < n; i++) {
            double x = a + i * h;
            sum += (i % 2 == 0) ? 2 * evaluate(f, x) : 4 * evaluate(f, x);
        }
        
        sum *= h / 3;
        n *= 2;  // Удваиваем число интервалов для следующей итерации
        iterations++;
    } while (fabs(sum - sum_prev) > eps && iterations < max_iterations && n < 100000);
    
    return sum;
}

// Создаем интегратор, использующий метод Симпсона
Integrator create_simpson_method(void) {
    Integrator integ = { "Simpson", simpson_integrate };
    return integ;
}