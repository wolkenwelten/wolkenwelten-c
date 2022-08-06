/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "widgets.h"

#include "../repl.h"
#include "../../gfx/textMesh.h"
#include "../../../../common/src/misc/misc.h"

static void widgetDrawAutocomplete(textMesh *m,int x,int y,int size){
	if(lispAutoCompleteCompleteSymbol){return;}
	int cy = y - size*8 - size*4 - size*2;
	const u32 c1[3] = {0xB0808080,0xB0909090,0xC0B86030};
	const u32 c2[3] = {0xD0606060,0xD0A0A0A0,0xE0A05028};
	for(uint i=0;i<lispAutoCompleteLen;i++){
		u32 gc = 0xFFF8F0EA;
		int c  = i&1;
		if((int)i == lispAutoCompleteSelection){
			c  = 2;
			gc = 0xFF3950F0;
		}
		textMeshVGradient(m, x-size*8, cy-size, size*18*8,size*8+size*2,c1[c],c2[c]);
		const lSymbol *sym = lispAutoCompleteList[i];
		int cx = x;
		for(uint ii=0;(ii<sizeof(lSymbol)) && (sym->c[ii] != 0);ii++){
			textMeshAddGlyph(m, cx, cy, size, sym->c[ii], gc, 0x00000000);
			cx += size*8;
		}
		cy -= size*8+size*2;
		if(cy < 0){break;}
	}
}

void widgetDrawLispLine(textMesh *m, const box2D area, int size, const char *rawLine, int lambda, int mark, int cursor){
	const int x = area.x;
	const int y = area.y;
	const int w = area.w;
	const int h = area.h;

	int openParens = 0, cx = x, cy = y, oldFont = m->font;;
	u32 cfgc = 0xFFFFFFFF;
	u8 c;
	m->font = 1;

	int selMin = MIN(mark,cursor),selMax = MAX(mark,cursor);
	if(selMin < 0){selMax = -1;}

	static u32 colors[16] = {
		0x00000000,
		0x60FF8040,
		0x608040FF,
		0x6040FF80,
		0x6040F0F0,
		0x6080FF40,
		0x604080FF,
		0x60FF40FF,

		0x60FFFFFF,
		0xA0FF8040,
		0xA08040FF,
		0xA040FF80,
		0xA040F0F0,
		0xA080FF40,
		0xA04080FF,
		0xA0FF40FF
	};
	if(lambda){
		if(lambda == 2){
			textMeshVGradient(m, cx-size*4, y-size*4, size*8*3 ,size*8*2,0xA01030FF,0x800410E0);
			textMeshVGradient(m, cx-size*4-2+size*8*3, y-size*4, 2 ,size*8*2,0x800410E0,0xA01030FF);
		}
		textMeshAddGlyph(m, cx, y, size, 20, colors[1] | 0xFF000000, 0x00000000);
		cx += size*8;
		textMeshAddGlyph(m, cx, y, size, '>', colors[1] | 0xFF000000, 0x00000000);
		cx += size*8;
		cx += size*8;
	}
	int selActive = 0;
	for(const char *line = rawLine;*line != 0;line++){
		const int curPos = line - rawLine;
		if((lispAutoCompleteLen > 0) && (cursor >= 0)){
			if(lispAutoCompleteStart == line-rawLine){
				widgetDrawAutocomplete(m,cx,y,size);
			}
		}
		if(curPos >= selMax){
			selActive = 0;
		}else if((curPos >= selMin) && (curPos < selMax)){
			selActive = 8;
		}
		if(((u8)line[0] == 0xCE) && ((u8)line[1] == 0xBB)){ // UTF-8 λ
			line++;
			c = 20;
		}else if(((u8)line[0] == 0xCE) && ((u8)line[1] == 0xB4)){ // UTF-8 δ
			line++;
			c = 19;
		}else if(((u8)line[0] == 0xCF) && ((u8)line[1] == 0x89)){ // UTF-8 ω
			line++;
			c = 18;
		}else if((u8)line[0] == '\n'){
			cy += size * 8;
			cx = x;
			continue;
		}else if(*line == '\033'){
			int fgc = -1, bgc = -1;;
			line += parseAnsiCode(line,&fgc,&bgc) - 1;
			if(fgc >= 0){cfgc = colorPalette[fgc];}
			//if(bgc >= 0){m->bgc = colorPalette[bgc];}
			continue;
		}else{
			c = *line;
		}
		if((*line == '[') || (*line == '(')){
			openParens++;
			if(cx < w){
				textMeshAddGlyphHG(m, cx, cy, size, c, cfgc, colors[((openParens-1)&7) | selActive],colors[(openParens&7) | selActive]);
			}
		}else if((*line == ']') || (*line == ')')){
			if(cx < w){
				textMeshAddGlyphHG(m, cx, cy, size, c, cfgc, colors[((openParens)&7) | selActive],colors[((openParens-1)&7) | selActive]);
			}
			openParens--;
		}else if(cx < w){
			textMeshAddGlyph(m, cx, cy, size, c, cfgc, colors[(openParens&7) | selActive]);
		}
		cx += size * 8;
	}
	for(;openParens > 0;openParens--){
		if(cx > w){break;}
		textMeshAddGlyphHG(m, cx, cy, size, ']', cfgc & 0x7FFFFFFF, colors[(openParens)&7] >> 1,colors[(openParens-1)&7] >> 1);
		cx += size * 8;
	}
	m->font = oldFont;

	if((cursor >= 0) && (lispAutoCompleteDescription[0]!=0)){
		textMeshVGradient(m, x-size*4, y+size*12, w+size*4 ,size*16,0xB0808080,0xD0606060);
		widgetDrawLispLine(m, rect(x, y+size*16, w, h), size,lispAutoCompleteDescription , 0, -1, -1);
	}
}
