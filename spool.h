#ifndef __SPOOL_H__

#include <stddef.h>

//init spool
//psize --- number of blocks
//bsize --- size of every block
//must call before do anything
//return NULL if failed
//return not NULL if succeed
void *sinit( size_t psize, size_t bsize );

//free the spool
//call before end of spool
void suninit( void );

//alloc blocks with length of memory
//return memory pointer if succeed
//otherwise return NULL
void *salloc( size_t len );

//free p
//free a NULL pointer will do nothing
void sfree( void *p );

#endif //__SPOOL_H__
