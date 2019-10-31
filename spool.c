#include <stdio.h>
//#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "spool.h"

//typedef unsigned char uchar;

#define POOL_SIZE 1024*4000
#define BLOCK_SIZE 1024


spool_t *spool = 0;

spool_t *sinit( spool_t* out_spool,size_t psize, size_t bsize )
{
	if(out_spool)
	{
		spool = out_spool;
		return spool;
	}

	spool  = malloc(sizeof(spool_t));
	if(spool == NULL)
	{
		printf("sinit:malloc for spool failed!");
		return NULL;
	}
	memset(spool,0,sizeof(spool_t));

    spool->psize = psize == 0 ? POOL_SIZE : psize;
    spool->bsize = bsize == 0 ? BLOCK_SIZE : bsize;
    spool->pool = malloc( spool->psize * spool->bsize );
    if( spool->pool  == NULL )
    {
		printf("sinit:malloc for spool->pool failed!");
        return NULL;
    }
	memset(spool->pool,0,psize*bsize);

    spool->blocks = malloc( sizeof( int ) * spool->psize );
    if( spool->blocks == NULL )
    {
		printf("sinit:malloc for spool->blocks failed!");
        return NULL;
    }

    for( int i = 0; i < spool->psize; i++ )
    {
        spool->blocks[i] = -1;
    }
    spool->apos = 0;

    if( pthread_mutex_init( &spool->mutex, NULL ) != 0 )
    {
		printf("sinit:pthread_mutex_init failed!");
        return NULL;
    }

    printf( "sinit OK at %p\n", spool->pool );
    return spool->pool;
}

void suninit()
{
    pthread_mutex_destroy( &spool->mutex );
    free( spool->pool );
    free( spool->blocks );
    return;
}

void sfree( void *p )
{
    //printf( "sfree %p\n", p );
    if( p == NULL )
    {
        return;
    }
    int pos = ( p - spool->pool ) / spool->bsize;
    if( pos > spool->psize || pos < 0 )
    {
        printf( "sfree error, invalid pointer, p = %p, pos = %d\n", p, pos );
        return ;
    }
    int ix = pos;

    if( pthread_mutex_lock( &spool->mutex ) != 0 )
    {
        printf( "lock error!\n" );
		return;
    }
    while( spool->blocks[ix] == pos && ix < spool->psize )
    {
        spool->blocks[ix] = -1;
        ix++;
    }
    if( pthread_mutex_unlock( &spool->mutex ) != 0 )
    {
        printf( "unlock error!\n" );
    }
}

//p -- the pointer
//start --- the start index in spool
//len --- the len of len in spool
//return 0 when failed, 1 when success
//
static int ptrlen( void *p, size_t *start,size_t *len)
{
    if( p == NULL )
    {
        return 0;
    }
    int pos = ( p - spool->pool ) / spool->bsize;
    if( pos > spool->psize || pos < 0 )
    {
        printf( "sfree error, invalid pointer, p = %p, pos = %d\n", p, pos );
        return 0;
    }
    int ix = pos;

    if( pthread_mutex_lock( &spool->mutex ) != 0 )
    {
        printf( "lock error!\n" );
		return 0;
    }
    while( spool->blocks[ix] == pos && ix < spool->psize )
    {
        ix++;
    }
    if( pthread_mutex_unlock( &spool->mutex ) != 0 )
    {
        printf( "unlock error!\n" );
		return 0;
    }
	*start = pos;
	*len = ix - pos;
	return 1;
}

static int ssearch( size_t start, size_t end, size_t blen )
{
    int frees = 0;
    //int found = -1;
    for( int i = start; i < end - blen; i++ )
    {
        if( spool->blocks[i] != -1 )
        {
            continue;
        }
        for( int j = 0; j < blen; j++ )
        {
            if( spool->blocks[i + j] != -1 )
            {
                frees = 0;
                break;
            }
            frees++;
        }
        //found
        if( frees == blen )
        {
            return i;
        }
        frees = 0;
    }
    return -1;
}

void *srealloc(void *p, size_t len )
{
	if(p==NULL)
	{
		return salloc(len);
	}
	if( len==0 )
	{
		sfree(p);
		return NULL;
	}
	size_t start = 0, plen = 0;
		
	int ret = ptrlen(p,&start, &plen );
	printf("p: start=%lu, plen=%lu\n",start,plen);
	if( ret != 1 )
	{
		return NULL;
	}

	if(plen*spool->bsize>=len)
	{//shrink
		//len>0, plen>0, p must be a valid spool ptr
		int pos = start;
		int ix = pos+len;
		while( spool->blocks[ix] == pos && ix< pos+plen && ix < spool->psize )
		{
			spool->blocks[ix] = -1;
			ix++;
		}
		return p;
	}

	void *ap = salloc(len);
	if(!ap)
	{
		return NULL;
	}
	memcpy(ap,p,plen*spool->psize);
	sfree(p);

	return ap;
}

void *scalloc( size_t len )
{
	void *p = salloc(len);
	if(p)
	{
		memset(p,0,len);
	}
	return p;
}

void *salloc( size_t len )
{
    if( pthread_mutex_lock( &spool->mutex ) != 0 )
    {
        printf( "lock error!\n" );
    }

    int blen = len % spool->bsize == 0 ? len / spool->bsize : len / spool->bsize + 1;
    void *p = NULL;
    int found = ssearch( spool->apos, spool->psize, blen );
    if( found != -1 )
    {
        for( int j = 0; j < blen; j++ )
        {
            spool->blocks[found + j] = found;
        }
        //printf( "salloc found=%d,p=%p,blen=%d,len=%ld\n"
        //        , found, spool->pool + found * spool->bsize, blen, len );
        p = spool->pool + found * spool->bsize;
        //spool->apos = found + blen;
    }
    else
    {
        //puts( "failed found space, search from header" );
        int found = ssearch( 0, spool->psize, blen );
        if( found != -1 )
        {
            for( int j = 0; j < blen; j++ )
            {
                spool->blocks[found + j] = found;
            }
            //printf( "salloc found=%d,p=%p,blen=%d,len=%ld\n"
            //        , found, spool->pool + found * spool->bsize, blen, len );
            p = spool->pool + found * spool->bsize;
            //spool->apos = found + blen;
        }
        else
        {
            printf( "salloc blen=%d failed,still can't found, give up", blen );
        }
    }
    if( pthread_mutex_unlock( &spool->mutex ) != 0 )
    {
        printf( "unlock error!\n" );
    }
    return p;
}

int sused( void )
{
    size_t used = 0;
    for( size_t i = 0; i < spool->psize; i++ )
    {
        if( spool->blocks[i] != -1 )
        {
            printf( "A" );
            used++;
        }
        else
        {
            printf( "O" );
        }
    }
    printf( "\n" );
    return used * 100 / spool->psize;
}


