#include <stdio.h>
#include <math.h>

#include "declarations.h"

// Метод деления отрезка пополам
static double bisection_solve(Function* f, Function* g, double a, double b, double eps, int* iterations) {
    *iterations = 0;
    
    // Проверяем случай a = b
    if (fabs(a - b) < eps) {
        double fa = evaluate(f, a) - evaluate(g, a);
        if (fabs(fa) < eps) {
            return a; // a является корнем
        }
        fprintf(stdout, "Warning: a = b and no root found at x = %.6f\n", a);
        return a;
    }
    
    double fa = evaluate(f, a) - evaluate(g, a);
    double fb = evaluate(f, b) - evaluate(g, b);
    
    // Проверяем, что на концах отрезка функция имеет разные знаки
    if (fa * fb >= 0) {
        fprintf(stdout, "Warning: Function values at endpoints have the same sign (f(a) = %.6f, f(b) = %.6f)\n", fa, fb);
        // Попробуем найти подходящий отрезок
        double step = (b - a) / 10.0;
        double x = a + step;
        
        while (x < b) {
            double fx = evaluate(f, x) - evaluate(g, x);
            if (fa * fx < 0) {
                // Нашли подходящий отрезок
                b = x;
                fb = fx;
                break;
            }
            if (fx * fb < 0) {
                // Нашли подходящий отрезок
                a = x;
                fa = fx;
                break;
            }
            // Обновляем точку x
            x += step;
        }
        
        // Если мы по-прежнему не нашли подходящий отрезок
        if (fa * fb >= 0) {
            fprintf(stdout, "Error: Could not find interval with opposite signs\n");
            // Возвращаем точку, где значение функции ближе к нулю
            return (fabs(fa) < fabs(fb)) ? a : b;
        }
    }
    
    double c, fc;
    
    while (*iterations < 1000) {
        // Находим середину отрезка
        c = (a + b) / 2.0;
        fc = evaluate(f, c) - evaluate(g, c);
        
        // Проверяем критерии остановки
        if (fabs(fc) < eps || fabs(b - a) < eps) {
            return c;
        }
        
        // Выбираем новую половину отрезка
        if (fc * fa < 0) {
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

// Комбинированный метод хорд и касательных
static double combined_solve(Function* f, Function* g, double a, double b, double eps, int* iterations) {
    *iterations = 0;
    
    // Проверяем случай a = b
    if (fabs(a - b) < eps) {
        double fa = evaluate(f, a) - evaluate(g, a);
        if (fabs(fa) < eps) {
            return a; // a является корнем
        }
        fprintf(stdout, "Warning: a = b and no root found at x = %.6f\n", a);
        return a;
    }
    
    // Функция разности f(x) - g(x)
    double fa = evaluate(f, a) - evaluate(g, a);
    double fb = evaluate(f, b) - evaluate(g, b);
    
    // Проверяем, что на концах отрезка функция имеет разные знаки
    if (fa * fb >= 0) {
        fprintf(stdout, "Warning: Function values at endpoints have the same sign (f(a) = %.6f, f(b) = %.6f)\n", fa, fb);
        // Попробуем найти подходящий отрезок
        double step = (b - a) / 10.0;
        double x = a + step;
        
        while (x < b) {
            double fx = evaluate(f, x) - evaluate(g, x);
            if (fa * fx < 0) {
                // Нашли подходящий отрезок
                b = x;
                fb = fx;
                break;
            }
            if (fx * fb < 0) {
                // Нашли подходящий отрезок
                a = x;
                fa = fx;
                break;
            }
            // Обновляем точку x
            x += step;
        }
        
        // Если мы по-прежнему не нашли подходящий отрезок
        if (fa * fb >= 0) {
            fprintf(stdout, "Error: Could not find interval with opposite signs\n");
            // Возвращаем точку, где значение функции ближе к нулю
            return (fabs(fa) < fabs(fb)) ? a : b;
        }
    }
    
    // Определяем начальные точки для методов
    double x0 = a;  // Для метода касательных
    double x1 = b;  // Для метода хорд
    
    while (*iterations < 1000) {
        // Вычисляем значения функции и производной
        double f0 = evaluate(f, x0) - evaluate(g, x0);
        double f1 = evaluate(f, x1) - evaluate(g, x1);
        
        // Проверяем, достаточно ли мы близко к корню
        if (fabs(f0) < eps) return x0;
        if (fabs(f1) < eps) return x1;
        if (fabs(x1 - x0) < eps) return (x0 + x1) / 2.0;
        
        // Шаг метода касательных (Ньютона)
        double df0 = evaluate_derivative(f, x0) - evaluate_derivative(g, x0);
        double x_newton = x0;
        
        if (fabs(df0) > eps) {
            x_newton = x0 - f0 / df0;
        } else {
            // Если производная близка к нулю, используем метод хорд вместо касательных
            x_newton = x0 - f0 * (x1 - x0) / (f1 - f0);
        }
        
        // Шаг метода хорд
        double x_chord = x1 - f1 * (x1 - x0) / (f1 - f0);
        
        // Проверка выхода за границы отрезка
        if (x_newton < a || x_newton > b) {
            x_newton = (x0 + x1) / 2.0;  // Заменяем метод касательных бисекцией
        }
        
        if (x_chord < a || x_chord > b) {
            x_chord = (x0 + x1) / 2.0;  // Заменяем метод хорд бисекцией
        }
        
        // Выбираем лучшее приближение на основе значения функции
        double f_newton = evaluate(f, x_newton) - evaluate(g, x_newton);
        double f_chord = evaluate(f, x_chord) - evaluate(g, x_chord);
        
        double x_new = (fabs(f_newton) < fabs(f_chord)) ? x_newton : x_chord;
        double f_new = (fabs(f_newton) < fabs(f_chord)) ? f_newton : f_chord;
        
        // Обновляем границы для следующей итерации
        if (f0 * f_new < 0) {
            x1 = x_new;
        } else {
            x0 = x_new;
        }
        
        (*iterations)++;
    }
    
    return (x0 + x1) / 2.0;
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