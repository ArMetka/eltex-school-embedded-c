#ifndef CHSTAT_H
#define CHSTAT_H

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#define USAGE_PROMPT "Usage: ./chstat [MODE] [-f FILE]"

int readArgs(int argc, char **argv, char *filename, int filename_len, char *mode, int mode_len);
// int doConvert();
int doStat(const char *filename);
// int doChmod();
void printMode(unsigned int mode);
void modeToString(unsigned int mode, char *string);
void modeToBinary(unsigned int mode, char *string);

#endif