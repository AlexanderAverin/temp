#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "src/parser/ast.h"
#include "lexer.h"  // Добавляем включение заголовочного файла лексера

// Ограничение на количество константных значений в секции .data
#define MAX_CONSTANTS 100
#define EPSILON 1e-10  // Эпсилон для сравнения чисел с плавающей точкой

// Буфер для хранения констант
double constants[MAX_CONSTANTS];
int const_count = 0;

// Прототипы функций
static int add_constant(double value);
static Node* build_ast_from_rpn(const char *rpn);
static void generate_node_asm_code(FILE *fp, Node* node);
static void generate_function_asm_code(FILE *fp, Node* ast, const char* func_name);
static void generate_asm_code(FILE *fp, Node* f1_ast, Node* f2_ast, Node* f3_ast);

// static void debug_lexer(const char* input);

// Функция для добавления константы в буфер
static int add_constant(double value) {
    // Проверяем, не существует ли уже такая константа
    for (int i = 0; i < const_count; i++) {
        // Безопасное сравнение с плавающей точкой
        if (fabs(constants[i] - value) < EPSILON) {
            return i;
        }
    }
    
    // Добавляем новую константу
    if (const_count < MAX_CONSTANTS) {
        constants[const_count] = value;
        return const_count++;
    }
    
    fprintf(stderr, "Error: Too many constants\n");
    exit(EXIT_FAILURE);
}

// Функция для построения AST из польской обратной записи с использованием лексера
static Node* build_ast_from_rpn(const char *rpn) {
    Lexer lexer;
    lexer_init(&lexer, rpn);
    
    Node* stack[50];  // Стек для узлов
    int top = -1;     // Индекс вершины стека
    
    while (lexer.current_token.type != TOKEN_EOF) {
        switch (lexer.current_token.type) {
            case TOKEN_NUMBER:
                stack[++top] = create_constant_node(lexer.current_token.value);
                break;
                
            case TOKEN_VARIABLE:
                stack[++top] = create_variable_node();
                break;
                
            case TOKEN_CONSTANT:
                if (strcmp(lexer.current_token.name, "pi") == 0) {
                    stack[++top] = create_constant_node(3.14159265358979323846);
                } else if (strcmp(lexer.current_token.name, "e") == 0) {
                    stack[++top] = create_constant_node(2.71828182845904523536);
                } else {
                    fprintf(stderr, "Error: Unknown constant '%s' at line %d, column %d\n",
                            lexer.current_token.name, lexer.current_token.line, lexer.current_token.column);
                    exit(EXIT_FAILURE);
                }
                break;
                
            case TOKEN_OPERATOR:
                if (top < 1) {
                    fprintf(stderr, "Error: Not enough operands for operator at line %d, column %d\n",
                            lexer.current_token.line, lexer.current_token.column);
                    exit(EXIT_FAILURE);
                }
                
                Node* right = stack[top--];
                Node* left = stack[top--];
                
                stack[++top] = create_binary_op_node(lexer.current_token.op, left, right);
                break;
                
            case TOKEN_FUNCTION:
                if (top < 0) {
                    fprintf(stderr, "Error: Not enough operands for function '%s' at line %d, column %d\n",
                            lexer.current_token.name, lexer.current_token.line, lexer.current_token.column);
                    exit(EXIT_FAILURE);
                }
                
                Node* operand = stack[top--];
                
                if (strcmp(lexer.current_token.name, "sin") == 0) {
                    stack[++top] = create_unary_op_node(OP_SIN, operand);
                } else if (strcmp(lexer.current_token.name, "cos") == 0) {
                    stack[++top] = create_unary_op_node(OP_COS, operand);
                } else if (strcmp(lexer.current_token.name, "tan") == 0) {
                    stack[++top] = create_unary_op_node(OP_TAN, operand);
                } else if (strcmp(lexer.current_token.name, "ctg") == 0) {
                    stack[++top] = create_unary_op_node(OP_CTG, operand);
                } else {
                    fprintf(stderr, "Error: Unknown function '%s' at line %d, column %d\n",
                            lexer.current_token.name, lexer.current_token.line, lexer.current_token.column);
                    free_ast(operand);
                    exit(EXIT_FAILURE);
                }
                break;
                
            case TOKEN_ERROR:
                fprintf(stderr, "Error: Unknown token '%s' at line %d, column %d\n",
                        lexer.current_token.name, lexer.current_token.line, lexer.current_token.column);
                exit(EXIT_FAILURE);
                break;
                
            default:
                fprintf(stderr, "Error: Unexpected token type at line %d, column %d\n",
                        lexer.current_token.line, lexer.current_token.column);
                exit(EXIT_FAILURE);
        }
        
        lexer_next_token(&lexer);
    }
    
    if (top != 0) {
        fprintf(stderr, "Error: Invalid RPN expression (too many operands or not enough operators)\n");
        exit(EXIT_FAILURE);
    }
    
    return stack[0];
}

