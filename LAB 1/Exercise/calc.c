#include <stdio.h>
#include "calc.h"

// Hàm tính toán
double calculate(double num1, char op, double num2) {
    switch (op) {
        case '+': return num1 + num2;
        case '-': return num1 - num2;
        case 'x': // Nhập "x" hoặc "*" đều được
        case '*': return num1 * num2;
        case '/':
            if (num2 == 0) {
                printf("MATH ERROR\n");
                return 0;
            }
            return num1 / num2;
        case '%':
            if ((int)num2 == 0) {
                printf("MATH ERROR\n");
                return 0;
            }
            return (int)num1 % (int)num2;
        default:
            printf("SYNTAX ERROR\n");
            return 0;
    }
}

int main() {
    double num1, num2, result;
    char op;

    while (1) {
        printf(">> ");
        if (scanf("%lf %c %lf", &num1, &op, &num2) != 3) {
            printf("SYNTAX ERROR\n");
            while (getchar() != '\n'); // Xóa buffer input lỗi
            continue;
        }

        result = calculate(num1, op, num2);
        printf("%.2f\n", result);
    }

    return 0;
}
