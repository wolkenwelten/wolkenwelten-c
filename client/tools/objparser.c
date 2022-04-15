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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	float x,y,z;
} vertexPos;

typedef struct {
	float u,v;
} vertexTex;

typedef struct {
	float x,y,z;
	float u,v;
} vertex;

vertexTex vertTex[8192];
int vertTexCount=1;

vertexPos vertPos[8192];
int vertPosCount=1;

vertex verts[8192];
int vertCount=0;

uint16_t indices[8192];
int indexCount=0;

char name[256];

FILE *cfp,*hfp;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define NORM(v,min,max,scale) ((v-min)/(max-min)*scale)
#define U8NORM(v,min,max) ((uint8_t)NORM(v,min,max,255))
#define U16NORM(v,min,max) ((uint16_t)NORM(v,min,max,65535))

bool vertexEquals(const vertex *a, const vertex *b){
	return a->x == b->x && a->y == b->y && a->z == b->z && a->u == b->u && a->v == b->v;
}

void parseLine(char *line){
	if(*line == 0){return;}
	if(*line == '\n'){return;}
	if(*line == '#'){return;}

	if(*line == 'v'){
		if(line[1] == ' '){
			sscanf(line+2,"%f %f %f",&vertPos[vertPosCount].x,&vertPos[vertPosCount].y,&vertPos[vertPosCount].z);
			vertPosCount++;
		}else if(line[1] == 'n'){

		}else if(line[1] == 't'){
			sscanf(line+2,"%f %f",&vertTex[vertTexCount].u,&vertTex[vertTexCount].v);
			vertTexCount++;
		}
	}
	if(*line == 'f'){
		int indices[9];
		sscanf(line+2,"%i/%i/%i %i/%i/%i %i/%i/%i",&indices[0],&indices[1],&indices[2],&indices[3],&indices[4],&indices[5],&indices[6],&indices[7],&indices[8]);
		verts[vertCount].x = vertPos[indices[0]].x;
		verts[vertCount].y = vertPos[indices[0]].y;
		verts[vertCount].z = vertPos[indices[0]].z;
		verts[vertCount].u = vertTex[indices[1]].u;
		verts[vertCount].v = 1.f-vertTex[indices[1]].v;
		vertCount++;

		verts[vertCount].x = vertPos[indices[3]].x;
		verts[vertCount].y = vertPos[indices[3]].y;
		verts[vertCount].z = vertPos[indices[3]].z;
		verts[vertCount].u = vertTex[indices[4]].u;
		verts[vertCount].v = 1.f-vertTex[indices[4]].v;
		vertCount++;

		verts[vertCount].x = vertPos[indices[6]].x;
		verts[vertCount].y = vertPos[indices[6]].y;
		verts[vertCount].z = vertPos[indices[6]].z;
		verts[vertCount].u = vertTex[indices[7]].u;
		verts[vertCount].v = 1.f-vertTex[indices[7]].v;
		vertCount++;
	}
}

void parseObj(char *obj){
	char *sol = obj;
	while(*obj != 0){
		if(*obj == '\n'){
			*obj = 0;
			parseLine(sol);
			sol = obj+1;
		}
		obj++;
	}
	parseLine(sol);
}

void indexObj(){
	static const uint16_t NotRemapped = 0xFFFF;
	static uint16_t vertexRemap[8192];
	static vertex reduced[8192];
	for(int i=0; i<sizeof(vertexRemap)/sizeof(vertexRemap[0]); ++i){
		vertexRemap[i] = NotRemapped;
	}

	int reducedVertCount=0;
	for(int v=0; v<vertCount; ++v){
		vertex current = verts[v];
		// Find a vertex with same data (at least 1 should, obviously)
		for(int candidate=0; candidate<=v; ++candidate){
			if(vertexEquals(&verts[candidate], &current)){
				// If the equal vertex hasn't been added to the reduced vertex buffer, do so
				if(vertexRemap[candidate] == NotRemapped){
					vertexRemap[candidate] = reducedVertCount;
					reduced[reducedVertCount] = verts[candidate];
					++reducedVertCount;
				}
				indices[v] = vertexRemap[candidate];
				break;
			}
		}
	}

	indexCount = vertCount;
	vertCount = reducedVertCount;
	memcpy(verts, reduced, sizeof(verts));
}

void loadObj(char *file){
	char *buf;
	FILE *fh = fopen(file,"r");
	size_t filesize = 0;
	size_t read = 0;
	if(fh == NULL){return;}

	fseek(fh,0,SEEK_END);
	filesize = ftell(fh);
	fseek(fh,0,SEEK_SET);
	buf = malloc(filesize+1);
	read = 0;
	while(read < filesize){
		read += fread(buf+read,1,filesize-read,fh);
	}
	buf[filesize]=0;
	fclose(fh);

	char *nameEnd=file;
	char *nameStart=file;
	for(;*nameEnd != '.';nameEnd++){
		if(*nameEnd == '/'){
			nameStart = nameEnd+1;
		}
	}

	char *s;
	for(s=name;nameStart != nameEnd;nameStart++){
			*s++ = *nameStart;
	}
	*s = 0;
	parseObj(buf);
	indexObj();
	free(buf);
}

