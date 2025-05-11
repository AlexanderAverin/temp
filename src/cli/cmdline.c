#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

#include <stdbool.h>

#include "cmdline.h"

#define MAX_PARAM_LEN 256

// Contructor (wrapper) for creating new options
Option create_option(char short_name, const char* full_name, const char* description, bool is_requires_arg) {
    // strdup(str) create copy of function (str) and return in. Uses for work with copy of string (not mutate)

    // I know that is bad way for working with structures :)
    Option new_opt;
    new_opt.short_name = short_name;
    new_opt.full_name = strdup(full_name);
    new_opt.description = strdup(description);
    new_opt.is_requires_arg = is_requires_arg;

    return new_opt;
}

// Free memory allocated for <Option> type. Free only char* fields
void free_option(Option* opt) {
    free(opt->full_name);
    free(opt->description);
}

// log help
// I know that is bad way for send args in functions like that, but i can`t use static data (functions calls in a not const)
void print_help(Option* options, int count_of_options) {
    printf("Usage: ./integral [options]\n");
    printf("Options:\n");
    for (int i = 0; i < count_of_options; i++) {
        printf("  -%c, --%-15s %s\n", options[i].short_name, options[i].full_name, options[i].description);
    }
}

// Типы обработчиков опций
typedef void (*option_handler_t)(CommandLineOptions* opts, const char* arg);

// Обработчики для каждой опции
static void handle_help(CommandLineOptions* opts, const char* arg) {
    (void)arg; // Не используем аргумент
    opts->help = true;
}

static void handle_show_roots(CommandLineOptions* opts, const char* arg) {
    (void)arg; // Не используем аргумент
    opts->show_roots = true;
}

static void handle_show_iterations(CommandLineOptions* opts, const char* arg) {
    (void)arg; // Не используем аргумент
    opts->show_iterations = true;
}

static void handle_test_root(CommandLineOptions* opts, const char* arg) {
    opts->test_root = true;
    if (arg && opts->test_root_params == NULL) {
        size_t len = strlen(arg);
        opts->test_root_params = (char*)malloc(len + 1);
        if (opts->test_root_params) {
            // Заменяем strncpy на memcpy и явно добавляем нулевой символ
            memcpy(opts->test_root_params, arg, len);
            opts->test_root_params[len] = '\0';
        }
    }
}

static void handle_test_integral(CommandLineOptions* opts, const char* arg) {
    opts->test_integral = true;
    if (arg && opts->test_integral_params == NULL) {
        size_t len = strlen(arg);
        opts->test_integral_params = (char*)malloc(len + 1);
        if (opts->test_integral_params) {
            // Заменяем strncpy на memcpy и явно добавляем нулевой символ
            memcpy(opts->test_integral_params, arg, len);
            opts->test_integral_params[len] = '\0';
        }
    }
}

static void handle_default(CommandLineOptions* opts, const char* arg) {
    (void)arg; // Не используем аргумент
    opts->help = true;
}

// Парсинг аргументов
CommandLineOptions parse_args(int argc, char* argv[], Option* options, int count_of_options) {
    CommandLineOptions opts = { false, false, false, false, false, NULL, NULL };
    
    // Создаем таблицу диспетчеризации обработчиков
    option_handler_t option_handlers[256] = {0}; // Инициализируем все нулями
    option_handlers['h'] = handle_help;
    option_handlers['r'] = handle_show_roots;
    option_handlers['i'] = handle_show_iterations;
    option_handlers['R'] = handle_test_root;
    option_handlers['I'] = handle_test_integral;
    
    // Подготовка для getopt_long
    struct option* long_options = calloc(count_of_options + 1, sizeof(struct option));
    if (!long_options) {
        fprintf(stderr, "Memory allocation failed for long_options\n");
        exit(EXIT_FAILURE);
    }
    
    char* optstring = calloc(2 * count_of_options + 1, sizeof(char));
    if (!optstring) {
        fprintf(stderr, "Memory allocation failed for optstring\n");
        free(long_options);
        exit(EXIT_FAILURE);
    }
    
    int optstring_idx = 0;
    for (int i = 0; i < count_of_options; i++) {
        long_options[i].name = options[i].full_name;
        long_options[i].has_arg = options[i].is_requires_arg ? required_argument : no_argument;
        long_options[i].flag = NULL;
        long_options[i].val = options[i].short_name;
        
        optstring[optstring_idx++] = options[i].short_name;
        if (options[i].is_requires_arg) {
            optstring[optstring_idx++] = ':';
        }
    }
    optstring[optstring_idx] = '\0';
    
    // Парсинг
    int opt;
    while ((opt = getopt_long(argc, argv, optstring, long_options, NULL)) != -1) {
        // Диспетчеризация через массив функций
        option_handler_t handler = 
            (opt >= 0 && opt < 256 && option_handlers[opt]) 
            ? option_handlers[opt] 
            : handle_default;
            
        handler(&opts, optarg);
    }
    
    free(long_options);
    free(optstring);
    return opts;
}

