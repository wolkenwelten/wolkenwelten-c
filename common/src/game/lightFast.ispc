/* An experiment to see how well we can integrate ISPC
 * code into the existing codebase, as well as how
 * well the generated instructions compare against
 * hand written assembly.
 */

export void lightSunlightChunkISPC(uniform int8 out[48][48][48],uniform const int8 blockData[16][16][16], uniform int8 curLight[16][16], uniform const int8 blockLight[256], uniform const int x, uniform const int y, uniform const int z, uniform const int8 sunlight){
	for(uniform int cy=15;cy>=0;cy--){
	for(uniform int cx=0;cx<16;cx++){
	for(uniform int cz=0;cz<16;cz++){
		uniform const int8 b = blockData[cx][cy][cz];
		if(b){
			curLight[cx][cz] = 0;
			out[x+cx][y+cy][z+cz] = blockLight[b];
		}else{
			curLight[cx][cz] = min(sunlight, curLight[cx][cz]+2);
			out[x+cx][y+cy][z+cz] = curLight[cx][cz];
		}
	}
	}
	}
}

export void lightSunlightAirISPC(uniform int8 out[48][48][48],uniform int8 curLight[16][16],uniform const int x, uniform const int y, uniform const int z, uniform const int8 sunlight){
	for(uniform int cy=15;cy>=0;cy--){
	foreach(cx = 0 ... 16, cz = 0 ... 16){
		curLight[cx][cz] = min(sunlight, curLight[cx][cz]+2);
		out[cx+x][cy+y][cz+z] = curLight[cx][cz];
	}
	}
}

static void lightBlurZ(uniform int8 out[48][48][48]){
	for(uniform int x=0;x < 48;x++){
	for(uniform int y=0;y < 48;y++){
	uniform int8 a = 0;
	for(uniform int z=0;z < 48;z++){
		a = max(out[x][y][z], a);
                out[x][y][z] = a;
		a = max(a - 2, 0);
	}

	a = 0;
	for(uniform int z=47;z >= 0;z--){
		a = max(out[x][y][z], a);
                out[x][y][z] = a;
		a = max(a - 2, 0);
	}
	}
	}
}

static void lightBlurY(uniform int8 out[48][48][48]){
	for(uniform int x=0;x < 48;x++){
	for(int z=programIndex;z < 48;z+=programCount){
	int8 a = 0;
	for(uniform int y=0;y < 48;y++){
		a = max(out[x][y][z], a);
		out[x][y][z] = a;
		a = max(a - 2, 0);
	}

	a = 0;
	for(uniform int y=47;y >= 0;y--){
		a = max(out[x][y][z], a);
		out[x][y][z] = a;
		a = max(a - 2, 0);
	}
	}
	}
}

static void lightBlurX(uniform int8 out[48][48][48]){
	for(uniform int y=0;y < 48;y++){
	for(int z=programIndex;z < 48;z+=programCount){
	int8 a = 0;
	for(uniform int x=0;x < 48;x++){
		a = max(out[x][y][z], a);
		out[x][y][z] = a;
		a = max(a - 2, 0);
	}

	a = 0;
	for(uniform int x=47;x >= 0;x--){
		a = max(out[x][y][z], a);
		out[x][y][z] = a;
		a = max(a - 2, 0);
	}
	}
	}
}

export void lightBlurISPC(uniform int8 out[48][48][48]){
	lightBlurX(out);
	lightBlurZ(out);
	lightBlurY(out);
}
