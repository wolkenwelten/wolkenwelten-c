#define _DEFAULT_SOURCE
#include "widget.h"

#include "gui.h"
#include <string.h>

void drawButton(textMesh *m, const char *label, int state, int x, int y, int w, int h){
	uint32_t color  = 0xFF555555;
	uint32_t tcolor = 0xFF777777;
	uint32_t bcolor = 0xFF333333;
	int textYOff    = (h - (2*8))/2;
	int textXOff    = (w-(strnlen(label,w/16)*16))/2;

	if(state == 0){
		if(mouseInBox(x,y,w,h)){
			if(mouseClicked[0]){
				state = 2;
			}else{
				state = 1;
			}
		}
	}

	if(state == 1){
		color = 0xFF444444;
	}else if(state == 2){
		color = 0xFF2A2A2A;
		textXOff+=1;
		textYOff+=1;
		int tmp = tcolor;
		tcolor = bcolor;
		bcolor = tmp;
	}

	textMeshBox(m,
		x+1, y+1,
		w-1, h-1,
		19.f/32.f, 31.f/32.f,
		 1.f/32.f,  1.f/32.f,
		color);

	textMeshBox(m,
		x, y,
		w, 1,
		19.f/32.f, 31.f/32.f,
		 1.f/32.f,  1.f/32.f,
		tcolor);
	textMeshBox(m,
		x, y,
		1, h,
		19.f/32.f, 31.f/32.f,
		 1.f/32.f,  1.f/32.f,
		tcolor);

	textMeshBox(m,
		x, y+h,
		w, 1,
		19.f/32.f, 31.f/32.f,
		 1.f/32.f,  1.f/32.f,
		bcolor);
	textMeshBox(m,
		x+w, y,
		1, h,
		19.f/32.f, 31.f/32.f,
		 1.f/32.f,  1.f/32.f,
		bcolor);

	textMeshAddStrPS(m,x+textXOff,y+textYOff,2,label);
}

bool mouseInBox(int x, int y, int w, int h){
	if(mousex < x  ){return false;}
	if(mousex > x+w){return false;}
	if(mousey < y  ){return false;}
	if(mousey > y+h){return false;}
	return true;
}
