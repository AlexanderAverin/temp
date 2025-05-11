#ifndef AST_H
#define AST_H

#include <stdio.h>  // Для fprintf

// Use camelCase with "_" case

typedef enum {
    NODE_CONSTANT,
    NODE_VARIABLE,
    NODE_BINARY_OP,
    NODE_UNARY_OP
} NodeType;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_POW,  // Добавлен оператор возведения в степень
    OP_SIN,
    OP_COS,
    OP_TAN,
    OP_CTG
} OperationType;

typedef struct Node {
    NodeType type;
    union {
        double constant_value;  // для NODE_CONSTANT
        OperationType op;       // для NODE_BINARY_OP и NODE_UNARY_OP
    };
    struct Node *left;          // левый операнд для бинарной операции или единственный для унарной
    struct Node *right;         // правый операнд для бинарной операции, NULL для унарной
} Node;

// Функции для работы с AST
Node* create_constant_node(double value);
Node* create_variable_node(void);
Node* create_binary_op_node(OperationType op, Node* left, Node* right);
Node* create_unary_op_node(OperationType op, Node* operand);
void free_ast(Node* root);
Node* clone_ast(Node* root);
Node* derive_ast(Node* root);  // Для вычисления производной

#endif