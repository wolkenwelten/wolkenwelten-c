#include "animal.h"

#include "../main.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>

animal  animalList[1<<10];
int     animalCount = 0;
animal *animalFirstFree = NULL;

animal *animalNew(float x, float y, float z , int type){
	animal *e = NULL;
	if(animalFirstFree == NULL){
		e = &animalList[animalCount++];
	}else{
		e = animalFirstFree;
		animalFirstFree = e->nextFree;
		if(animalFirstFree == e){
			animalFirstFree = NULL;
		}
	}
	animalReset(e);
	
	e->x          = x;
	e->y          = y;
	e->z          = z;
	e->yaw        = 0.f;
	e->pitch      = 0.f;
	e->roll       = 0.f;

	e->flags      = 0;
	e->type       = type;
	
	e->nextFree   = NULL;
	e->curChungus = NULL;
	
	return e;
}

void animalFree(animal *e){
	if(e == NULL){return;}
	e->nextFree = animalFirstFree;
	animalFirstFree = e;
	if(e->nextFree == NULL){
		e->nextFree = e;
	}
}

void animalUpdateAll(){
	for(int i=0;i<animalCount;i++){
		if(animalList[i].nextFree != NULL){ continue; }
		if(!(animalList[i].flags & ANIMAL_UPDATED)){
			animalUpdate(&animalList[i]);
		}
		animalList[i].flags &= ~ANIMAL_UPDATED;
	}
}
