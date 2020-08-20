#pragma once

#define PI (3.1415926535897932384626433832795f)

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

struct sfx;
typedef struct sfx sfx;
struct bgm;
typedef struct bgm bgm;

#include "network/packet.h"
#include "gfx_structs.h"
#include "game_structs.h"
