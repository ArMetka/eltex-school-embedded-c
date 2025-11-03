#ifndef IP4TEST_H
#define IP4TEST_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define USAGE_PROMPT "Usage: ./ip4test GATEWAY[/MASK] [MASK] N"
#define RAND_SEED 1

int readArgs(int argc, char **argv, unsigned int *gateway, unsigned int *netmask, int *n);
int testHitProbability(unsigned int gateway, unsigned int netmask, int n);
int parseAddr4(char *src, unsigned int *result);
void printAddr4(unsigned int addr);

#endif