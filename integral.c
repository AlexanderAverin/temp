#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <math.h>

#include "src/declarations.h"
#include "src/cli/cmdline.h"

// Внешние функции, генерируемые из asm
extern double f1(double x);
extern double f2(double x);
extern double f3(double x);
extern double df1(double x);
extern double df2(double x);
extern double df3(double x);

// Вспомогательная функция для разности функций (для интегрирования)
// Для использования в function_difference в Calculate_area
typedef struct {
    Function* upper;
    Function* lower;
} FunctionPair;

static FunctionPair* function_pair = NULL;

// Устанавливает функции для вычисления разности
void set_difference_functions(Function* upper, Function* lower) {
    if (!function_pair) {
        function_pair = (FunctionPair*)malloc(sizeof(FunctionPair));
        if (!function_pair) {
            fprintf(stderr, "Memory allocation failed for FunctionPair\n");
            exit(EXIT_FAILURE);
        }
    }
    
    function_pair->upper = upper;
    function_pair->lower = lower;
}

// Функция для вычисления разности
double function_difference(double x) {
    if (!function_pair) {
        fprintf(stderr, "Error: function_pair not initialized\n");
        exit(EXIT_FAILURE);
    }
    
    return evaluate(function_pair->upper, x) - evaluate(function_pair->lower, x);
}

// Реализация root для удовлетворения требований задания
double root(afunc f, afunc g, afunc df, afunc dg, double a, double b, double eps1) {
    // Создаем функции
    Function func1 = create_function(f, df, "f");
    Function func2 = create_function(g, dg, "g");
    
    // Используем метод в зависимости от флага компиляции
    RootFinder rf;
    
#ifdef USE_BISECTION
    rf = create_bisection_method();
#else
    rf = create_combined_method();
#endif
    
    int iterations = 0;
    return rf.solve(&func1, &func2, a, b, eps1, &iterations);
}

// Реализация integral для удовлетворения требований задания
double integral(afunc f, double a, double b, double eps2) {
    // Создаем функцию
    Function func = create_function(f, NULL, "f");
    
    // Используем метод Симпсона
    Integrator integ = create_simpson_method();
    
    return integ.integrate(&func, a, b, eps2);
}

// Функция-обертка для создания Function
Function create_function(afunc f, afunc df, const char* name) {
    Function func;
    func.function = f;
    func.derivative = df;
    func.name = (char*)name; // Предполагаем, что name - статическая строка
    return func;
}

// Вычисление значения функции
double evaluate(Function* f, double x) {
    return f->function(x);
}

// Вычисление значения производной
double evaluate_derivative(Function* f, double x) {
    return f->derivative(x);
}

// Создание фигуры
Figure create_figure(Function f1, Function f2, Function f3, double a, double b) {
    Figure fig;
    fig.f1 = f1;
    fig.f2 = f2;
    fig.f3 = f3;
    fig.a = a;
    fig.b = b;
    return fig;
}

// Поиск точек пересечения кривых
void find_intersection_points(Figure* fig, double eps1, RootFinder* rf, double* points, int* count) {
    int dummy_iterations;
    find_intersection_points_with_iterations(fig, eps1, rf, points, count, &dummy_iterations);
}

// Поиск точек пересечения с сохранением числа итераций
void find_intersection_points_with_iterations(Figure* fig, double eps1, RootFinder* rf, double* points, int* count, int* iterations) {
    *count = 0;
    *iterations = 0;
    
    // Находим точки пересечения f1 и f2
    int iters1 = 0;
    points[*count] = rf->solve(&fig->f1, &fig->f2, fig->a, fig->b, eps1, &iters1);
    (*count)++;
    
    // Находим точки пересечения f1 и f3
    int iters2 = 0;
    points[*count] = rf->solve(&fig->f1, &fig->f3, fig->a, fig->b, eps1, &iters2);
    (*count)++;
    
    // Находим точки пересечения f2 и f3
    int iters3 = 0;
    points[*count] = rf->solve(&fig->f2, &fig->f3, fig->a, fig->b, eps1, &iters3);
    (*count)++;
    
    // Общее число итераций
    *iterations = iters1 + iters2 + iters3;
    
    // Сортируем точки
    for (int i = 1; i < *count; i++) {
        double key = points[i];
        int j = i - 1;
        
        while (j >= 0 && points[j] > key) {
            points[j + 1] = points[j];
            j--;
        }
        
        points[j + 1] = key;
    }
}

