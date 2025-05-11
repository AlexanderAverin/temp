#include <stdlib.h>
#include <math.h>
#include "ast.h"

// Создает узел константы
Node* create_constant_node(double value) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->type = NODE_CONSTANT;
        node->constant_value = value;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

// Создает узел переменной (x)
Node* create_variable_node(void) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->type = NODE_VARIABLE;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

// Создает узел бинарной операции
Node* create_binary_op_node(OperationType op, Node* left, Node* right) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->type = NODE_BINARY_OP;
        node->op = op;
        node->left = left;
        node->right = right;
    }
    return node;
}

// Создает узел унарной операции
Node* create_unary_op_node(OperationType op, Node* operand) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->type = NODE_UNARY_OP;
        node->op = op;
        node->left = operand;
        node->right = NULL;
    }
    return node;
}

// Освобождает память, занятую AST
void free_ast(Node* root) {
    if (root) {
        free_ast(root->left);
        free_ast(root->right);
        free(root);
    }
}

// Клонирует AST
Node* clone_ast(Node* root) {
    if (!root) return NULL;
    
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return NULL;
    
    new_node->type = root->type;
    
    switch (root->type) {
        case NODE_CONSTANT:
            new_node->constant_value = root->constant_value;
            break;
        case NODE_VARIABLE:
            break;
        case NODE_BINARY_OP:
        case NODE_UNARY_OP:
            new_node->op = root->op;
            break;
        default:
            // Обработка неизвестных типов узлов
            fprintf(stderr, "Warning: Unknown node type in clone_ast\n");
            break;
    }
    
    new_node->left = clone_ast(root->left);
    new_node->right = clone_ast(root->right);
    
    return new_node;
}

