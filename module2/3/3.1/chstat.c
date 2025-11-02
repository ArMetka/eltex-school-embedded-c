#include "./chstat.h"
#include <stdio.h>

int main(int argc, char **argv) {
    char filename[256];
    char mode[64];

    if (readArgs(argc, argv, filename, 256, mode, 64) != 1) {
        return 1;
    }

    if (filename[0] == '\0' && mode[0] != '\0') { // convert mode to binary (case 1)
        return !doConvert(mode);
    } else if (filename[0] != '\0' && mode[0] == '\0') { // stat (case 2)
        return !doStat(filename);
    } else if (filename[0] != '\0' && mode[0] != '\0') { // chmod (case 3)
        return !doChmod(filename, mode);
    } else { // shouldn't be here
        printf("Invalid arguments!\n%s\n", USAGE_PROMPT);
        return 1;
    }

    return 0;
}

int readArgs(int argc, char **argv, char *filename, int filename_len, char *mode, int mode_len) {
    *filename = '\0';
    *mode = '\0';

    if (argc < 2) {
        printf("Missing filename or mode!\n%s\n", USAGE_PROMPT);
        return 0;
    } else if (argc > 4) {
        printf("Too many arguments!\n%s\n", USAGE_PROMPT);
        return 0;
    } else {
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] == '-' && argv[i][1] != '-') {
                switch (argv[i][1]) {
                    case 'f':
                        if (i == argc - 1) {
                            printf("Missing filename for option \'-f\'!\n%s\n", USAGE_PROMPT);
                            return 0;
                        } else {
                            strncpy(filename, argv[++i], filename_len);
                        }
                        break;
                    default:
                        printf("Unknown option: \'%c\'\n%s\n", argv[i][1], USAGE_PROMPT);
                        return 0;
                }
            } else {
                strncpy(mode, argv[i], mode_len);
            }
        }
    }

    return 1;
}

int doConvert(const char *mode) {
    unsigned int mode_new;

    if (!calculateNewMode(mode, 0, &mode_new)) {
        return 0;
    }

    printMode(mode_new);

    return 1;
}

int doStat(const char *filename) {
    unsigned int mode = 0;

    if (!getStat(filename, &mode)) {
        return 0;
    }

    printMode(mode);

    return 1;
}

int doChmod(const char *filename, const char *mode) {
    unsigned int mode_old = 0;
    unsigned int mode_new = 0;

    if (!getStat(filename, &mode_old)) {
        return 0;
    }

    if (!calculateNewMode(mode, mode_old, &mode_new)) {
        return 0;
    }

    printf("(old) ");
    printMode(mode_old);
    printf("(new) ");
    printMode(mode_new);

    return 1;
}

int calculateNewMode(const char *mode, unsigned int mode_old, unsigned int *mode_new) {
    *mode_new = 0;

    if (mode[0] <= '7' && mode[0] >= '0') {
        // parse as number (755)
        unsigned int mode_tmp;
        if (sscanf(mode, "%u", &mode_tmp) != 1) {
            printf("Invalid mode format!\n");
            return 0;
        }
        *mode_new = ((mode_tmp / 100) << 6) + ((mode_tmp / 10 % 10) << 3) + mode_tmp % 10;
    } else if (mode[0] == 'r' || mode[0] == '-') {
        // parse as string (rwxr-xr-x)
        for (int i = 0; i < 9; i++) {
            if (((i % 3 == 0) && mode[i] == 'r') || ((i % 3 == 1) && mode[i] == 'w') || ((i % 3 == 2) && mode[i] == 'x')) {
                *mode_new = *mode_new | (1 << (8 - i));
            } else if (mode[i] == '-') {
                continue; // 0
            } else {
                printf("Invalid mode format!\n");
                return 0;
            }
        }
    } else if (mode[0] == 'u' || mode[0] == 'g' || mode[0] == 'o' || mode[0] == 'a') {
        // parse as string (u+rx,g+x,o-r)
        for (int i = 0; mode[i] != '\0'; mode_old = *mode_new) {
            unsigned int users_mask = 0;
            unsigned int access_mask = 0;
            char op = '\0';

            for (; mode[i] != '+' && mode[i] != '-' && mode[i] != '='; i++) {
                switch (mode[i]) {
                    case 'u':
                        users_mask |= U_MASK;
                        break;
                    case 'g':
                        users_mask |= G_MASK;
                        break;
                    case 'o':
                        users_mask |= O_MASK;
                        break;
                    case 'a':
                        users_mask |= A_MASK;
                        break;
                    default:
                        printf("Invalid mode format!\n");
                        return 0;
                }
            }

            op = mode[i++];

            for (; mode[i] != '\0' && mode[i] != ','; i++) {
                switch (mode[i]) {
                    case 'r':
                        access_mask |= R_MASK;
                        break;
                    case 'w':
                        access_mask |= W_MASK;
                        break;
                    case 'x':
                        access_mask |= X_MASK;
                        break;
                    default:
                        printf("Invalid mode format!\n");
                        return 0;
                }
            }

            switch (op) {
                case '+':
                    *mode_new = mode_old | (users_mask & access_mask);
                    break;
                case '-':
                    *mode_new = mode_old & ~(users_mask & access_mask);
                    break;
                case '=':
                    *mode_new = (mode_old & ~users_mask) | (users_mask & access_mask);
                    break;
                default:
                    printf("Invalid mode format!\n");
                    return 0;
            }

            if (mode[i++] == '\0') {
                return 1;
            }
        }
    } else {
        printf("Invalid mode format!\n");
        return 0;
    }

    return 1;
}

int getStat(const char *filename, unsigned int *mode) {
    struct stat stats;

    if (stat(filename, &stats) != 0) {
        printf("Failed to get file's stat!\n");    
        return 0;
    }
    *mode = stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO); // rwx for user, group and others (9 last bits)

    return 1;
}

void printMode(unsigned int mode) {
    char mode_string[16], mode_binary[16];
    modeToString(mode, mode_string);
    modeToBinary(mode, mode_binary);

    printf("Access: %u%u%u/%s (%s)\n",
        (mode & S_IRWXU) >> 6,
        (mode & S_IRWXG) >> 3,
        mode & S_IRWXO,
        mode_string,
        mode_binary
    );
}

void modeToString(unsigned int mode, char *string) {
    sprintf(string, "%c%c%c%c%c%c%c%c%c",
        mode & S_IRUSR ? 'r' : '-',
        mode & S_IWUSR ? 'w' : '-',
        mode & S_IXUSR ? 'x' : '-',
        mode & S_IRGRP ? 'r' : '-',
        mode & S_IWGRP ? 'w' : '-',
        mode & S_IXGRP ? 'x' : '-',
        mode & S_IROTH ? 'r' : '-',
        mode & S_IWOTH ? 'w' : '-',
        mode & S_IXOTH ? 'x' : '-'
    );
}

void modeToBinary(unsigned int mode, char *string) {
    for (int i = 8; i >= 0; i--) {
        string[i] = mode & (1 << (8 - i)) ? '1' : '0';
    }
    string[9] = '\0';
}
