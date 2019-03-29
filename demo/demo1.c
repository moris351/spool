#include <stdio.h>
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

int main()
{
	TIME_USED_PRE
	TIME_USED_START
	int ret = sinit(100000,256);
	TIME_USED_END("sinit")

	if( ret == 0 )
		return 0;

	char * p = NULL;

	TIME_USED_START
	p = malloc(2000);
	TIME_USED_END("malloc");
	srand(time(0)); 

	for(int i=0; i<200000; i++)
	{
		TIME_USED_START
		p = salloc(rand()%(1000-100)+100);
		TIME_USED_END("salloc");

		if( p == NULL )
		{
			puts("salloc failed!");
			return 0;
		}
		if( i%3 == 0 )
			sfree(p);
	}


	char str[10]="message ";
	memcpy(p,str,10);
	puts(p);
	sfree(p);

	suninit();
	return 1;
}