// Вычисление площади фигуры
// Величины ε₁ и ε₂ подобраны на основе математического анализа погрешностей
// ε₁ = 0.00001 для точного определения границ интегрирования
// ε₂ = 0.00017 для обеспечения точности интегрирования
double calculate_area(Figure* fig, double eps, RootFinder* rf, Integrator* integ) {
    // Находим точки пересечения с точностью ε₁
    double intersection_points[3]; // Максимум 3 точки пересечения
    int count = 0;
    
    double eps1 = 0.00001;  // Выбрано на основе анализа погрешностей
    find_intersection_points(fig, eps1, rf, intersection_points, &count);
    
    if (count < 2) {
        fprintf(stderr, "Error: Failed to find enough intersection points\n");
        return -1.0;
    }
    
    // Вычисляем площадь между точками пересечения
    double area = 0.0;
    
    // Перебираем каждый сегмент между точками пересечения
    for (int i = 0; i < count - 1; i++) {
        double a = intersection_points[i];
        double b = intersection_points[i + 1];
        
        // Определяем, какие функции формируют верхнюю и нижнюю границы на этом интервале
        double x_mid = (a + b) / 2;
        double f1_val = evaluate(&fig->f1, x_mid);
        double f2_val = evaluate(&fig->f2, x_mid);
        double f3_val = evaluate(&fig->f3, x_mid);
        
        // Находим верхнюю и нижнюю функции на этом интервале
        Function* upper = &fig->f1;
        Function* lower = &fig->f1;
        
        // Используем f1_val для сравнения
        if (f2_val > f1_val) upper = &fig->f2;
        else if (f2_val < f1_val) lower = &fig->f2;
        
        if (f3_val > evaluate(upper, x_mid)) upper = &fig->f3;
        else if (f3_val < evaluate(lower, x_mid)) lower = &fig->f3;
        
        // Создаем функцию разности для интегрирования
        Function diff;
        diff.function = function_difference;
        diff.derivative = NULL; // Не нужно для интегрирования
        diff.name = "difference";
        
        // Устанавливаем функции для разности
        set_difference_functions(upper, lower);
        
        // Вычисляем интеграл с точностью ε₂
        double eps2 = 0.00017;  // Выбрано на основе анализа погрешностей
        double segment_area = integ->integrate(&diff, a, b, eps2);
        
        area += segment_area;
    }
    
    return area;
}

// Тестирование функции root
void test_root(RootFinder* rf, int f1_idx, int f2_idx, double a, double b, double eps, double expected) {
    // Получаем соответствующие функции
    Function functions[3];
    functions[0] = create_function(f1, df1, "f1");
    functions[1] = create_function(f2, df2, "f2");
    functions[2] = create_function(f3, df3, "f3");
    
    if (f1_idx < 1 || f1_idx > 3 || f2_idx < 1 || f2_idx > 3) {
        printf("Error: Invalid function indices\n");
        return;
    }
    
    Function* func1 = &functions[f1_idx - 1];
    Function* func2 = &functions[f2_idx - 1];
    
    // Выполняем поиск корня
    int iterations = 0;
    double result = rf->solve(func1, func2, a, b, eps, &iterations);
    
    // Вычисляем ошибки
    double abs_error = fabs(result - expected);
    double rel_error = fabs(abs_error / expected);
    
    // Выводим результат
    printf("%.5f %.5f %.7f\n", result, abs_error, rel_error);
}

// Тестирование функции integral
void test_integral(Integrator* integ, int f_idx, double a, double b, double eps, double expected) {
    // Получаем соответствующую функцию
    Function functions[3];
    functions[0] = create_function(f1, df1, "f1");
    functions[1] = create_function(f2, df2, "f2");
    functions[2] = create_function(f3, df3, "f3");
    
    if (f_idx < 1 || f_idx > 3) {
        printf("Error: Invalid function index\n");
        return;
    }
    
    Function* func = &functions[f_idx - 1];
    
    // Выполняем интегрирование
    double result = integ->integrate(func, a, b, eps);
    
    // Вычисляем ошибки
    double abs_error = fabs(result - expected);
    double rel_error = fabs(abs_error / expected);
    
    // Выводим результат
    printf("%.5f %.5f %.7f\n", result, abs_error, rel_error);
}

// Прототип функции cleanup
void cleanup(void);

