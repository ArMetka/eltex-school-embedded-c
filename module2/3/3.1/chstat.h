#ifndef CHSTAT_H
#define CHSTAT_H

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#define USAGE_PROMPT "Usage: ./chstat [MODE] [-f FILE]"

#define U_MASK 0b111000000
#define G_MASK 0b000111000
#define O_MASK 0b000000111
#define A_MASK (U_MASK | G_MASK | O_MASK)
#define R_MASK 0b100100100
#define W_MASK 0b010010010
#define X_MASK 0b001001001

int readArgs(int argc, char **argv, char *filename, int filename_len, char *mode, int mode_len);
int doConvert(const char *mode);
int doStat(const char *filename);
int doChmod(const char *filename, const char *mode);
int calculateNewMode(const char *mode, unsigned int mode_old, unsigned int *mode_new);
int getStat(const char *filename, unsigned int *mode);
void printMode(unsigned int mode);
void modeToString(unsigned int mode, char *string);
void modeToBinary(unsigned int mode, char *string);

// supported mode formats:
// - [0-7][0-7][0-7]
// - ([r-][w-][x-]){3}
// - [ugoa]+[-+=]([rwx]){0,3}(","[ugoa]+[-+=]([rwx]){0,3})*

// examples:
// - 755
// - rwxr-xr-x
// - u=rwx,g=rx,o=rx
// - a+x

// if 755:
//     (1)
//     new = ((755 / 100) << 6) + ((755 / 10 % 10) << 3) + 755 % 10
//
// if u=rx:
//     (1)
//     mask of u (111000000)
//     mask of rx (101101101)
//     & (101000000)
//     
//     (2)
//     mask of u (111000000)
//     ~ (000111111)
//     
//     (3)
//     old_stat (.........)
//     2 result (000111111)
//     & (000......)
//
//     (4)
//     3 result (000......)
//     1 result (101000000)
//     | (101......)
//     
// if u+x:
//     (1)
//     mask of u (111000000)
//     mask of x (001001001)
//     & (001000000)
//     
//     (2)
//     old_stat (.........)
//     1 result (001000000)
//     | (..1......)
//
// if u-x:
//     (1)
//     mask of u (111000000)
//     mask of x (001001001)
//     & (001000000)
//     
//     (2)
//     1 result (001000000)
//     ~ (110111111)
//     
//     (3)
//     old_stat (.........)
//     2 result (110111111)
//     & (..0......)

#endif