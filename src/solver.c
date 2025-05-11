#include <stdio.h>
#include <math.h>
#include "declarations.h"

// Комбинированный метод хорд и касательных
static double combined_solve(Function* f, Function* g, double a, double b, double eps, int* iterations) {
    *iterations = 0;
    
    // Проверяем случай a = b
    if (fabs(a - b) < eps) {
        double fa = evaluate(f, a) - evaluate(g, a);
        if (fabs(fa) < eps) {
            return a; // a является корнем
        }
        // Возвращаем a с предупреждением (или можно вернуть NaN для ошибки)
        fprintf(stderr, "Warning: a = b and no root found at x = %.6f\n", a);
        return a;
    }
    
    double x_a = a, x_b = b;
    
    while (fabs(x_b - x_a) > eps && *iterations < 1000) {
        double fa = evaluate(f, x_a) - evaluate(g, x_a);
        double fb = evaluate(f, x_b) - evaluate(g, x_b);
        
        // Проверяем сходимость
        if (fabs(fa) < eps) return x_a;
        if (fabs(fb) < eps) return x_b;
        
        // Шаг метода касательных из x_a
        double df_a = evaluate_derivative(f, x_a) - evaluate_derivative(g, x_a);
        double x_newton = x_a;
        if (fabs(df_a) > 1e-10) { // Избегаем деления на ноль
            x_newton = x_a - fa / df_a;
        }
        
        // Шаг метода хорд из x_b
        double x_chord = x_b;
        if (fabs(fb - fa) > 1e-10) { // Избегаем деления на ноль
            x_chord = x_b - fb * (x_b - x_a) / (fb - fa);
        }
        
        // Выбираем новую точку
        double x_new = x_newton;
        if (x_new < a || x_new > b) x_new = x_chord;
        if (x_new < a || x_new > b) x_new = (x_a + x_b) / 2; // Запасной вариант
        
        double f_new = evaluate(f, x_new) - evaluate(g, x_new);
        
        // Обновляем границы
        if (f_new * fa < 0) {
            x_b = x_new;
        } else {
            x_a = x_new;
        }
        
        (*iterations)++;
    }
    
    return (x_a + x_b) / 2;
}

// Метод деления отрезка пополам (бисекции)
static double bisection_solve(Function* f, Function* g, double a, double b, double eps, int* iterations) {
    *iterations = 0;
    
    // Проверяем случай a = b
    if (fabs(a - b) < eps) {
        double fa = evaluate(f, a) - evaluate(g, a);
        if (fabs(fa) < eps) {
            return a; // a является корнем
        }
        // Возвращаем a с предупреждением
        fprintf(stderr, "Warning: a = b and no root found at x = %.6f\n", a);
        return a;
    }
    
    double fa = evaluate(f, a) - evaluate(g, a);
    double fb = evaluate(f, b) - evaluate(g, b);
    
    // Проверяем, что на концах отрезка функция имеет разные знаки
    if (fa * fb >= 0) {
        fprintf(stderr, "Warning: Function values at endpoints have the same sign\n");
        // В этом случае корень может отсутствовать на отрезке
        // Но мы всё равно попробуем найти его
    }
    
    double c, fc;
    
    while (fabs(b - a) > eps && *iterations < 1000) {
        // Находим середину отрезка
        c = (a + b) / 2.0;
        fc = evaluate(f, c) - evaluate(g, c);
        
        // Проверяем, достаточно ли близко мы подошли к корню
        if (fabs(fc) < eps) {
            return c;
        }
        
        // Выбираем новую половину отрезка
        if (fa * fc < 0) {
            b = c;
            fb = fc;
        } else {
            a = c;
            fa = fc;
        }
        
        (*iterations)++;
    }
    
    // Возвращаем середину финального отрезка
    return (a + b) / 2.0;
}

// Создаем функцию для инициализации комбинированного метода
RootFinder create_combined_method(void) {
    RootFinder rf = { "Combined Chord-Tangent", combined_solve };
    return rf;
}

// Создаем функцию для инициализации метода бисекции
RootFinder create_bisection_method(void) {
    RootFinder rf = { "Bisection", bisection_solve };
    return rf;
}