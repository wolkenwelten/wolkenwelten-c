#include "animal.h"

#include "../main.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/network/messages.h"

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

void animalDel(int i){
	if(i >= animalCount) {return;}
	if(i < 0)            {return;}
	animalList[i] = animalList[--animalCount];
}

void animalUpdateAll(){
	for(int i=animalCount-1;i>=0;i--){
		if(animalList[i].nextFree != NULL){ continue; }
		animalUpdate(&animalList[i]);
		if(animalList[i].y < -256.f){animalDel(i);}
	}
}

float animalClosestPlayer(animal *e, character **cChar){
	*cChar = NULL;
	float ret = 4096.f;
	for(int i=0;i<clientCount;++i){
		if(clients[i].c == NULL){continue;}
		float dx = clients[i].c->x - e->x;
		float dy = clients[i].c->y - e->y;
		float dz = clients[i].c->z - e->z;
		float d  = (dx*dx)+(dy*dy)+(dz*dz);
		if(d < ret){
			ret = d;
			*cChar = clients[i].c;
		}
	}
	return ret;
}

void animalThink(animal *e){
	character *cChar;
	const float dist = animalClosestPlayer(e,&cChar);
	if((dist < 1024.f) && (cChar != NULL)){
		vec caDist = vecMulS(vecNorm(vecNew(e->x - cChar->x,e->y - cChar->y, e->z - cChar->z)),0.01f);
		e->vx = caDist.x;
		//e->vy = caDist.y;
		e->vz = caDist.z;
	}
}

void animalThinkAll(){
	for(int i=0;i<animalCount;i++){
		if(animalList[i].nextFree != NULL){ continue; }
		animalThink(&animalList[i]);
	}
}

void animalEmptySync(int c){
	packet *rp = &packetBuffer;

	rp->val.u[12] = 0;
	rp->val.u[13] = 0;
	
	packetQueue(rp,30,14*4,c);
}

void animalSync(int c, int i){
	packet *rp = &packetBuffer;
	animal *e = &animalList[i];
	
	rp->val.f[ 0] = e->x;
	rp->val.f[ 1] = e->y;
	rp->val.f[ 2] = e->z;
	rp->val.f[ 3] = e->yaw;
	rp->val.f[ 4] = e->pitch;
	rp->val.f[ 5] = e->roll;
	rp->val.f[ 6] = e->vx;
	rp->val.f[ 7] = e->vy;
	rp->val.f[ 8] = e->vz;
	rp->val.f[ 9] = e->yoff;
	rp->val.u[10] = e->flags;
	rp->val.i[11] = e->type;
	
	rp->val.u[12] = i;
	rp->val.u[13] = animalCount;
	
	packetQueue(rp,30,14*4,c);
}

void animalSyncPlayer(int c){
	if(animalCount == 0){
		animalEmptySync(c);
	}
	for(int i=0;i<animalCount;i++){
		animalSync(c,i);
	}
}

uint8_t *animalSave(animal *e, uint8_t *b){
	float    *f = (float *)   b;
	uint32_t *u = (uint32_t *)b;
	
	if(e      == NULL){return b;}
	
	b[0] = 0x03;
	b[1] = 0;
	b[2] = 0;
	b[3] = 0;
	
	f[ 1] = e->x;
	f[ 2] = e->y;
	f[ 3] = e->z;
	f[ 4] = e->yaw;
	f[ 5] = e->pitch;
	f[ 6] = e->roll;
	f[ 7] = e->vx;
	f[ 8] = e->vy;
	f[ 9] = e->vz;
	f[10] = e->yoff;
	u[11] = e->flags;
	u[12] = e->type;
	
	return b+13*4;
}

uint8_t *animalLoad(uint8_t *b){
	float    *f = (float *)   b;
	uint32_t *u = (uint32_t *)b;
	
	animal *e = animalNew(f[1],f[2],f[3],u[12]);
	if(e == NULL){return b+13*4;}
	e->yaw   = f[ 4];
	e->pitch = f[ 5];
	e->roll  = f[ 6];
	e->vx    = f[ 7];
	e->vy    = f[ 8];
	e->vz    = f[ 9];
	e->yoff  = f[10];
	e->flags = u[11];
	
	return b+13*4;
}

uint8_t *animalSaveChungus(chungus *c,uint8_t *b){
	if(c == NULL){return b;}	
	for(int i=0;i<animalCount;i++){
		if(animalList[i].curChungus != c){continue;}
		b = animalSave(&animalList[i],b);
	}
	return b;
}

void animalDelChungus(chungus *c){
	if(c == NULL){return;}
	for(int i=animalCount-1;i>=0;i--){
		if(animalList[i].curChungus != c){continue;}
		animalDel(i);
	}
}
