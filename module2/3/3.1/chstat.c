#include "./chstat.h"

int main(int argc, char **argv) {
    char filename[256];
    char mode[64];

    if (readArgs(argc, argv, filename, 256, mode, 64) != 1) {
        return 1;
    }

    if (filename[0] == '\0' && mode[0] != '\0') { // convert mode to binary (case 1)
        //
    } else if (filename[0] != '\0' && mode[0] == '\0') { // stat (case 2)
        return !doStat(filename);
    } else if (filename[0] != '\0' && mode[0] != '\0') { // chmod (case 3)
        //
    } else { // shouldn't be here
        printf("Invalid arguments!\n%s\n", USAGE_PROMPT);
        return 1;
    }

    // if (filename[0] != '\0' && stat(filename, &stats) != 0) {
    //     printf("Failed to get file's stat!\n");
    //     return 1;
    // }


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
            if (argv[i][0] == '-') {
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

int doStat(const char *filename) {
    struct stat stats;
    unsigned int stat_mode = 0;

    if (stat(filename, &stats) != 0) {
        printf("Failed to get file's stat!\n");    
        return 0;
    }
    stat_mode = stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO); // rwx for user, group and others (9 last bits)
    printMode(stat_mode);

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
