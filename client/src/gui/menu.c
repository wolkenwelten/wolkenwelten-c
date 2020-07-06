#include "menu.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gfx/textMesh.h"
#include "../network/client.h"
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
	
	if(!textInputActive && (textInputLock == 3)){
		snprintf(playerName,sizeof(playerName),"%s",textInputGetBuffer());
		textInputLock = 0;
	}

	if(mouseLClicked){
		if((mousex > (screenWidth-256-32)) && (mousex < screenWidth-32)){
			if((mousey > 32) && (mousey < 64)){
				#ifndef __EMSCRIPTEN__
				singleplayer    = true;
				gameRunning     = true;
				textInputLock   = 0;
				textInputActive = false;
				hideMouseCursor();
				#endif
			} else if((mousey > 80) && (mousey < 112)){
				if(!textInputActive){
					textInput(8,screenHeight-24,256,16,2);
				}
			} else if((mousey > 130) && (mousey < 162)){
				if(!textInputActive){
					textInput(8,screenHeight-24,256,16,3);
				}
			} else if((mousey > 178) && (mousey < 200)){
				quit=true;
			}
		}
	}
}

void menuBackground(){
	float u = 8.f/16.f*128.f;
	float v = 8.f/32.f*128.f;
	float s = 1.f/32.f*128.f;
	int   x = 0;
	int   y = 0;
	int   w = screenWidth;
	int   h = screenHeight;

	textMeshAddVert(menuM,x,y,u  ,v  ,0xFFFFAF63);
	textMeshAddVert(menuM,x,h,u  ,v+s,0xFFFF6825);
	textMeshAddVert(menuM,w,h,u+s,v+s,0xFFFF6825);

	textMeshAddVert(menuM,w,h,u+s,v+s,0xFFFF6825);
	textMeshAddVert(menuM,w,y,u+s,v  ,0xFFFFAF63);
	textMeshAddVert(menuM,x,y,u  ,v  ,0xFFFFAF63);
}

void renderMenu(){
	int buttonY = 32;
	char playerNameBuf[48];
	
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
	
	snprintf(playerNameBuf,sizeof(playerNameBuf),"Your Name: %s",playerName);
	textMeshAddStrPS(menuM,32,176,2,playerNameBuf);

	#ifndef __EMSCRIPTEN__
	textMeshBox(menuM,
		screenWidth-256-32, buttonY,
		256, 32,
		8.f/16.f, 4.f/16.f,
		1.f/16.f, 1.f/16.f,
		0xFF555555);
	textMeshAddStrPS(menuM,screenWidth-256-32,buttonY+8,2,"  Singleplayer");
	#endif
	buttonY += 32 + 16;

	textMeshBox(menuM,
		screenWidth-256-32, buttonY,
		256, 32,
		8.f/16.f, 4.f/16.f,
		1.f/16.f, 1.f/16.f,
		0xFF555555);
	textMeshAddStrPS(menuM,screenWidth-256-32,buttonY+8,2,"  Multiplayer");
	buttonY += 32 + 16;
	
	textMeshBox(menuM,
		screenWidth-256-32, buttonY,
		256, 32,
		8.f/16.f, 4.f/16.f,
		1.f/16.f, 1.f/16.f,
		0xFF555555);
	textMeshAddStrPS(menuM,screenWidth-256-32,buttonY+8,2,"  Change Name");
	buttonY += 32 + 16;

	textMeshBox(menuM,
		screenWidth-256-32, buttonY,
		256, 32,
		8.f/16.f, 4.f/16.f,
		1.f/16.f, 1.f/16.f,
		0xFF555555);
	textMeshAddStrPS(menuM,screenWidth-256-32,buttonY+8,2,"      Quit");
	buttonY += 32 + 16;

	if(textInputActive && (textInputLock == 2)){
		textMeshAddStrPS(menuM,8,screenHeight-42,2,"Servername:");
	}
	if(textInputActive && (textInputLock == 3)){
		textMeshAddStrPS(menuM,8,screenHeight-42,2,"Your Name:");
	}
	if((menuError != NULL) && (*menuError != 0)){
		textMeshAddStrPS(menuM,8,screenHeight-58,2,menuError);
	}

	textMeshDraw(menuM);
	textInputDraw();
	drawCursor();
}

