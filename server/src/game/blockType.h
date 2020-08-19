#pragma once
#include "../../../common/src/common.h"

void          blockTypeInit();
const char   *blockTypeGetName (uint8_t b);
int           blockTypeGetHP   (uint8_t b);
blockCategory blockTypeGetCat  (uint8_t b);
bool          blockTypeValid   (uint8_t b);
