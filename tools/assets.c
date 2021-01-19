#include <stdio.h>

char *fileSlug(char *path){
	static char ret[1024];
	char c,*r,*start;
	r = ret;
	start = NULL;
	while((c = *path++)){
		if((c == '/') || (c == '.') || (c < 32)){
			if(start == NULL){start = r+1;}
			*r++ = '_';
			continue;
		}
		*r++ = c;
	}
	*r = 0;
	return start;
}

int main(int argc,char *argv[]){
	char fn[512];
	snprintf(fn,sizeof(fn)-1,"%s.c",argv[1]);
	fn[sizeof(fn)-1] = 0;
	FILE *cfp = fopen(fn,"w");
	snprintf(fn,sizeof(fn)-1,"%s.h",argv[1]);
	fn[sizeof(fn)-1] = 0;
	FILE *hfp = fopen(fn,"w");
	static unsigned char buffer[1024*1024*32];
	size_t filelen=0,readlen=0,ii;
	int lc=0,i;
	if((cfp == NULL) || (hfp == NULL)){
		fprintf(stderr,"Error opening src/tmp/assets*");
		return 1;
	}
	fprintf(hfp,"#pragma once\n#include <stddef.h>\n\n");
	fprintf(cfp,"#include <stddef.h>\n\n");
	for(i=2;i<argc;i++){
		FILE *afp = fopen(argv[i],"rb");
		if(afp == NULL){
			fprintf(stderr,"Error opening %s\n",argv[i]);
			return 2;
		}
		readlen=0;
		fseek(afp,0,SEEK_END);
		filelen = ftell(afp);
		fseek(afp,0,SEEK_SET);

		while(readlen < filelen){
			readlen += fread(buffer+readlen,1,filelen-readlen,afp);
		}
		fclose(afp);

		fprintf(hfp,"extern        size_t %s_len;\n",fileSlug(argv[i]));
		fprintf(hfp,"extern unsigned char %s_data[];\n\n",fileSlug(argv[i]));

		fprintf(cfp,"       size_t %s_len    = %u;\n",fileSlug(argv[i]),(int)filelen);
		fprintf(cfp,"unsigned char %s_data[] = {\n ",fileSlug(argv[i]));
		lc=0;
		for(ii=0;ii<filelen;ii++){
			fprintf(cfp,"0x%02X,",buffer[ii]);
			if(++lc > 19){
				fprintf(cfp,"\n ");
				lc=0;
			}
		}
		fprintf(cfp,"0\n};\n\n");


	}

	fclose(cfp);
	fclose(hfp);
	return 0;
}
