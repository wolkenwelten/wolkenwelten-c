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
#include "boundaries.h"
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/texture.h"
#include "../gfx/shader.h"
#include "../voxel/chunk.h"

#include <stdio.h>
#include <stdlib.h>
#include "../gfx/gl.h"

int drawBoundariesStyle = 0;

static void drawBoundariesReal(const vertexFlat *data, uint dataCount){
	static uint vao = 0;
	static uint vbo = 0;
	static uint vboSize = 0;

	shaderBind(sBoundary);
	matMul      (matMVP,matView,matProjection);
	shaderMatrix(sBoundary,matMVP);

	if(vao == 0){glGenVertexArrays(1, &vao);}
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	if(vbo == 0){glGenBuffers(1, &vbo);}
	glBindBuffer(GL_ARRAY_BUFFER,vbo);

	if(gfxUseSubData && (vboSize >= dataCount)){
		glBufferSubData(GL_ARRAY_BUFFER, 0, dataCount*sizeof(vertex),  data);
	}else{
		glBufferData(GL_ARRAY_BUFFER, dataCount*sizeof(vertex),  data, GL_DYNAMIC_DRAW);
		vboSize = dataCount;
	}
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertexFlat), (void *)(((char *)&data[0].x) - ((char *)data)));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertexFlat), (void *)(((char *)&data[0].rgba) - ((char *)data)));

	glDrawArrays(GL_LINES,0,dataCount);
}

void drawBoundaries(int size){
	static vertexFlat *buf = NULL;
	if(buf == NULL){
		buf = malloc((1 << 18) * sizeof(vertexFlat));
	}
	uint bufi = 0;

	const int range = (int)renderDistance * 2;
	const int steps = range / size;
	const int sx = ((int)player->pos.x) / size * size;
	const int sy = ((int)player->pos.y) / size * size;
	const int sz = ((int)player->pos.z) / size * size;

	for(int x = -steps; x <= steps; x++){
	for(int y = -steps; y <= steps; y++){
		const int cx = sx + x * size;
		const int cy = sy + y * size;
		buf[bufi++] = (vertexFlat){cx,cy,sz - range,0xFF00FFFF};
		buf[bufi++] = (vertexFlat){cx,cy,sz + range,0xFF00FFFF};
		if(bufi >= (1 << 18)){
			fprintf(stderr,"Boundary overflow!\n");
			return;
		}
	}
	}

	for(int x = -steps; x <= steps; x++){
	for(int z = -steps; z <= steps; z++){
		const int cx = sx + x * size;
		const int cz = sz + z * size;
		buf[bufi++] = (vertexFlat){cx,sy - range,cz,0xFFFF00FF};
		buf[bufi++] = (vertexFlat){cx,sy + range,cz,0xFFFF00FF};
		if(bufi >= (1 << 18)){
			fprintf(stderr,"Boundary overflow!\n");
			return;
		}
	}
	}

	for(int y = -steps; y <= steps; y++){
	for(int z = -steps; z <= steps; z++){
		const int cy = sy + y * size;
		const int cz = sz + z * size;
		buf[bufi++] = (vertexFlat){sx - range,cy,cz,0xFFFFFF00};
		buf[bufi++] = (vertexFlat){sx + range,cy,cz,0xFFFFFF00};
		if(bufi >= (1 << 18)){
			fprintf(stderr,"Boundary overflow!\n");
			return;
		}
	}
	}

	drawBoundariesReal(buf,bufi);
}
