#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

// Инициализирует лексер с заданной входной строкой
void lexer_init(Lexer* lexer, const char* input) {
    lexer->input = input;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    // Получаем первый токен
    lexer_next_token(lexer);
}

// Получение следующего токена из входной строки
void lexer_next_token(Lexer* lexer) {
    // Пропускаем пробелы, табуляции, переносы строк
    while (lexer->input[lexer->position] && isspace(lexer->input[lexer->position])) {
        if (lexer->input[lexer->position] == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        lexer->position++;
    }
    
    // Конец файла
    if (!lexer->input[lexer->position]) {
        lexer->current_token.type = TOKEN_EOF;
        return;
    }
    
    // Запоминаем начальную позицию для отчета об ошибках
    int start_column = lexer->column;
    
    // Число
    if (isdigit(lexer->input[lexer->position]) || 
        (lexer->input[lexer->position] == '.' && isdigit(lexer->input[lexer->position + 1])) ||
        (lexer->input[lexer->position] == '-' && isdigit(lexer->input[lexer->position + 1]))) {
        
        char buffer[256] = {0};
        int i = 0;
        
        if (lexer->input[lexer->position] == '-') {
            buffer[i++] = lexer->input[lexer->position++];
            lexer->column++;
        }
        
        while (isdigit(lexer->input[lexer->position]) || lexer->input[lexer->position] == '.') {
            buffer[i++] = lexer->input[lexer->position++];
            lexer->column++;
        }
        
        lexer->current_token.type = TOKEN_NUMBER;
        lexer->current_token.value = atof(buffer);
        lexer->current_token.line = lexer->line;
        lexer->current_token.column = start_column;
        return;
    }
    
    // Переменная, функция или константа
    if (isalpha(lexer->input[lexer->position])) {
        char buffer[32] = {0};
        int i = 0;
        
        while (isalnum(lexer->input[lexer->position]) || lexer->input[lexer->position] == '_') {
            if (i < sizeof(buffer) - 1) {
                buffer[i++] = lexer->input[lexer->position];
            }
            lexer->position++;
            lexer->column++;
        }
        
        buffer[i] = '\0';
        
        // Проверяем, является ли это переменной 'x'
        if (strcmp(buffer, "x") == 0) {
            lexer->current_token.type = TOKEN_VARIABLE;
            snprintf(lexer->current_token.name, sizeof(lexer->current_token.name), "%s", buffer);
        }
        // Проверяем, является ли это константой (pi, e)
        else if (strcmp(buffer, "pi") == 0 || strcmp(buffer, "e") == 0) {
            lexer->current_token.type = TOKEN_CONSTANT;
            snprintf(lexer->current_token.name, sizeof(lexer->current_token.name), "%s", buffer);
        }
        // Иначе это функция
        else {
            lexer->current_token.type = TOKEN_FUNCTION;
            snprintf(lexer->current_token.name, sizeof(lexer->current_token.name), "%s", buffer);
        }
        
        lexer->current_token.line = lexer->line;
        lexer->current_token.column = start_column;
        return;
    }
    
    // Операторы
    char op = lexer->input[lexer->position++];
    lexer->column++;
    
    lexer->current_token.type = TOKEN_OPERATOR;
    lexer->current_token.line = lexer->line;
    lexer->current_token.column = start_column;
    
    switch (op) {
        case '+': lexer->current_token.op = OP_ADD; break;
        case '-': lexer->current_token.op = OP_SUB; break;
        case '*': lexer->current_token.op = OP_MUL; break;
        case '/': lexer->current_token.op = OP_DIV; break;
        case '^': lexer->current_token.op = OP_POW; break;
        default:
            lexer->current_token.type = TOKEN_ERROR;
            lexer->current_token.name[0] = op;
            lexer->current_token.name[1] = '\0';
    }
}

// Преобразует тип токена в строку (для отладки)
const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_VARIABLE: return "VARIABLE";
        case TOKEN_OPERATOR: return "OPERATOR";
        case TOKEN_FUNCTION: return "FUNCTION";
        case TOKEN_CONSTANT: return "CONSTANT";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// Преобразует токен в строку (для отладки)
const char* token_to_string(Token* token, char* buffer, size_t buffer_size) {
    switch (token->type) {
        case TOKEN_NUMBER:
            snprintf(buffer, buffer_size, "NUMBER(%g)", token->value);
            break;
        case TOKEN_VARIABLE:
            snprintf(buffer, buffer_size, "VARIABLE(%s)", token->name);
            break;
        case TOKEN_OPERATOR:
            switch (token->op) {
                case OP_ADD: snprintf(buffer, buffer_size, "OPERATOR(+)"); break;
                case OP_SUB: snprintf(buffer, buffer_size, "OPERATOR(-)"); break;
                case OP_MUL: snprintf(buffer, buffer_size, "OPERATOR(*)"); break;
                case OP_DIV: snprintf(buffer, buffer_size, "OPERATOR(/)"); break;
                case OP_POW: snprintf(buffer, buffer_size, "OPERATOR(^)"); break;
                default: snprintf(buffer, buffer_size, "OPERATOR(?)"); break;
            }
            break;
        case TOKEN_FUNCTION:
            snprintf(buffer, buffer_size, "FUNCTION(%s)", token->name);
            break;
        case TOKEN_CONSTANT:
            snprintf(buffer, buffer_size, "CONSTANT(%s)", token->name);
            break;
        case TOKEN_EOF:
            snprintf(buffer, buffer_size, "EOF");
            break;
        case TOKEN_ERROR:
            snprintf(buffer, buffer_size, "ERROR(%s)", token->name);
            break;
        default:
            snprintf(buffer, buffer_size, "UNKNOWN");
            break;
    }
    
    return buffer;
}