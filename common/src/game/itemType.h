#pragma once
#include "../common.h"
#include "../nujel/nujel.h"

extern itemType itemTypes[256];

void itemTypeInit();
void itemTypeLispClosure(lClosure *c);
