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
	if( spool.pool  == 0 )
	{
		return 0;
	}

	spool.blocks = malloc(sizeof(int)*spool.psize);
	if( spool.blocks == 0 )
	{
		return 0;
	}

	for(int i=0;i<spool.psize;i++)
	{
		spool.blocks[i]=-1;
	}

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
	//printf("sfree %p\n",p);
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

//alloc blocks with length of memory
//return memory pointer if succeed
//otherwise return NULL
void * salloc(size_t len)
{
	if (pthread_mutex_lock(&spool.mutex) != 0){
		printf("lock error!\n");
	}
	int blen=len/spool.bsize+1;
	//printf("blen=%d\n",blen);
	void * p = NULL;

	int frees=0;
	for(int i=0;i<spool.psize;i++)
	{
		for( int j=0;j<blen;j++)
		{
			if( spool.blocks[i+j]==-1 )
			{
				frees++;
			}	
		}
		if( frees == blen )
		{
			for( int j=0;j<blen;j++)
			{
				spool.blocks[i+j]=i; 
			}

			//printf("salloc i=%d,p=%p,len=%d\n",i,spool.pool+i*spool.bsize,blen);
			
			p = spool.pool+i*spool.bsize;
			break;
		}
		frees=0;
	}

	if (pthread_mutex_unlock(&spool.mutex) != 0){
		printf("unlock error!\n");
	}

	return p;
}
