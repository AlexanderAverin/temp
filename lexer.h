#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include "src/parser/ast.h"  // Для доступа к OperationType

// Типы токенов для польской обратной записи
typedef enum {
    TOKEN_NUMBER,     // Числовая константа (например, 3.14)
    TOKEN_VARIABLE,   // Переменная x
    TOKEN_OPERATOR,   // Операторы: +, -, *, /, ^
    TOKEN_FUNCTION,   // Функции: sin, cos, tan, ctg
    TOKEN_CONSTANT,   // Именованные константы: pi, e
    TOKEN_EOF,        // Конец входной строки
    TOKEN_ERROR       // Ошибка лексического анализа
} TokenType;

// Структура для представления токена
typedef struct {
    TokenType type;         // Тип токена
    union {
        double value;       // Значение для TOKEN_NUMBER
        char name[32];      // Имя для TOKEN_VARIABLE, TOKEN_FUNCTION, TOKEN_CONSTANT
        OperationType op;   // Оператор для TOKEN_OPERATOR
    };
    int line, column;       // Позиция в исходной строке для отчетов об ошибках
} Token;

// Структура лексера
typedef struct {
    const char* input;      // Входная строка
    size_t position;        // Текущая позиция во входной строке
    int line, column;       // Текущая строка и столбец
    Token current_token;    // Текущий токен
} Lexer;

// Функции для работы с лексером
void lexer_init(Lexer* lexer, const char* input);
void lexer_next_token(Lexer* lexer);
const char* token_type_to_string(TokenType type);
const char* token_to_string(Token* token, char* buffer, size_t buffer_size);

#endif // LEXER_H