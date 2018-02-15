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
    ERR_UNEXPECTED_END,
    ERR_NO_MATCHING_BRACKET
} error_code_t;

typedef enum state_e {
    EXPECTING_DIGIT,
    EXPECTING_OPERATOR
} state_t;

/**
 * for the multiplication precedence, we're keeping the acumulated
 * multiplications number so far and the operator found before it
 **/
error_code_t operate(long number, long *mult_number,
        char operator, char *mult_operator, long *result) {
    if (operator == '*') {
        /* just multiply the acumulation */
        *mult_number *= number;
    } else if (operator == '+' || operator == '-') {
        /* do the previous operation and start new acumulation */
        if (*mult_operator == '+') {
            *result += *mult_number;
        } else {
            *result -= *mult_number;
        }
        *mult_number = number;
        *mult_operator = operator;
    } else {
        fprintf(stderr, "Unknown operator: '%c'\n", operator);
        return ERR_UNKNOWN_OPERATOR;
    }
    return ERR_OK;
}

error_code_t parse(char *input, char *end_ptr, long *result) {
    long r = 0, mult_number = 1, number = 0;
    state_t state = EXPECTING_DIGIT;
    error_code_t rc = ERR_OK;
    char operator = '*', mult_operator = '+';
    char *number_end;
    for (char *current_ptr = input; current_ptr < end_ptr && rc == ERR_OK;
            /* shifting the pointer manually inside */) {
        if (isspace(*current_ptr)) {
            /* do nothing */
            current_ptr++;
            continue;
        }
        switch (state) {
            case EXPECTING_DIGIT:
                if ((*current_ptr >= '0' && *current_ptr <= '9') ||
                    (*current_ptr == '-' && current_ptr[1] >= '0'
                     && current_ptr[1] <= '9')) {
                    /* parse the number and operate on it */
                    number = strtol(current_ptr, &number_end, 10);
                    rc = operate(number, &mult_number,
                            operator, &mult_operator, &r);
                    if (rc != ERR_OK) {
                        return rc;
                    }
                    current_ptr = number_end;
                    state = EXPECTING_OPERATOR;
                } else if (*current_ptr == '(') {
                    /* find the matching bracket and parse the inside
                     * and return the result, then operate on it */
                    int bracket_depth = 0;
                    char *bracket_finder;
                    char found_bracket = 0;
                    for (bracket_finder = current_ptr + 1;
                            bracket_finder < end_ptr; bracket_finder++) {
                        if (*bracket_finder == '(') {
                            bracket_depth++;
                        } else if (*bracket_finder == ')') {
                            if (bracket_depth > 0) {
                                bracket_depth--;
                            } else {
                                found_bracket = 1;
                                break;
                            }
                        }
                    }
                    if (found_bracket) {
                        rc = parse(current_ptr + 1, bracket_finder, &number);
                        if (rc != ERR_OK) {
                            return rc;
                        }
                        rc = operate(number, &mult_number,
                                operator, &mult_operator, &r);
                        if (rc != ERR_OK) {
                            return rc;
                        }
                        current_ptr = bracket_finder + 1;
                        state = EXPECTING_OPERATOR;
                    } else {
                        fprintf(stderr, "Didn't find matching bracket\n");
                        return ERR_NO_MATCHING_BRACKET;
                    }
                } else {
                    fprintf(stderr, "Expected digit, got: '%c'\n", *current_ptr);
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
                    fprintf(stderr, "Expected operator, got: '%c'\n", *current_ptr);
                    return ERR_UNEXPECTED_CHAR;
                }
                break;
        }
    }

    /* do the operation we were keeping with the acumulated number */
    if (mult_operator == '+') {
        r += mult_number;
    } else if (mult_operator == '-') {
        r -= mult_number;
    } else {
        fprintf(stderr, "Unknown operator: '%c'\n", mult_operator);
        return ERR_UNKNOWN_OPERATOR;
    }

    if (state == EXPECTING_DIGIT) {
        fprintf(stderr, "Unexpected end\n");
        return ERR_UNEXPECTED_END;
    }

    *result = r;
    return ERR_OK;
}

int main() {
    error_code_t rc = ERR_OK;
    char input[1024] = {0};

    char *control_ptr = fgets(input, 1023, stdin);
    if (NULL == control_ptr) {
        fprintf(stderr, "Error while reading from input\n");
        return ERR_COULD_NOT_READ;
    }

    long result = 0;
    rc = parse(input, input + strlen(input), &result);

    if (rc == ERR_OK) {
        printf("Result: %ld\n", result);
    }

    return rc;
}
