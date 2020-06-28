#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum blockCategory {
	NONE,
	DIRT,
	STONE,
	WOOD,
	LEAVES
} blockCategory;

void          blockTypeInit();
const char   *blockTypeGetName (uint8_t b);
int           blockTypeGetHP   (uint8_t b);
blockCategory blockTypeGetCat  (uint8_t b);
bool          blockTypeValid   (uint8_t b);
