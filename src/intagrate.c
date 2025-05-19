#include <math.h>

#include "declarations.h"

static double simpson_integrate(Function* f, double a, double b, double eps) {
    int n = 4;  // Начальное число интервалов (должно быть четным)
    int max_iterations = 20;  
    double result_prev = 0.0;
    double result = 0.0;
    
    for (int iterations = 0; iterations < max_iterations; iterations++) {
        double h = (b - a) / n;
        double sum = evaluate(f, a) + evaluate(f, b);  // Краевые точки
        
        for (int i = 1; i < n; i++) {
            double x = a + i * h;
            if (i % 2 == 0) {
                sum += 2 * evaluate(f, x);  // Четные узлы
            } else {
                sum += 4 * evaluate(f, x);  // Нечетные узлы
            }
        }
        
        result = sum * h / 3.0;
        
        // Проверяем сходимость
        if (iterations > 0 && fabs(result - result_prev) < eps) {
            break;
        }
        
        result_prev = result;
        n *= 2;  // Удваиваем число интервалов
    }
    
    return result;
}

Integrator create_simpson_method(void) {
    Integrator integ = { "Simpson", simpson_integrate };
    return integ;
}