// Безопасный парсинг параметров тестов
bool parse_test_root_params(const char* params, int* f1, int* f2, double* a, double* b, double* eps, double* expected) {
    if (!params || !f1 || !f2 || !a || !b || !eps || !expected) {
        return false;
    }
    
    // Создаем локальную копию с ограниченным размером
    char copy[MAX_PARAM_LEN];
    if (strlen(params) >= MAX_PARAM_LEN) {
        return false;
    }
    strncpy(copy, params, MAX_PARAM_LEN - 1);
    copy[MAX_PARAM_LEN - 1] = '\0';
    
    // Парсим токены
    char* saveptr = NULL; // Для потокобезопасного strtok_r
    char* token = strtok_r(copy, ":", &saveptr);
    if (!token) return false;
    *f1 = atoi(token);
    
    token = strtok_r(NULL, ":", &saveptr);
    if (!token) return false;
    *f2 = atoi(token);
    
    token = strtok_r(NULL, ":", &saveptr);
    if (!token) return false;
    *a = atof(token);
    
    token = strtok_r(NULL, ":", &saveptr);
    if (!token) return false;
    *b = atof(token);
    
    token = strtok_r(NULL, ":", &saveptr);
    if (!token) return false;
    *eps = atof(token);
    
    token = strtok_r(NULL, ":", &saveptr);
    if (!token) return false;
    *expected = atof(token);
    
    return true;
}

bool parse_test_integral_params(const char* params, int* f, double* a, double* b, double* eps, double* expected) {
    if (!params || !f || !a || !b || !eps || !expected) {
        return false;
    }
    
    // Создаем локальную копию с ограниченным размером
    char copy[MAX_PARAM_LEN];
    if (strlen(params) >= MAX_PARAM_LEN) {
        return false;
    }
    strncpy(copy, params, MAX_PARAM_LEN - 1);
    copy[MAX_PARAM_LEN - 1] = '\0';
    
    // Парсим токены
    char* saveptr = NULL; // Для потокобезопасного strtok_r
    char* token = strtok_r(copy, ":", &saveptr);
    if (!token) return false;
    *f = atoi(token);
    
    token = strtok_r(NULL, ":", &saveptr);
    if (!token) return false;
    *a = atof(token);
    
    token = strtok_r(NULL, ":", &saveptr);
    if (!token) return false;
    *b = atof(token);
    
    token = strtok_r(NULL, ":", &saveptr);
    if (!token) return false;
    *eps = atof(token);
    
    token = strtok_r(NULL, ":", &saveptr);
    if (!token) return false;
    *expected = atof(token);
    
    return true;
}

// Очистка ресурсов в CommandLineOptions
void free_command_line_options(CommandLineOptions* opts) {
    if (opts) {
        free(opts->test_root_params);
        free(opts->test_integral_params);
        opts->test_root_params = NULL;
        opts->test_integral_params = NULL;
    }
}

// Освобождение памяти для массива опций
void free_options(Option* options, int count) {
    for (int i = 0; i < count; i++) {
        free_option(&options[i]);
    }
}

// Функция main была удалена, чтобы избежать конфликта с main в integral.c