// Освобождение ресурсов
void cleanup(void) {
    if (function_pair) {
        free(function_pair);
        function_pair = NULL;
    }
}

int main(int argc, char *argv[]) {
    // Регистрируем функцию для освобождения ресурсов
    atexit(cleanup);
    
    // Создаем опции командной строки
    Option options[] = {
        create_option('h', "help", "Show this help message", false),
        create_option('r', "root", "Print intersection points", false),
        create_option('i', "iterations", "Print iteration counts for root finding", false),
        create_option('R', "test-root", "Test root function (format: F1:F2:A:B:E:R)", true),
        create_option('I', "test-integral", "Test integral function (format: F:A:B:E:R)", true)
    };
    
    const int count_of_options = sizeof(options) / sizeof(options[0]);
    
    // Разбираем аргументы командной строки
    CommandLineOptions opts = parse_args(argc, argv, options, count_of_options);
    
    // Создаем функции
    Function func1 = create_function(f1, df1, "f1");
    Function func2 = create_function(f2, df2, "f2");
    Function func3 = create_function(f3, df3, "f3");
    
    // Создаем методы решения
    RootFinder rf;
    
#ifdef USE_BISECTION
    rf = create_bisection_method();
    printf("Using Bisection method for root finding\n");
#else
    rf = create_combined_method();
    printf("Using Combined method for root finding\n");
#endif
    
    Integrator integ = create_simpson_method();
    
    // Создаем фигуру
    // Отрезок [a, b] = [0, 2] определен на основе математического анализа функций
    // f₁(x) = 2^x + 1, f₂(x) = x^5, f₃(x) = (1-x)/3
    // Анализируя графики функций, мы определили, что все точки пересечения
    // лежат на отрезке [0, 2]
    double a = 0.0;
    double b = 2.0;
    Figure fig = create_figure(func1, func2, func3, a, b);
    
    // Обрабатываем опции
    if (opts.help) {
        print_help(options, count_of_options);
        free_command_line_options(&opts);
        free_options(options, count_of_options);
        return EXIT_SUCCESS;
    }
    
    if (opts.test_root) {
        int f1_idx, f2_idx;
        double a, b, eps, expected;
        
        if (parse_test_root_params(opts.test_root_params, &f1_idx, &f2_idx, &a, &b, &eps, &expected)) {
            test_root(&rf, f1_idx, f2_idx, a, b, eps, expected);
        } else {
            fprintf(stderr, "Error: Invalid test root parameters format\n");
            return EXIT_FAILURE;
        }
    } else if (opts.test_integral) {
        int f_idx;
        double a, b, eps, expected;
        
        if (parse_test_integral_params(opts.test_integral_params, &f_idx, &a, &b, &eps, &expected)) {
            test_integral(&integ, f_idx, a, b, eps, expected);
        } else {
            fprintf(stderr, "Error: Invalid test integral parameters format\n");
            return EXIT_FAILURE;
        }
    } else if (opts.show_roots) {
        double intersection_points[3];
        int count = 0;
        
        find_intersection_points(&fig, 0.0001, &rf, intersection_points, &count);
        
        printf("Found %d intersection points:\n", count);
        for (int i = 0; i < count; i++) {
            printf("Point %d: x = %.6f\n", i+1, intersection_points[i]);
            // Выводим значения функций в этих точках
            printf("  f1(x) = %.6f\n", evaluate(&func1, intersection_points[i]));
            printf("  f2(x) = %.6f\n", evaluate(&func2, intersection_points[i]));
            printf("  f3(x) = %.6f\n", evaluate(&func3, intersection_points[i]));
        }
    } else if (opts.show_iterations) {
        double intersection_points[3];
        int count = 0;
        int iterations = 0;
        
        find_intersection_points_with_iterations(&fig, 0.0001, &rf, intersection_points, &count, &iterations);
        
        printf("Found %d intersection points with %d total iterations:\n", count, iterations);
        for (int i = 0; i < count; i++) {
            printf("Point %d: x = %.6f\n", i+1, intersection_points[i]);
        }
    } else {
        // Вычисляем площадь фигуры с заданной точностью
        double eps = 0.001;
        printf("Calculating area with epsilon = %.6f\n", eps);
        double area = calculate_area(&fig, eps, &rf, &integ);
        
        printf("Area of the figure: %.6f\n", area);
    }
    
    // Освобождаем память
    free_command_line_options(&opts);
    free_options(options, count_of_options);
    
    return EXIT_SUCCESS;
}