// Генерация ассемблерного кода для узла AST
static void generate_node_asm_code(FILE *fp, Node* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_CONSTANT: {
            int const_idx = add_constant(node->constant_value);
            fprintf(fp, "    fld qword [const%d]\n", const_idx);
            break;
        }
        
        case NODE_VARIABLE:
            fprintf(fp, "    fld qword [ebp + 8]\n");
            break;
            
        case NODE_BINARY_OP:
            // Генерируем код для обоих операндов
            generate_node_asm_code(fp, node->left);
            generate_node_asm_code(fp, node->right);
            
            // Выполняем операцию
            switch (node->op) {
                case OP_ADD:
                    fprintf(fp, "    faddp\n");
                    break;
                case OP_SUB:
                    fprintf(fp, "    fsubp\n");
                    break;
                case OP_MUL:
                    fprintf(fp, "    fmulp\n");
                    break;
                case OP_DIV:
                    fprintf(fp, "    fdivp\n");
                    break;
                case OP_POW:
                    // Для вычисления x^y в FPU: y * log2(x)
                    // Используем свойство логарифмов: x^y = 2^(y * log2(x))
                    
                    // Подготавливаем стек: st0 = y, st1 = x
                    // Меняем порядок, чтобы st0 = x, st1 = y
                    fprintf(fp, "    fxch\n");
                    
                    // Вычисляем log2(x): st0 = log2(x), st1 = y
                    fprintf(fp, "    fld1\n");      // st0 = 1.0, st1 = x, st2 = y
                    fprintf(fp, "    fxch\n");      // st0 = x, st1 = 1.0, st2 = y
                    fprintf(fp, "    fyl2x\n");     // st0 = log2(x), st1 = y
                    
                    // Умножаем log2(x) на y: st0 = y * log2(x)
                    fprintf(fp, "    fmulp\n");     // st0 = y * log2(x)
                    
                    // Вычисляем 2^(y * log2(x))
                    fprintf(fp, "    fld st0\n");   // st0 = y * log2(x), st1 = y * log2(x)
                    fprintf(fp, "    frndint\n");   // st0 = int(y * log2(x)), st1 = y * log2(x)
                    fprintf(fp, "    fxch st1\n");  // st0 = y * log2(x), st1 = int(y * log2(x))
                    fprintf(fp, "    fsub st0, st1\n"); // st0 = frac(y * log2(x)), st1 = int(y * log2(x))
                    fprintf(fp, "    f2xm1\n");     // st0 = 2^frac(y * log2(x)) - 1, st1 = int(y * log2(x))
                    fprintf(fp, "    fld1\n");      // st0 = 1.0, st1 = 2^frac(...) - 1, st2 = int(y * log2(x))
                    fprintf(fp, "    faddp\n");     // st0 = 2^frac(...), st1 = int(y * log2(x))
                    fprintf(fp, "    fscale\n");    // st0 = 2^(y * log2(x)), st1 = int(y * log2(x))
                    fprintf(fp, "    fstp st1\n");  // st0 = 2^(y * log2(x))
                    break;
                default:
                    fprintf(stderr, "Error: Unknown binary operation\n");
                    exit(EXIT_FAILURE);
            }
            break;
            
        case NODE_UNARY_OP:
            // Генерируем код для операнда
            generate_node_asm_code(fp, node->left);
            
            // Выполняем операцию
            switch (node->op) {
                case OP_SIN:
                    fprintf(fp, "    fsin\n");
                    break;
                case OP_COS:
                    fprintf(fp, "    fcos\n");
                    break;
                case OP_TAN:
                    fprintf(fp, "    fptan\n");
                    fprintf(fp, "    fstp st0\n");  // Удаляем значение 1.0, оставленное fptan
                    break;
                case OP_CTG:
                    fprintf(fp, "    fptan\n");
                    fprintf(fp, "    fdivr st0, st1\n");  // Вычисляем котангенс как 1/tg
                    fprintf(fp, "    fstp st1\n");        // Удаляем значение 1.0
                    break;
                default:
                    fprintf(stderr, "Error: Unknown unary operation\n");
                    exit(EXIT_FAILURE);
            }
            break;
            
        default:
            fprintf(stderr, "Error: Unknown node type\n");
            exit(EXIT_FAILURE);
    }
}

