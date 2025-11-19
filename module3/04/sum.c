#include <stdio.h>

int main(int argc, char **argv) {
    long long int result = 0;

    for (int i = 1; i < argc; i++) {
        long long int input;
        if (sscanf(argv[i], "%lld", &input) == 1) {
            result += input;
        }
    }

    printf("%lld\n", result);

    return 0;
}
