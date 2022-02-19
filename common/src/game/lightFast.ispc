/* An experiment to see how well we can integrate ISPC
 * code into the existing codebase, as well as how
 * well the generated instructions compare against
 * hand written assembly.
 */

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

export void ispcLightBlur(uniform int8 out[48][48][48]){
	lightBlurX(out);
	lightBlurZ(out);
	lightBlurY(out);
}
