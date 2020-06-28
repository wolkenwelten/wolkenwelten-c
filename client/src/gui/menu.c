#include "menu.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gfx/textMesh.h"
#include "../misc/options.h"
#include "../tmp/cto.h"
#include "../main.h"

#include <stdbool.h>
#include <string.h>

textMesh *menuM;
char *menuError = "";

void initMenu(){
	menuM = textMeshNew();
	menuM->tex = tGui;
}

void updateMenu(){
	if(!textInputActive && (textInputLock == 2)){
		char *buf = textInputGetBuffer();
		strncpy(serverName,buf,sizeof(serverName)-1);
		textInputLock = 0;
		gameRunning = true;
		hideMouseCursor();
	}

	if(mouseLClicked){
		if((mousex > (screenWidth-256-32)) && (mousex < screenWidth-32)){
			if((mousey > 32) && (mousey < 64)){
				singleplayer = true;
				gameRunning = true;
				textInputLock = 0;
				textInputActive = false;
				hideMouseCursor();
			} else if((mousey > 80) && (mousey < 112)){
				if(!textInputActive){
					textInput(8,screenHeight-24,256,16,2);
				}
			} else if((mousey > 130) && (mousey < 162)){
				quit=true;
			}
		}
	}
}

void menuBackground(){
	float u = 8.f/16.f*128.f;
	float v = 8.f/32.f*128.f;
	float s = 1.f/32.f*128.f;
	int x = 0;
	int y = 0;
	int w = screenWidth;
	int h = screenHeight;




	textMeshAddVert(menuM,x,y,u  ,v  ,0xFFFFAF63);
	textMeshAddVert(menuM,x,h,u  ,v+s,0xFFFF6825);
	textMeshAddVert(menuM,w,h,u+s,v+s,0xFFFF6825);

	textMeshAddVert(menuM,w,h,u+s,v+s,0xFFFF6825);
	textMeshAddVert(menuM,w,y,u+s,v  ,0xFFFFAF63);
	textMeshAddVert(menuM,x,y,u  ,v  ,0xFFFFAF63);
}

void renderMenu(){
	shaderBind(sTextMesh);
	shaderMatrix(sTextMesh,matOrthoProj);
	if(mouseHidden){
		showMouseCursor();
	}
	updateMouse();
	updateMenu();
	textMeshEmpty(menuM);
	menuBackground();

	textMeshAddStrPS(menuM,32,32,4,"Wolkenwelten");
	textMeshPrintfPS(menuM,32,72,2,"Pre-Alpha %s [%.8s]",VERSION,COMMIT);
	textMeshBox(menuM,
		128, 96,
		64, 64,
		7.f/8.f, 2.f/8.f,
		1.f/8.f, 1.f/8.f,
		0xFFFFFFFF);

	textMeshBox(menuM,
		screenWidth-256-32, 32,
		256, 32,
		8.f/16.f, 4.f/16.f,
		1.f/16.f, 1.f/16.f,
		0xFF555555);
	textMeshAddStrPS(menuM,screenWidth-256-32,40,2,"  Singleplayer");

	textMeshBox(menuM,
		screenWidth-256-32, 80,
		256, 32,
		8.f/16.f, 4.f/16.f,
		1.f/16.f, 1.f/16.f,
		0xFF555555);
	textMeshAddStrPS(menuM,screenWidth-256-32,88,2,"  Multiplayer");

	textMeshBox(menuM,
		screenWidth-256-32, 130,
		256, 32,
		8.f/16.f, 4.f/16.f,
		1.f/16.f, 1.f/16.f,
		0xFF555555);
	textMeshAddStrPS(menuM,screenWidth-256-32,138,2,"      Quit");

	if(textInputActive && (textInputLock == 2)){
		textMeshAddStrPS(menuM,8,screenHeight-42,2,"Servername:");
	}
	if((menuError != NULL) && (*menuError != 0)){
		textMeshAddStrPS(menuM,8,screenHeight-58,2,menuError);
	}

	textMeshDraw(menuM);
	textInputDraw();
	drawCursor();
}

