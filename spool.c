#include <stdio.h>
//#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>

#include "spool.h"

//typedef unsigned char uchar;

#define POOL_SIZE 1024*4000
#define BLOCK_SIZE 1024

typedef struct
{
    pthread_mutex_t mutex;
    void *pool;
    size_t psize;
    int *blocks;
    size_t bsize;
    size_t apos;
} spool_t;


spool_t spool;

void *sinit( size_t psize, size_t bsize )
{
    if( pthread_mutex_init( &spool.mutex, NULL ) != 0 )
    {
        return NULL;
    }
    spool.psize = psize == 0 ? POOL_SIZE : psize;
    spool.bsize = bsize == 0 ? BLOCK_SIZE : bsize;
    spool.pool = malloc( spool.psize * spool.bsize );
    if( spool.pool  == NULL )
    {
		pthread_mutex_destroy(&spool.mutex);
        return NULL;
    }
    spool.blocks = malloc( sizeof( int ) * spool.psize );
    if( spool.blocks == NULL )
    {
		pthread_mutex_destroy(&spool.mutex);
        return NULL;
    }
    for( int i = 0; i < spool.psize; i++ )
    {
        spool.blocks[i] = -1;
    }
    spool.apos = 0;
    printf( "sinit OK at %p\n", spool.pool );
    return spool.pool;
}

void suninit()
{
    pthread_mutex_destroy( &spool.mutex );
    free( spool.pool );
    free( spool.blocks );
    return;
}

void sfree( void *p )
{
    //printf( "sfree %p\n", p );
    if( p == NULL )
    {
        return;
    }
    int pos = ( p - spool.pool ) / spool.bsize;
    if( pos > spool.psize || pos < 0 )
    {
        printf( "sfree error, invalid pointer, p = %p, pos = %d\n", p, pos );
        return ;
    }
    int ix = pos;

    if( pthread_mutex_lock( &spool.mutex ) != 0 )
    {
        printf( "lock error!\n" );
		return;
    }
    while( spool.blocks[ix] == pos && ix < spool.psize )
    {
        spool.blocks[ix] = -1;
        ix++;
    }
    if( pthread_mutex_unlock( &spool.mutex ) != 0 )
    {
        printf( "unlock error!\n" );
    }
}

int ssearch( size_t start, size_t end, size_t blen )
{
    int frees = 0;
    //int found = -1;
    for( int i = start; i < end - blen; i++ )
    {
        if( spool.blocks[i] != -1 )
        {
            continue;
        }
        for( int j = 0; j < blen; j++ )
        {
            if( spool.blocks[i + j] != -1 )
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

void *salloc( size_t len )
{
    if( pthread_mutex_lock( &spool.mutex ) != 0 )
    {
        printf( "lock error!\n" );
    }

    int blen = len % spool.bsize == 0 ? len / spool.bsize : len / spool.bsize + 1;
    void *p = NULL;
    int found = ssearch( spool.apos, spool.psize, blen );
    if( found != -1 )
    {
        for( int j = 0; j < blen; j++ )
        {
            spool.blocks[found + j] = found;
        }
        //printf( "salloc found=%d,p=%p,blen=%d,len=%ld\n"
        //        , found, spool.pool + found * spool.bsize, blen, len );
        p = spool.pool + found * spool.bsize;
        spool.apos = found + blen;
    }
    else
    {
        //puts( "failed found space, search from header" );
        int found = ssearch( 0, spool.psize, blen );
        if( found != -1 )
        {
            for( int j = 0; j < blen; j++ )
            {
                spool.blocks[found + j] = found;
            }
            //printf( "salloc found=%d,p=%p,blen=%d,len=%ld\n"
            //        , found, spool.pool + found * spool.bsize, blen, len );
            p = spool.pool + found * spool.bsize;
            spool.apos = found + blen;
        }
        else
        {
            printf( "salloc blen=%d failed,still can't found, give up", blen );
        }
    }
    if( pthread_mutex_unlock( &spool.mutex ) != 0 )
    {
        printf( "unlock error!\n" );
    }
    return p;
}

int sused( void )
{
    size_t used = 0;
    for( size_t i = 0; i < spool.psize; i++ )
    {
        if( spool.blocks[i] != -1 )
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
    return used * 100 / spool.psize;
}


