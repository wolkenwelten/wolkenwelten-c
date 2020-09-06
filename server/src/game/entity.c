#include "entity.h"

#include "../main.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>

entity entityList[1<<14];
int entityCount = 0;
entity *entityFirstFree = NULL;

entity *entityNew(float x, float y, float z , float yaw, float pitch, float roll){
	entity *e = NULL;
	if(entityFirstFree == NULL){
		e = &entityList[entityCount++];
	}else{
		e = entityFirstFree;
		entityFirstFree = e->nextFree;
		if(entityFirstFree == e){
			entityFirstFree = NULL;
		}
	}
	entityReset(e);
	e->x          = x;
	e->y          = y;
	e->z          = z;
	e->yaw        = yaw;
	e->pitch      = pitch;
	e->roll       = roll;
	e->nextFree   = NULL;
	e->curChungus = NULL;
	return e;
}

void entityFree(entity *e){
	if(e == NULL){return;}
	e->nextFree = entityFirstFree;
	entityFirstFree = e;
	if(e->nextFree == NULL){
		e->nextFree = e;
	}
}

void entityUpdateAll(){
	for(int i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(!entityList[i].updated){
			entityUpdate(&entityList[i]);
		}
		entityList[i].updated = false;
	}
}
