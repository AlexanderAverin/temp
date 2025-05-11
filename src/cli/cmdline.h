#ifndef CMDLINE_H
#define CMDLINE_H

#include <stdbool.h>

typedef struct {
    char short_name;
    char* full_name;
    char* description;
    bool is_requires_arg;
} Option;

typedef struct {
    bool help;
    bool show_roots;
    bool show_iterations;
    bool test_root;
    bool test_integral;
    char* test_root_params;
    char* test_integral_params;
} CommandLineOptions;

Option create_option(char short_name, const char* full_name, const char* description, bool is_requires_arg);
void free_option(Option* opt);
void print_help(Option* options, int count_of_options);
CommandLineOptions parse_args(int argc, char* argv[], Option* options, int count_of_options);
bool parse_test_root_params(const char* params, int* f1, int* f2, double* a, double* b, double* eps, double* expected);
bool parse_test_integral_params(const char* params, int* f, double* a, double* b, double* eps, double* expected);
void free_command_line_options(CommandLineOptions* opts);
void free_options(Option* options, int count);

#endif