#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum error_code_e {
    ERR_OK,
    ERR_COULD_NOT_READ,
    ERR_EMPTY_INPUT,
    ERR_UNEXPECTED_CHAR,
    ERR_UNKNOWN_OPERATOR,
    ERR_UNEXPECTED_END
} error_code_t;

typedef enum state_e {
    EXPECTING_DIGIT,
    EXPECTING_OPERATOR
} state_t;

error_code_t operate(long *result, long number, char operator) {
    error_code_t rc = ERR_OK;
    switch (operator) {
        case '+':
            *result += number;
            break;
        case '-':
            *result -= number;
            break;
        case '*':
            *result *= number;
            break;
        default:
           rc = ERR_UNKNOWN_OPERATOR;
           break;
    }
    return rc;
}

int main() {
    char input[1024] = {0};

    char *control_ptr = fgets(input, 1023, stdin);
    if (NULL == control_ptr) {
        fprintf(stderr, "Error while reading from input\n");
        return ERR_COULD_NOT_READ;
    }

    if (strlen(input) == 0) {
        fprintf(stderr, "Empty input\n");
    }

    long result = 0, number = 0;
    state_t state = EXPECTING_DIGIT;
    char operator = '+';
    char *end_ptr = input;
    error_code_t rc = ERR_OK;

    for (char *current_ptr = input; *current_ptr != '\0' && rc == ERR_OK;
            /*shifting the pointer manually inside*/) {
        if (isspace(*current_ptr)) {
            /*do nothing*/
            current_ptr++;
            continue;
        }
        switch (state) {
            case EXPECTING_DIGIT:
                if (*current_ptr >= '0' && *current_ptr <= '9') {
                    number = strtol(current_ptr, &end_ptr, 10);
                    rc = operate(&result, number, operator);
                    if (rc != ERR_OK) {
                        fprintf(stderr, "Unknown operator: %c\n", operator);
                        return rc;
                    }
                    state = EXPECTING_OPERATOR;
                    current_ptr = end_ptr;
                } else {
                    fprintf(stderr, "Expected digit, got: %c\n", *current_ptr);
                    return ERR_UNEXPECTED_CHAR;
                }
                break;
            case EXPECTING_OPERATOR:
                if (*current_ptr == '+' || *current_ptr == '-' ||
                    *current_ptr == '*') {
                    operator = *current_ptr;
                    state = EXPECTING_DIGIT;
                    current_ptr++;
                } else {
                    fprintf(stderr, "Expected operator, got: %c\n", *current_ptr);
                    return ERR_UNEXPECTED_CHAR;
                }
                break;
        }
    }

    if (state == EXPECTING_DIGIT) {
        fprintf(stderr, "Unexpected end\n");
        return ERR_UNEXPECTED_END;
    }

    if (rc == ERR_OK) {
        printf("Result: %ld\n", result);
    }

    return rc;
}
