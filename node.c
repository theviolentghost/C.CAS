#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    NODE_CONSTANT,
    NODE_VARIABLE,
    NODE_FUNCTION,
    NODE_OPERATOR,
} NodeType;

typedef struct Node Node;

struct Node {
    NodeType type;
    union {
        struct { // for constants
            float value;
        } constant; 

        struct { // for variables
            char name;
        } variable;  

        struct { // for operators
            char operation;
            Node* left;
            Node* right;
            void (*order)(Node*);
        } operator;

        struct { //for functions
            char name[5];
            Node* input;
            float base; //base of functon like roots, logs, etc
        } function;
    } data;
    float (*evaluate)(Node*, float*);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// util

void freeNode(Node* node) {
    if (node) {
        if (node->type == NODE_OPERATOR) {
            freeNode(node->data.operator.left);
            freeNode(node->data.operator.right);
        }
        free(node);
    }
}

int gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

void floatToFraction(float value, int* numerator, int* denominator, int max_denominator) {
    int sign = (value < 0) ? -1 : 1;
    value = fabs(value);

    // Initialize variables for the continued fraction algorithm
    int lower_numerator = 0, lower_denominator = 1;
    int upper_numerator = 1, upper_denominator = 0;

    while (1) {
        int middle_numerator = lower_numerator + upper_numerator;
        int middle_denominator = lower_denominator + upper_denominator;

        if (middle_denominator > max_denominator) {
            break;
        }

        if (middle_numerator / (float)middle_denominator < value) {
            lower_numerator = middle_numerator;
            lower_denominator = middle_denominator;
        } else {
            upper_numerator = middle_numerator;
            upper_denominator = middle_denominator;
        }

        // Stop if we get a close approximation
        if (fabs(value - (middle_numerator / (float)middle_denominator)) < 1e-6) {
            *numerator = sign * middle_numerator;
            *denominator = middle_denominator;
            return;
        }
    }

    // Choose the closer fraction between lower and upper bounds
    if (fabs(value - (lower_numerator / (float)lower_denominator)) <
        fabs(value - (upper_numerator / (float)upper_denominator))) {
        *numerator = sign * lower_numerator;
        *denominator = lower_denominator;
    } else {
        *numerator = sign * upper_numerator;
        *denominator = upper_denominator;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constant

float Constant_evaluate(Node* node, float* vars);

Node* Constant(float value) {
    Node* node = malloc(sizeof(Node));
    if (node) {
        node->type = NODE_CONSTANT;
        node->data.constant.value = value;
        node->evaluate = Constant_evaluate;
    }
    return node;
}

float Constant_evaluate(Node* node, float* vars) {
    return node->data.constant.value;
}
char* Constant_toString(Node* node, bool fraction) {
    char* result = (char*)malloc(20 * sizeof(char));

    if(fraction) {
        int numerator, denominator;
        const int maxDenominator = 10000;
        floatToFraction(node->data.constant.value, &numerator, &denominator, maxDenominator);

        snprintf(result, 20, "%d/%d", numerator, denominator);
    } else {
        snprintf(result, 20, "%.2f", node->data.constant.value);
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variable

float Variable_evaluate(Node* node, float* vars);

Node* Variable(char name) {
    Node* node = malloc(sizeof(Node));
    if (node) {
        node->type = NODE_VARIABLE;
        node->data.variable.name = name;
        node->evaluate = Variable_evaluate;
    }
    return node;
}

float Variable_evaluate(Node* node, float* vars) {
    return vars[node->data.variable.name - 'a']; 
}
char* Variable_toString() {
    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Operator

float Operator_evaluate(Node* node, float* vars);

Node* Operator(char operation, Node* left, Node* right) {
    Node* node = malloc(sizeof(Node));
    if (node) {
        node->type = NODE_OPERATOR;
        node->data.operator.operation = operation;
        node->data.operator.left = left;
        node->data.operator.right = right;
        node->evaluate = Operator_evaluate;
    }
    return node;
}

float Operator_evaluate(Node* node, float* vars) {
    float left = node->data.operator.left->evaluate(node->data.operator.left, vars);
    float right = node->data.operator.right->evaluate(node->data.operator.right, vars);
    switch (node->data.operator.operation) {
        case '+': return left + right;
        case '-': return left - right;
        case '*': return left * right;
        case '/': return right != 0 ? left / right : 0.0;
        //case '^': return (float)pow((double)left, (double)right); //no no work
    }
    return 0.0;
}
char* Operator_toString() {
    
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    float variables[26]= {0};
    variables['x' - 'a'] = 78;

    Node* root = Operator('/', Constant(5), Variable('x'));
    printf("%f\n", root->evaluate(root, variables));

    Node* c = Constant(.1285548112);
    printf("%s", Constant_toString(c, true));

    return 0;
}
