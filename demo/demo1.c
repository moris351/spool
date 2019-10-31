#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

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

#include <time.h>
#define TIME_USED_PRE \
    clock_t start, end;\
    double cpu_time_used;

#define TIME_USED_START\
    start = clock();

#define TIME_USED_END(func)\
    end = clock();\
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;\
	printf("%s time used %0.8f\n", func, cpu_time_used);

int sused( void );

int main()
{
	TIME_USED_PRE
	TIME_USED_START
	void* head = sinit(0,100000,256);
	TIME_USED_END("sinit")

	if( head == 0 )
		return 0;

	char * p = NULL;

	TIME_USED_START
	p = malloc(2000);
	sfree(p);

	TIME_USED_END("malloc");
	srand(time(0)); 

	for(int i=0; i<200000; i++)
	{
		TIME_USED_START
		p = salloc(rand()%(10000-100)+100);
		TIME_USED_END("salloc");

		if( p == NULL )
		{
			puts("salloc failed!");
			break;
		}
		if( i%2 == 0  || i%3 ==0 || i%5 == 0 || i% 7== 0 )
			sfree(p);
	}


	printf("pool used=%d%%",sused());

	suninit();
	return 1;
}

