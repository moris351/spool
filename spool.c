#include <stdio.h>
#include "spool.h"

spool_t spool;

//init spool
//psize --- number of blocks
//bsize --- size of every block
//must call before do anything
//return 0 if failed
//return 1 if succeed
int sinit(ulong psize,ulong bsize)
{
	if (pthread_mutex_init(&spool.mutex, NULL) != 0){
		return 0;
	}

	spool.psize = psize == 0 ? MAX_POOL_SIZE: psize;
	spool.bsize = bsize == 0 ? BLOCK_SIZE: bsize;

	spool.pool = malloc(spool.psize*spool.bsize);
	if( spool.pool  == NULL )
	{
		return 0;
	}

	spool.blocks = malloc(sizeof(int)*spool.psize);
	if( spool.blocks == NULL )
	{
		return 0;
	}

	for(int i=0;i<spool.psize;i++)
	{
		spool.blocks[i]=-1;
	}

	spool.apos = 0;

	//printf("sinit OK\n");
	return 1;
}

//free the spool
//call before end of spool
void suninit()
{
	pthread_mutex_destroy(&spool.mutex);

	free(spool.pool);
	free(spool.blocks);

	return;

}

//free p
//free a NULL pointer will do nothing
void sfree(void * p)
{
	printf("sfree %p\n",p);
	if( p == NULL )
		return;

	if (pthread_mutex_lock(&spool.mutex) != 0){
		printf("lock error!\n");
	}
	int pos = ((uchar*)p - spool.pool)/spool.bsize;
	int ix=pos;
	while(spool.blocks[ix]==pos && ix<spool.psize)
	{
		spool.blocks[ix]=-1;
		ix++;
	}

	if (pthread_mutex_unlock(&spool.mutex) != 0){
		printf("unlock error!\n");
	}
}

int ssearch(ulong start, ulong end, ulong blen)
{
	int frees=0;
	int found = -1;

	for(int i=start;i<end-blen;i++)
	{
		if( spool.blocks[i] != -1 )
			continue;

		for( int j=0;j<blen;j++)
		{
			if( spool.blocks[i+j]!=-1 )
			{
				frees=0;
				break;
			}	

			frees++;
		}
		//found
		if( frees == blen )
		{
			return i;
		}
		frees=0;
	}
	return -1;
}
//alloc blocks with length of memory
//return memory pointer if succeed
//otherwise return NULL
void * salloc(size_t len)
{
	if (pthread_mutex_lock(&spool.mutex) != 0){
		printf("lock error!\n");
	}
	int blen=len%spool.bsize == 0 ? len/spool.bsize : len/spool.bsize+1;
	void * p = NULL;

	int found = ssearch(spool.apos,spool.psize,blen);
	if( found !=-1 )
	{
		for( int j=0;j<blen;j++)
		{
			spool.blocks[found+j]=found; 
		}

		printf("salloc found=%d,p=%p,len=%d\n",found,spool.pool+found*spool.bsize,blen);
		
		p = spool.pool+found*spool.bsize;
		spool.apos = found+blen;
	}else
	{
		puts("failed found space, search from header");
		int found = ssearch(0,spool.apos,blen);
		if( found !=-1 )
		{
			for( int j=0;j<blen;j++)
			{
				spool.blocks[found+j]=found; 
			}

			printf("salloc found=%d,p=%p,len=%d\n",found,spool.pool+found*spool.bsize,blen);
			
			p = spool.pool+found*spool.bsize;
			spool.apos = found+blen;
		}
		else
		{
			puts("still can't found, give up");
		}
	}		

	if (pthread_mutex_unlock(&spool.mutex) != 0){
		printf("unlock error!\n");
	}

	return p;
}
