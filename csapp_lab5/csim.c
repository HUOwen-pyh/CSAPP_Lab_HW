#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int missv, hitv, evictv;
int sv, Ev, bv;
int globalt;

typedef struct Line{
	int rect;
	int flag;
	unsigned mask;
}line;

typedef struct SigSet{
	line* linearr;
}sigset;

sigset* mycache;

void init_cache();
void free_cache();
int load_addr(unsigned addr);

int main(int argc, char* argv[])
{
	assert(argc==9);
	missv = 0, hitv = 0, evictv = 0;
	globalt = 0;
	const char *fmode = "r";
	sv = argv[2][0] - '0';
	Ev = argv[4][0] - '0';
	bv = argv[6][0] - '0';
	init_cache();
	const char* filename = argv[8];
	FILE* fptr;
	assert((fptr=fopen(filename,fmode))!=NULL);

	char buf[256];
	while(1){	
		fgets(buf,256,fptr);
		if(feof(fptr))break;
		if(buf[0]=='I')continue;
		unsigned cur_addr = 0;
		for(int idx = 3;buf[idx]!=',';idx++){
			char tmp = buf[idx];
			cur_addr <<= 4;
			if(tmp>='0'&&tmp<='9')cur_addr += (unsigned)(tmp-'0');
			else cur_addr += (unsigned)(tmp-'a'+10);
		}
		int retval = load_addr(cur_addr);
		if(buf[1]=='M')hitv ++;
		if(retval == 0)hitv ++;		
		else if(retval == 1)missv ++;
		else{
			missv ++;
			evictv ++;
		}
//		printf("%d\n", retval);

	}
	assert(fclose(fptr)==0);
	free_cache();
	printSummary(hitv,missv,evictv);
	return 0;
}

void init_cache(){
	assert((mycache=(sigset*)malloc((1<<sv)*sizeof(sigset)))!=NULL);
	for(int i = 0;i<(1<<sv);i++){
		assert((mycache[i].linearr = (line*)malloc(Ev*sizeof(line)))!=NULL);
		for(int j = 0;j<Ev;j++)(mycache[i].linearr)[j].flag = 0;
	}
	return;
}

void free_cache(){
	for(int i = 0;i<(1<<sv);i++){
		free(mycache[i].linearr);
	}
	free(mycache);
	mycache = NULL;
	return;
}

int load_addr(unsigned addr){
	globalt++;
	unsigned sidx = ((1<<sv)-1);
	sidx &= (addr>>bv);
	unsigned curmask = (addr>>(sv+bv));
	int emptcnt = -1;
	for(int i = 0;i<Ev;i++){
		if((mycache[sidx].linearr)[i].flag == 1&& (mycache[sidx].linearr)[i].mask == curmask){
			(mycache[sidx].linearr)[i].rect = globalt;
			return 0;
		}
		if((mycache[sidx].linearr)[i].flag == 0)emptcnt = i;
	}
	if(emptcnt != -1){
		(mycache[sidx].linearr)[emptcnt].flag = 1;
		(mycache[sidx].linearr)[emptcnt].rect = globalt;
		(mycache[sidx].linearr)[emptcnt].mask = curmask;
	       return 1;
	}
	int mint = globalt;
	for(int i = 0;i<Ev;i++)if((mycache[sidx].linearr)[i].rect<mint){
		emptcnt = i;
		mint = (mycache[sidx].linearr)[i].rect;
	}
	//printf("%d %d %x\n", sidx, curmask, addr);
	assert(emptcnt>=0);
	(mycache[sidx].linearr)[emptcnt].rect = globalt;
	(mycache[sidx].linearr)[emptcnt].mask = curmask;
	return 2;
}