// Генерация полного ассемблерного кода для функции
static void generate_function_asm_code(FILE *fp, Node* ast, const char* func_name) {
    fprintf(fp, "%s:\n", func_name);
    fprintf(fp, "    push ebp\n");
    fprintf(fp, "    mov ebp, esp\n");
    
    // Генерируем код для вычисления выражения
    generate_node_asm_code(fp, ast);
    
    // Завершаем функцию
    fprintf(fp, "    pop ebp\n");
    fprintf(fp, "    ret\n\n");
}

// Генерация ассемблерного кода для всех функций
static void generate_asm_code(FILE *fp, Node* f1_ast, Node* f2_ast, Node* f3_ast) {
    // Сначала сбрасываем счетчик констант
    const_count = 0;
    
    // Предварительно обходим все AST, чтобы собрать все используемые константы
    // Создаем временные AST для производных
    Node* df1_ast = derive_ast(f1_ast);
    Node* df2_ast = derive_ast(f2_ast);
    Node* df3_ast = derive_ast(f3_ast);
    
    // Создаем временный файл для сбора констант
    FILE* temp_fp = tmpfile();
    if (!temp_fp) {
        fprintf(stderr, "Error: Could not create temporary file\n");
        free_ast(df1_ast);
        free_ast(df2_ast);
        free_ast(df3_ast);
        exit(EXIT_FAILURE);
    }
    
    // Собираем константы, записывая во временный файл (он не будет использоваться)
    generate_node_asm_code(temp_fp, f1_ast);
    generate_node_asm_code(temp_fp, f2_ast);
    generate_node_asm_code(temp_fp, f3_ast);
    generate_node_asm_code(temp_fp, df1_ast);
    generate_node_asm_code(temp_fp, df2_ast);
    generate_node_asm_code(temp_fp, df3_ast);
    
    fclose(temp_fp);
    
    // Теперь все константы собраны, можно генерировать реальный файл
    
    // Начало файла
    fprintf(fp, "section .data\n");
    
    // Генерируем константы
    for (int i = 0; i < const_count; i++) {
        fprintf(fp, "    const%d dq %lf\n", i, constants[i]);
    }
    
    // Секция с кодом
    fprintf(fp, "\nsection .text\n");
    fprintf(fp, "    global f1\n");
    fprintf(fp, "    global f2\n");
    fprintf(fp, "    global f3\n");
    fprintf(fp, "    global df1\n");
    fprintf(fp, "    global df2\n");
    fprintf(fp, "    global df3\n\n");
    
    // Генерируем код для функций
    generate_function_asm_code(fp, f1_ast, "f1");
    generate_function_asm_code(fp, f2_ast, "f2");
    generate_function_asm_code(fp, f3_ast, "f3");
    
    // Генерируем производные - используем уже созданные AST
    generate_function_asm_code(fp, df1_ast, "df1");
    generate_function_asm_code(fp, df2_ast, "df2");
    generate_function_asm_code(fp, df3_ast, "df3");
    
    // Освобождаем память
    free_ast(df1_ast);
    free_ast(df2_ast);
    free_ast(df3_ast);
}

