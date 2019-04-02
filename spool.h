#ifndef __SPOOL_H__

#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>

typedef unsigned char uchar;
typedef unsigned long ulong;

#define MAX_POOL_SIZE 1024*4000
#define BLOCK_SIZE 1024

typedef struct 
{
	pthread_mutex_t mutex;
	uchar * pool;
	ulong psize;
	int *blocks;
	ulong bsize;
	ulong apos;
}spool_t;

void * sinit(ulong psize,ulong bsize);

void suninit();

void sfree(void * p);

void * salloc(size_t len);

#endif //__SPOOL_H__
