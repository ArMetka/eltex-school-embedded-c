#include <stdio.h>

#include "./calc.h"

char charp;  // char for scanf

int printMenu() {
    int result = 0;

    printf("\n----Calculator CLI----\n");

    printf("1) Add\n");
    printf("2) Add (bulk)\n");
    printf("3) Multiply\n");
    printf("4) Multiply (bulk)\n");
    printf("5) Subtract\n");
    printf("6) Divide\n");
    printf("0) Exit\n");

    printf("\nselect op: ");
    scanf("%d%c", &result, &charp);

    return result;
}

int getUserArg(const char *prompt, double *result, int allow_blank) {
    char buf[128];
    buf[0] = '\0';

    do {
        if ((buf[0] == '\n') && !allow_blank) {
            printf("value is required!\n");
        } else if ((buf[0] == '\n') && allow_blank) {
            return 0;
        }

        printf("%s", prompt);
        fgets(buf, 128, stdin);
    } while ((!allow_blank && (buf[0] == '\n')) || (sscanf(buf, "%lf", result) != 1));

    return 1;
}

int main() {
    double buf1, buf2;
    char prompt_buf[64];

    while (1 != 0) {
        int count = 0;
        int op = printMenu();
        if (op == 0) {  // exit
            break;
        }

        switch (op) {
            case 1:  // add
                getUserArg("arg1 = ", &buf1, 0);
                getUserArg("arg2 = ", &buf2, 0);
                printf("answer = %.2lf\n", add(2, buf1, buf2));
                break;
            case 2:  // bulk add
                printf("bulk mode: enter \"\\n\" to stop\n");
                buf2 = 0;
                do {
                    if (count != 0) {
                        buf2 = add(2, buf2, buf1);
                    }
                    sprintf(prompt_buf, "arg%d = ", ++count);
                } while (getUserArg(prompt_buf, &buf1, 1));
                printf("answer = %.2lf\n", buf2);
                break;
            case 3:  // mul
                getUserArg("arg1 = ", &buf1, 0);
                getUserArg("arg2 = ", &buf2, 0);
                printf("answer = %.2lf\n", multiply(2, buf1, buf2));
                break;
            case 4:  // bulk mul
                printf("bulk mode: enter \"\\n\" to stop\n");
                buf2 = 1;
                do {
                    if (count != 0) {
                        buf2 = multiply(2, buf2, buf1);
                    }
                    sprintf(prompt_buf, "arg%d = ", ++count);
                } while (getUserArg(prompt_buf, &buf1, 1));
                printf("answer = %.2lf\n", buf2);
                break;
            case 5:  // sub
                getUserArg("arg1 = ", &buf1, 0);
                getUserArg("arg2 = ", &buf2, 0);
                printf("answer = %.2lf\n", subtract(buf1, buf2));
                break;
            case 6:  // div
                getUserArg("arg1 = ", &buf1, 0);
                getUserArg("arg2 = ", &buf2, 0);
                printf("answer = %.2lf\n", divide(buf1, buf2));
                break;
            default:
                printf("Invalid input!\n");
                break;
        }
    }
    return 0;
}
