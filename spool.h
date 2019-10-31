#ifndef __SPOOL_H__
#define __SPOOL_H__

#include <stddef.h>

#include <pthread.h>
typedef struct
{
    pthread_mutex_t mutex; 
    void *pool;
    size_t psize;
    int *blocks;
    size_t bsize;
    size_t apos;
} spool_t;

/**
 * @brief sinit 
 * 
 * @detail 内存分配。并初始化分配内存。
 * 			假如outside_spool不为0，则初始化spool为outside_spool
 * 			假如outside_spool为0，则使用psize,bsize初始化自己的spool
 * 
 * @param  outside_spool 输入，外部spool
 * @param  psize 输入，内存块数
 * @param  bsize  输入，内存块大小(字节)
 * 
 * @return 返回spool指针，失败返回空
 */
spool_t* sinit( spool_t* outside_spool,size_t psize, size_t bsize );

/**
 * @brief suninit 
 * 
 * @detail 释放spool
 * 
 * @param  
 * 
 * @return 
 */
void suninit( void );

/**
 * @brief  salloc
 * 
 * @detail 内存分配。不初始化。
 * 
 * @param  len 输入，内存大小(字节)
 * 
 * @return 返回分配的内存首地址，失败返回空
 */
void *salloc( size_t len );

/**
 * @brief  scalloc
 * 
 * @detail 内存分配。初始化分配的内存（设置为0）。
 * 
 * @param  len 输入，内存大小(字节)
 * 
 * @return 返回分配的内存首地址，失败返回空
 */
void *scalloc( size_t len );

/**
 * @brief  srealloc
 * 
 * @detail 内存分配。初始化分配的内存（设置为0）。
 * 
 * @param  p 输入，要释放的内存
 * @param  len 输入，内存大小(字节)
 * 
 * @return 返回分配的内存首地址，失败返回空
 */
void *srealloc(void *p, size_t len );
/**
 * @brief  sfree
 * 
 * @detail 释放申请的内存。
 * 
 * @param  ptr  输入，内存指针
 * 
 * @return 无
 */
void sfree( void *p );

#endif //__SPOOL_H__
