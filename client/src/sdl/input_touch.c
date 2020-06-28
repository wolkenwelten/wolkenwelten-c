#include "input_touch.h"

#include "../game/character.h"
#include "../gui/inventory.h"
#include "../main.h"

#include <SDL.h>

bool touchzonesPressed[4];
int touchzonesFingers[4];

int leftFingerID = -1;
float leftFingerX = 0;
float leftFingerY = 0;

int rightFingerID = -1;
float rightFingerX = 0;
float rightFingerY = 0;

void doTouchupdate(float *vx,float *vy,float *vz){
	if(leftFingerID != -1){
		float xoff = leftFingerX*2;
		float yoff = (leftFingerY-0.5f)*2;
		if(xoff < 0.f){xoff = 0.f;}
		if(xoff > 1.f){xoff = 1.f;}
		if(yoff < 0.f){yoff = 0.f;}
		if(yoff > 1.f){yoff = 1.f;}
		xoff = (xoff - 0.5f)*2.f;
		yoff = (yoff - 0.5f)*2.f;
		*vx = xoff;
		*vz = yoff;
	}
	if(rightFingerID != -1){
		float xoff = (rightFingerX-0.5f)*2;
		float yoff = (rightFingerY-0.5f)*2;
		if(xoff < 0.f){xoff = 0.f;}
		if(xoff > 1.f){xoff = 1.f;}
		if(yoff < 0.f){yoff = 0.f;}
		if(yoff > 1.f){yoff = 1.f;}
		xoff = (xoff - 0.5f)*2.f;
		yoff = (yoff - 0.5f)*2.f;

		if(!isInventoryOpen()){
			characterRotate(player,(float)(xoff*5),(float)(yoff*5),0.f);
		}

	}
	if(touchzonesPressed[0]){
		*vy = 1.f;
	}
	if(touchzonesPressed[1]){
		touchzonesPressed[1] = false;
		characterFireHook(player);
	}
}

bool touchSneak(){
	return false;
}
bool touchMine(){
	return touchzonesPressed[2];
}
bool touchActivate(){
	return touchzonesPressed[3];
}

void touchEventHandler(const SDL_Event *e){
	switch(e->type){
		case SDL_FINGERDOWN:
			if(e->tfinger.y > 0.5f){
				if(e->tfinger.x < 0.5f){
					if(leftFingerID == -1){
						leftFingerID = e->tfinger.fingerId;
						leftFingerX = e->tfinger.x;
						leftFingerY = e->tfinger.y;
					}
				}else{
					if(rightFingerID == -1){
						rightFingerID = e->tfinger.fingerId;
						rightFingerX = e->tfinger.x;
						rightFingerY = e->tfinger.y;
					}
				}
			}else{
				touchzonesPressed[(int)(e->tfinger.x*4)] = true;
				touchzonesFingers[(int)(e->tfinger.x*4)] = e->tfinger.fingerId;
			}
		break;

		case SDL_FINGERMOTION:
			if(leftFingerID == e->tfinger.fingerId){
				leftFingerX = e->tfinger.x;
				leftFingerY = e->tfinger.y;
			}else if(rightFingerID == e->tfinger.fingerId){
				rightFingerX = e->tfinger.x;
				rightFingerY = e->tfinger.y;
			}

		break;

		case SDL_FINGERUP:
			if(leftFingerID == e->tfinger.fingerId){
				leftFingerID = -1;
			}else if(rightFingerID == e->tfinger.fingerId){
				rightFingerID = -1;
			}
			for(int i=0;i<4;i++){
				if(e->tfinger.fingerId == touchzonesFingers[i]){
					touchzonesPressed[i] = false;
					touchzonesFingers[i] = -1;
				}
			}
		break;
	}
}

