#include "../gfx/texture.h"
#include "../game/blockType.h"
#include "../tmp/assets.h"

#include "../gfx/glew.h"
#include "../gfx/lodepng.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int boundTexture = 0;
int textureCount          = 0;
texture textureList[32];

texture *tBlocks;
texture *tGui;
texture *tCursor;
texture *tItems;
texture *tCrosshair;
texture *tRope;
texture *tBlockMining;

void textureLoadSurface(texture *t, int w, int h, const void *data){
	t->w = w;
	t->h = h;
	if(t->ID == 0){
		glGenTextures(1, &t->ID);
	}
	glBindTexture(GL_TEXTURE_2D, t->ID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void textureLoad(texture *t, const unsigned char *data, const size_t dataLen){
	unsigned char *pixels = NULL;
	int error = lodepng_decode32(&pixels, &t->w, &t->h, data, dataLen);
	if(error){
		fprintf(stderr,"Error decoding PNG\n");
		exit(4);
	}
	if(t->ID == 0){
		glGenTextures(1, &t->ID);
	}
	glBindTexture(GL_TEXTURE_2D, t->ID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
}

texture *textureNew(const unsigned char *data, size_t dataLen){
	texture *tex = &textureList[textureCount++];
	tex->ID = 0;
	textureLoad(tex,data,dataLen);
	return tex;
}

texture *textureNewSurface(int w, int h, const void *data){
	texture *tex = &textureList[textureCount++];
	tex->ID = 0;
	textureLoadSurface(tex,w,h,data);
	return tex;
}

void textureFree(){
	for(int i=0;i<textureCount;i++){
		if(textureList[i].ID == 0){continue;}
		glDeleteTextures(1, &textureList[i].ID);
	}
}

void textureBind(const texture *tex){
	if(boundTexture == tex->ID){return;}
	glBindTexture(GL_TEXTURE_2D, tex->ID);
	glUniform1i(tex->ID ,(GLint) 0);
	boundTexture = tex->ID;
}

uint32_t darken(uint32_t in){
	uint32_t b = (in >> 16) & 0xFF;
	uint32_t g = (in >>  8) & 0xFF;
	uint32_t r = (in      ) & 0xFF;

	r = (((r << 8) / 3) >> 7) & 0xFF;
	g = (((g << 8) / 3) >> 7) & 0xFF;
	b = (((b << 8) / 3) >> 7) & 0xFF;

	return (b<<16) | (g<<8) | (r) | 0xFF000000;
}

uint32_t interpol(uint32_t c1,uint32_t c2,uint32_t c3,uint32_t c4){
	uint32_t b = (((c1>>16)&0xFF) + ((c2>>16)&0xFF) + ((c3>>16)&0xFF) + ((c4>>16)&0xFF)) >> 2;
	uint32_t g = (((c1>> 8)&0xFF) + ((c2>> 8)&0xFF) + ((c3>> 8)&0xFF) + ((c4>> 8)&0xFF)) >> 2;
	uint32_t r = (((c1    )&0xFF) + ((c2    )&0xFF) + ((c3    )&0xFF) + ((c4    )&0xFF)) >> 2;

	return (b<<16) | (g<<8) | (r) | 0xFF000000;
}


void textureBuildBlockIcons(){
	uint32_t *tblocks, *titems;
	unsigned int bw,bh,iw,ih;
	lodepng_decode32((unsigned char **)&tblocks, &bw, &bh, gfx_blocks_png_data, gfx_blocks_png_len);
	lodepng_decode32((unsigned char **)&titems, &iw, &ih, gfx_items_png_data, gfx_items_png_len);

	for(int i=0;i<256;i++){
		if(!blockTypeValid(i)){continue;}
		const int dx = (i & 0x1F) << 5;
		const int dy = (i >> 5)   << 5;
		int sx,sy;
		for(int y=0;y<32;y++){
			for(int x=0;x<32;x++){
				titems[((y+dy)<<10) | (x+dx)] = 0x000000FF;
			}
		}
		sx = blockTypeGetTexX(i,0) << 5;
		sy = blockTypeGetTexY(i,0) << 5;
		for(int y=0;y<16;y++){
			for(int x=0;x<16;x++){
				const uint32_t c = interpol(
					tblocks[((sy+(y<<1)  )<<9) | (sx+(x<<1)  )],
					tblocks[((sy+(y<<1)  )<<9) | (sx+(x<<1)+1)],
					tblocks[((sy+(y<<1)+1)<<9) | (sx+(x<<1)  )],
					tblocks[((sy+(y<<1)+1)<<9) | (sx+(x<<1)+1)]
				);
				titems[((y+dy+8+(x>>1))<<10) | (x+dx)] = darken(c);
			}
		}
		sx = blockTypeGetTexX(i,4) << 5;
		sy = blockTypeGetTexY(i,4) << 5;
		for(int y=0;y<16;y++){
			for(int x=0;x<16;x++){
				const uint32_t c = interpol(
					tblocks[((sy+(y<<1)  )<<9) | (sx+(x<<1)  )],
					tblocks[((sy+(y<<1)  )<<9) | (sx+(x<<1)+1)],
					tblocks[((sy+(y<<1)+1)<<9) | (sx+(x<<1)  )],
					tblocks[((sy+(y<<1)+1)<<9) | (sx+(x<<1)+1)]
				);
				titems[((y+dy+8+(7-(x>>1)))<<10) | (x+dx+16)] = darken(c);
			}
		}
		sx = blockTypeGetTexX(i,2) << 5;
		sy = blockTypeGetTexY(i,2) << 5;
		for(int y=0;y<32;y++){
			for(int x=0;x<32;x++){
				int ndx = 16 + (x>>1) - (y >> 1);
				int ndy = (x>>2) + (y>>2) + (x&1);
				titems[((ndy+dy)<<10) | (ndx+dx)] = tblocks[((sy+y)<<9) | (sx+x)];
			}
		}
	}
	textureLoadSurface(tItems,iw,ih,titems);
	free(tblocks);
	free(titems);
}

void textureInit(){
	tBlocks      = textureNew(gfx_blocks_png_data,    gfx_blocks_png_len    );
	tCursor      = textureNew(gfx_cursor_png_data,    gfx_cursor_png_len    );
	tGui         = textureNew(gfx_gui_png_data,       gfx_gui_png_len       );
	tItems       = textureNew(gfx_items_png_data,     gfx_items_png_len     );
	tCrosshair   = textureNew(gfx_crosshair_png_data, gfx_crosshair_png_len );
	tRope        = textureNew(gfx_rope_png_data,      gfx_rope_png_len      );
	tBlockMining = textureNew(gfx_mining_png_data,    gfx_mining_png_len    );
}