float cacheEfficiency(int capacity, const uint16_t *indices, int indexCount){
	static const uint16_t CacheMaxCapacity = 128;
	uint16_t cache[CacheMaxCapacity];
	for(int i=0; i<capacity; ++i){
		cache[i] = 0xFFFF;
	}
	int misses=0, hits=0;
	for(int i=0; i<indexCount; ++i){
		uint16_t index = indices[i];
		// Is vertex in cache?
		uint16_t cachePosition = 0xFFFF;
		for(int j=0; j<capacity; ++j){
			if(cache[j] == index){
				cachePosition = j;
				break;
			}
		}
		if(cachePosition == 0xFFFF){
			// Cache miss, shift everything right and add to front, and record miss
			memmove(cache + 1, cache, (capacity - 1) * sizeof(cache[0]));
			cache[0] = index;
			++misses;
		}else {
			// Cache hit, move to front & shift inbetween items, and record hit
			if(cachePosition != 0){
				memmove(cache + 1, cache, cachePosition * sizeof(cache[0]));
			}
			cache[0] = index;
			++hits;
		}
	}
	return 1.f - ((float)(misses) / (hits + misses));
}

void printObj(){
	int i;

	// Determine bounding box
	vertexPos min = { 0.f, 0.f, 0.f }, max = { 0.f, 0.f, 0.f };
	for(i=0;i<vertCount;i++){
		const vertex v = verts[i];
		min.x = MIN(min.x, v.x);
		min.y = MIN(min.y, v.y);
		min.z = MIN(min.z, v.z);
		max.x = MAX(max.x, v.x);
		max.y = MAX(max.y, v.y);
		max.z = MAX(max.z, v.z);
	}

	// Output vertex data with coords normalized to the bounding box's space, as u16
	fprintf(cfp,"static const assetVertex %s_verts[] = {\n",name);
	for(i=0;i<vertCount;i++){
		if(i != 0){
			fprintf(cfp," ,");
		}else{
			fprintf(cfp,"  ");
		}
		fprintf(cfp,"{%i,%i,%i,%i,%i}\n",
			U8NORM(verts[i].x, min.x, max.x),
			U8NORM(verts[i].y, min.y, max.y),
			U8NORM(verts[i].z, min.z, max.z),
			U8NORM(verts[i].u, -1.25f, 1.25f),
			U8NORM(verts[i].v, -1.25f, 1.25f)
		);
	}
	fprintf(cfp,"};\n");
	// Check for < 256 instead of <= 256, because on primitive restart is always enabled on WebGL,
	// and 0xFF/0xFFFF are always used as restart indices on u8/u16 index buffers respectively.
	const char *indexType = vertCount < 256 ? "u8" : "u16";
	const char *indexTypeEnum = vertCount < 256 ? "meshIndexTypeU8" : "meshIndexTypeU16";
	fprintf(cfp,"static const %s %s_indices[] = {\n",indexType,name);
	for(i=0;i<indexCount;i++){
		fprintf(cfp,i == 0 ? "%i" : ", %i", indices[i]);
	}
	fprintf(cfp,"};\n");

	fprintf(cfp,
		"assetMeshdata %s_meshdata = {\n"
		"\t.bbox = { .min = { %f, %f, %f }, .max = { %f, %f, %f } },\n"
		"\t.vertexData = %s_verts,\n"
		"\t.vertexCount = %i,\n"
		"\t.indexData = { .%s = %s_indices },\n"
		"\t.indexCount = %i,\n"
		"\t.indexType = %s\n"
		"};\n",
		name,
		min.x, min.y, min.z, max.x, max.y, max.z,
		name,
		vertCount,
		indexType,
		name,
		indexCount,
		indexTypeEnum
	);

	fprintf(hfp,"extern assetMeshdata %s_meshdata;\n",name);
}

int main(int argc,char *argv[]){
	int i;

	if(argc < 2){
		fprintf(stderr,"Usage: ./objparser file.obj...\n");
		return 1;
	}
	cfp = fopen("client/src/tmp/meshAssets.c","w");
	fprintf(cfp,"#include \"meshAssets.h\"\n\n");
	hfp = fopen("client/src/tmp/meshAssets.h","w");
	fprintf(hfp,"#pragma once\n");
	fprintf(hfp,"#include \"../gfx/mesh.h\"\n\n");
	for(i=1;i<argc;i++){
		vertTexCount=1;
		vertPosCount=1;
		vertCount=0;
		indexCount=0;
		loadObj(argv[i]);
		/*
		printf("%32s: %4d verts & %4d indices (%.1fx), cache hits %.1f%% %.1f%% %.1f%% %.1f%%\n",
			argv[i],
			vertCount,
			indexCount,
			(float)(indexCount)/vertCount,
			cacheEfficiency(16, indices, indexCount)*100,
			cacheEfficiency(32, indices, indexCount)*100,
			cacheEfficiency(64, indices, indexCount)*100,
			cacheEfficiency(128, indices, indexCount)*100
			);
		*/
		printObj();
	}
	fclose(cfp);
	fclose(hfp);
	return 0;
}