// Вычисляет производную выражения по правилам дифференцирования
Node* derive_ast(Node* root) {
    if (!root) return NULL;
    
    switch (root->type) {
        case NODE_CONSTANT:
            return create_constant_node(0.0); // d/dx(const) = 0
            
        case NODE_VARIABLE:
            return create_constant_node(1.0); // d/dx(x) = 1
            
        case NODE_BINARY_OP:
            switch (root->op) {
                case OP_ADD: // d/dx(f + g) = df/dx + dg/dx
                    return create_binary_op_node(OP_ADD, 
                        derive_ast(root->left), 
                        derive_ast(root->right));
                    
                case OP_SUB: // d/dx(f - g) = df/dx - dg/dx
                    return create_binary_op_node(OP_SUB, 
                        derive_ast(root->left), 
                        derive_ast(root->right));
                    
                case OP_MUL: // d/dx(f * g) = df/dx * g + f * dg/dx
                    return create_binary_op_node(OP_ADD, 
                        create_binary_op_node(OP_MUL, derive_ast(root->left), clone_ast(root->right)),
                        create_binary_op_node(OP_MUL, clone_ast(root->left), derive_ast(root->right)));
                    
                case OP_DIV: { // d/dx(f / g) = (df/dx * g - f * dg/dx) / g^2
                    // (df/dx * g)
                    Node* term1 = create_binary_op_node(OP_MUL, 
                        derive_ast(root->left), 
                        clone_ast(root->right));
                    
                    // (f * dg/dx)
                    Node* term2 = create_binary_op_node(OP_MUL,
                        clone_ast(root->left),
                        derive_ast(root->right));
                    
                    // (df/dx * g - f * dg/dx)
                    Node* numerator = create_binary_op_node(OP_SUB, term1, term2);
                    
                    // g^2 = g * g
                    Node* denominator = create_binary_op_node(OP_MUL,
                        clone_ast(root->right),
                        clone_ast(root->right));
                    
                    // ((df/dx * g - f * dg/dx) / g^2)
                    return create_binary_op_node(OP_DIV, numerator, denominator);
                }
                
                case OP_POW:
                    // Упрощенный подход: проверяем, является ли один из операндов константой
                    if (root->left->type == NODE_CONSTANT) {
                        // Случай c^g: d/dx(c^g) = c^g * ln(c) * dg/dx
                        double c = root->left->constant_value;
                        
                        // c^g
                        Node* c_pow_g = clone_ast(root);
                        
                        // ln(c) как константа
                        double ln_c = log(c);
                        Node* ln_c_node = create_constant_node(ln_c);
                        
                        // dg/dx
                        Node* dg_dx = derive_ast(root->right);
                        
                        // c^g * ln(c) * dg/dx
                        return create_binary_op_node(OP_MUL,
                            create_binary_op_node(OP_MUL, c_pow_g, ln_c_node),
                            dg_dx);
                            
                    } else if (root->right->type == NODE_CONSTANT) {
                        // Случай f^c: d/dx(f^c) = c * f^(c-1) * df/dx
                        double c = root->right->constant_value;
                        
                        // c
                        Node* c_node = create_constant_node(c);
                        
                        // f^(c-1)
                        Node* f_pow_c_minus_1 = create_binary_op_node(OP_POW,
                            clone_ast(root->left),
                            create_constant_node(c - 1.0));
                            
                        // df/dx
                        Node* df_dx = derive_ast(root->left);
                        
                        // c * f^(c-1) * df/dx
                        return create_binary_op_node(OP_MUL,
                            create_binary_op_node(OP_MUL, c_node, f_pow_c_minus_1),
                            df_dx);
                    } else {
                        // Общий случай f^g слишком сложен
                        // Вернем просто 0 как упрощение
                        fprintf(stderr, "Warning: General case of derivative for f^g not implemented\n");
                        return create_constant_node(0.0);
                    }
                
                default:
                    return create_constant_node(0.0); // неизвестная операция
            }
            
        case NODE_UNARY_OP:
            switch (root->op) {
                case OP_SIN: // d/dx(sin(f)) = cos(f) * df/dx
                    return create_binary_op_node(OP_MUL,
                        create_unary_op_node(OP_COS, clone_ast(root->left)),
                        derive_ast(root->left));
                    
                case OP_COS: // d/dx(cos(f)) = -sin(f) * df/dx
                    return create_binary_op_node(OP_MUL,
                        create_binary_op_node(OP_MUL,
                            create_constant_node(-1.0),
                            create_unary_op_node(OP_SIN, clone_ast(root->left))),
                        derive_ast(root->left));
                    
                case OP_TAN: { // d/dx(tan(f)) = sec^2(f) * df/dx = (1/cos^2(f)) * df/dx
                    // cos(f)
                    Node* cos_f = create_unary_op_node(OP_COS, clone_ast(root->left));
                    
                    // cos^2(f)
                    Node* cos_squared = create_binary_op_node(OP_MUL, cos_f, clone_ast(cos_f));
                    
                    // 1/cos^2(f)
                    Node* sec_squared = create_binary_op_node(OP_DIV, 
                        create_constant_node(1.0), 
                        cos_squared);
                    
                    // (1/cos^2(f)) * df/dx
                    return create_binary_op_node(OP_MUL, sec_squared, derive_ast(root->left));
                }
                
                case OP_CTG: { // d/dx(cot(f)) = -csc^2(f) * df/dx = -(1/sin^2(f)) * df/dx
                    // sin(f)
                    Node* sin_f = create_unary_op_node(OP_SIN, clone_ast(root->left));
                    
                    // sin^2(f)
                    Node* sin_squared = create_binary_op_node(OP_MUL, sin_f, clone_ast(sin_f));
                    
                    // 1/sin^2(f)
                    Node* csc_squared = create_binary_op_node(OP_DIV, 
                        create_constant_node(1.0), 
                        sin_squared);
                    
                    // -1 * (1/sin^2(f))
                    Node* neg_csc_squared = create_binary_op_node(OP_MUL,
                        create_constant_node(-1.0),
                        csc_squared);
                    
                    // -csc^2(f) * df/dx
                    return create_binary_op_node(OP_MUL, neg_csc_squared, derive_ast(root->left));
                }
                
                default:
                    return create_constant_node(0.0); // неизвестная операция
            }
            
        default:
            return create_constant_node(0.0); // неизвестный тип узла
    }
}