// Вспомогательная функция для отладки лексера
// void debug_lexer(const char* input) {
//     printf("Debug lexer for input: %s\n", input);
    
//     Lexer lexer;
//     lexer_init(&lexer, input);
    
//     char buffer[100];
//     while (lexer.current_token.type != TOKEN_EOF) {
//         printf("  %s at line %d, column %d\n", 
//                token_to_string(&lexer.current_token, buffer, sizeof(buffer)),
//                lexer.current_token.line, 
//                lexer.current_token.column);
        
//         lexer_next_token(&lexer);
//     }
    
//     printf("  %s\n", token_to_string(&lexer.current_token, buffer, sizeof(buffer)));
// }

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    FILE *input_fp = fopen(argv[1], "r");
    if (!input_fp) {
        fprintf(stderr, "Error: Could not open input file %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    
    // Читаем диапазон
    double a, b;
    if (fscanf(input_fp, "%lf %lf", &a, &b) != 2) {
        fprintf(stderr, "Error: Could not read range from input file\n");
        fclose(input_fp);
        return EXIT_FAILURE;
    }
    
    // Пропускаем оставшуюся часть строки
    char buffer[256];  // Уменьшаем размер буфера
    if (fgets(buffer, sizeof(buffer), input_fp) == NULL) {
        fprintf(stderr, "Error: Could not read end of line\n");
        fclose(input_fp);
        return EXIT_FAILURE;
    }
    
    // Читаем три выражения в польской обратной записи
    char rpn1[256], rpn2[256], rpn3[256];  // Уменьшаем размер буферов
    
    if (!fgets(rpn1, sizeof(rpn1), input_fp)) {
        fprintf(stderr, "Error: Could not read first expression\n");
        fclose(input_fp);
        return EXIT_FAILURE;
    }
    
    if (!fgets(rpn2, sizeof(rpn2), input_fp)) {
        fprintf(stderr, "Error: Could not read second expression\n");
        fclose(input_fp);
        return EXIT_FAILURE;
    }
    
    if (!fgets(rpn3, sizeof(rpn3), input_fp)) {
        fprintf(stderr, "Error: Could not read third expression\n");
        fclose(input_fp);
        return EXIT_FAILURE;
    }
    
    fclose(input_fp);
    
    // Отладка лексера (раскомментируйте для отладки)
    // debug_lexer(rpn1);
    // debug_lexer(rpn2);
    // debug_lexer(rpn3);
    
    // Строим AST для каждого выражения
    Node* f1_ast = build_ast_from_rpn(rpn1);
    Node* f2_ast = build_ast_from_rpn(rpn2);
    Node* f3_ast = build_ast_from_rpn(rpn3);
    
    // Генерируем ассемблерный код
    FILE *output_fp = fopen(argv[2], "w");
    if (!output_fp) {
        fprintf(stderr, "Error: Could not open output file %s\n", argv[2]);
        free_ast(f1_ast);
        free_ast(f2_ast);
        free_ast(f3_ast);
        return EXIT_FAILURE;
    }
    
    generate_asm_code(output_fp, f1_ast, f2_ast, f3_ast);
    
    fclose(output_fp);
    
    // Освобождаем память
    free_ast(f1_ast);
    free_ast(f2_ast);
    free_ast(f3_ast);
    
    printf("Assembly code generated successfully: %s\n", argv[2]);
    return EXIT_SUCCESS;
}