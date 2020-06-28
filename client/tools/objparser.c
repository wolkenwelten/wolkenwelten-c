#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

char name[256];

FILE *cfp,*hfp;

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
	free(buf);
}

void printObj(){
	int i;

	fprintf(cfp,"vertex %s_verts[] = {\n",name);
	for(i=0;i<vertCount;i++){
		if(i != 0){
			fprintf(cfp," ,");
		}else{
			fprintf(cfp,"  ");
		}
		fprintf(cfp,"{%f,%f,%f,%f,%f,1.f}\n",verts[i].x,verts[i].y,verts[i].z,verts[i].u,verts[i].v);
	}
	fprintf(cfp,"};\n");
	fprintf(cfp,"size_t %s_count = %i;\n\n",name,vertCount);

	fprintf(hfp,"extern vertex %s_verts[];\n",name);
	fprintf(hfp,"extern size_t %s_count;\n\n",name);
}

int main(int argc,char *argv[]){
	int i;

	if(argc < 2){
		fprintf(stderr,"Usage: ./objparser file.obj...\n");
		return 1;
	}
	cfp = fopen("client/src/tmp/meshassets.c","w");
	fprintf(cfp,"#include \"meshassets.h\"\n\n");
	hfp = fopen("client/src/tmp/meshassets.h","w");
	fprintf(hfp,"#pragma once\n");
	fprintf(hfp,"#include \"../gfx/mesh.h\"\n\n");
	for(i=1;i<argc;i++){
		loadObj(argv[i]);
		printObj();
		vertTexCount=1;
		vertPosCount=1;
		vertCount=0;
	}
	fclose(cfp);
	fclose(hfp);
	return 0;
}
