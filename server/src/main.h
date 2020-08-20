#pragma once
#include <stdint.h>
#include <stdbool.h>

extern bool quit;
extern char *termColors[16];
extern char *termReset;

uint64_t getMillis();
