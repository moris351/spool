# spool
thread safe memory  pool for performance or security
# usage:
## init spool first
```
//init spool
//psize --- number of blocks
//bsize --- size of every block
//must call before do anything
//return NULL if failed
//return not NULL if succeed
void *sinit( size_t psize, size_t bsize );
```
for example
```
if( NULL == sinit( 1024*100, 1024) ) //100M
	return 0;
```
## alloc memory
```
//alloc blocks with length of memory
//return memory pointer if succeed
//otherwise return NULL
void *salloc( size_t len );
```
## free memory
```
//free p
//free a NULL pointer will do nothing
void sfree( void *p );
```
## uninitialize spool
```
//free the spool
//call before end of spool
void suninit( void );
```
