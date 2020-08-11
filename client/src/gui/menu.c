#include "menu.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gfx/textMesh.h"
#include "../network/client.h"
#include "../misc/options.h"
#include "../tmp/assets.h"
#include "../tmp/cto.h"
#include "../main.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

textMesh *menuM;
char *menuError = "";

bool showAttribution  = false;
int  attributionLines = 0;
static int gamepadSelection = -1;

void initMenu(){
	menuM = textMeshNew();
	menuM->tex = tGui;
	
	attributionLines = 0;
	unsigned char *s = txt_attribution_txt_data;
	while(*s++ != 0){
		if(*s == '\n'){attributionLines++;}
	}
}

void updateMenu(){
	if(!textInputActive && (textInputLock == 2)){
		char *buf = textInputGetBuffer();
		strncpy(serverName,buf,sizeof(serverName)-1);
		textInputLock   = 0;
		connectionTries = 0;
		gameRunning = true;
		hideMouseCursor();
	}

	if(!textInputActive && (textInputLock == 3)){
		snprintf(playerName,sizeof(playerName),"%s",textInputGetBuffer());
		textInputLock = 0;
	}
}

void startSingleplayer(){
	singleplayer    = true;
	gameRunning     = true;
	textInputLock   = 0;
	textInputActive = false;
	connectionTries = 0;
	hideMouseCursor();
}

void updateMenuClick(int x, int y, int btn){
	if(gameRunning){return;}
	
	if(showAttribution){
		showAttribution = false;
		return;
	}

	if((btn == 1) && (x > (screenWidth-256-32)) && (x < screenWidth-32)){
		if((y > 32) && (y < 64)){
			#ifndef __EMSCRIPTEN__
			startSingleplayer();
			#endif
		} else if((y > 80) && (y < 112)){
			if(!textInputActive){
				textInput(8,screenHeight-24,256,16,2);
			}
		} else if((y > 130) && (y < 162)){
			if(!textInputActive){
				textInput(8,screenHeight-24,256,16,3);
			}
		} else if((y > 178) && (y < 200)){
			showAttribution=true;
		} else if((y > 216) && (y < 248)){
			quit=true;
		}
	}
}

void menuBackground(){
	float u = 19.f/32.f*128.f;
	float v = 31.f/32.f*128.f;
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

void drawMenuAttributions(){
	static int scroll = 0;
	static int scrollDir = 1;
	
	const int textHeight = attributionLines * 16;
	
	if(scroll > textHeight-screenHeight){
		scrollDir=-1;
	}
	if(scroll < 0){
		scrollDir=1;
	}
	scroll+=scrollDir;
	
	textMeshPrintfPS(menuM,16,16-scroll,2,"Attribution:\n%s",txt_attribution_txt_data);
}

void drawMenuButtons(){
	int buttonY = 32;
	char playerNameBuf[48];
	int ci = gamepadSelection;
	
	textMeshAddStrPS(menuM,32,32,4,"Wolkenwelten");
	textMeshPrintfPS(menuM,32,72,2,"Pre-Alpha %s [%.8s]",VERSION,COMMIT);
	textMeshBox(menuM,
		128, 96,
		64, 64,
		26.f/32.f, 31.f/32.f,
		1.f/32.f,  1.f/32.f,
		0xFFFFFFFF);

	snprintf(playerNameBuf,sizeof(playerNameBuf),"Your Name: %s",playerName);
	textMeshAddStrPS(menuM,32,176,2,playerNameBuf);

	#ifndef __EMSCRIPTEN__
	drawButton(menuM,"Singleplayer",ci-- == 0,screenWidth-256-32,buttonY,256,32);
	#else
	ci--;
	#endif
	buttonY += 32 + 16;

	drawButton(menuM,"Multiplayer",ci-- == 0,screenWidth-256-32,buttonY,256,32);
	buttonY += 32 + 16;

	drawButton(menuM,"Change Name",ci-- == 0,screenWidth-256-32,buttonY,256,32);
	buttonY += 32 + 16;
	
	drawButton(menuM,"Attribution",ci-- == 0,screenWidth-256-32,buttonY,256,32);
	buttonY += 32 + 16;

	#ifndef __EMSCRIPTEN__
	drawButton(menuM,"Quit",ci-- == 0,screenWidth-256-32,buttonY,256,32);
	#endif
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
	
	if(showAttribution){
		drawMenuAttributions();
	}else{
		drawMenuButtons();
	}

	textMeshDraw(menuM);
	textInputDraw();
	drawCursor();
}

void updateMenuGamepad(int btn){
	mouseHidden = true;
	
	if(showAttribution){
		showAttribution = false;
		return;
	}
	
	if(btn != 1){return;}
	switch(gamepadSelection){
		case 0:
			#ifndef __EMSCRIPTEN__
			startSingleplayer();
			#endif
		break;
		
		case 1:
			if(!textInputActive){
				textInput(8,screenHeight-24,256,16,2);
			}
		break;
		
		case 2:
			if(!textInputActive){
				textInput(8,screenHeight-24,256,16,3);
			}
		break;
		
		case 3:
			showAttribution=true;
		break;
		
		case 4:
			quit=true;
		break;
	}
}

void changeMenuSelection(int off){
	
	gamepadSelection += off;
	if(gamepadSelection < 0){
		gamepadSelection = 4;
	}
	if(gamepadSelection > 4){
		gamepadSelection = 0;
	}
	//fprintf(stderr,"Gamepadselection: %i\n",gamepadSelection);
}