#include "entity.h"

#include "../main.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>

entity entityList[1<<14];
int entityCount = 0;
entity *entityFirstFree = NULL;
