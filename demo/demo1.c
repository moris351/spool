#include <stdio.h>
#include <string.h>

#include "../spool.h"
/*
typedef void * (*malloc_func)(size_t);
typedef void   (*free_func)(void *);

void set_allocation_func(malloc_func afunc, free_func ffunc);
void set_allocation_func(malloc_func afunc, free_func ffunc)
{
    demo_alloc = afunc;
    demo_free = ffunc;
}
static void *demo_malloc(size_t size);
static void demo_free(void *ptr);
*/
int main()
{
	int ret = sinit(1024,1999);
	if( ret == 0 )
		return 0;

	char * p = salloc(1000);
	if( p == NULL )
		return 0;

	char str[10]="message ";
	memcpy(p,str,10);
	printf(p);
	sfree(p);

	suninit();
	return 1;
}

