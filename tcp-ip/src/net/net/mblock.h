#ifndef MBLOCK_H
#define MBLOCK_H

#include "nlist.h"
#include "nlocker.h"

typedef struct _mblock_t {
	nlist_t free_list;
	void* start;
	nlocker_t locker;
	sys_sem_t alloc_sem;
}mblock_t;

/// <summary>
/// 初始化mblock
/// </summary>
/// <param name="mblock">需要初始化的mblock</param>
/// <param name="mem">数组的起始位置</param>
/// <param name="blk_size">数组元素的大小</param>
/// <param name="cnt">数组表项个数</param>
/// <param name="locker_type">locker的类型</param>
/// <returns>是否初始化成功</returns>
net_err_t mblock_init(mblock_t* mblock, void* mem, int blk_size, int cnt, nlocker_type_t locker_type);

/// <summary>
/// 申请空闲内存块
/// </summary>
/// <param name="mblock"></param>
/// <param name="ms">无法获取mblock时希望的等待时间，ms&lt;0不等待，ms=0死等，否则等待x毫秒</param>
/// <returns>有空闲内存返回内存地址，否则返回nullptr</returns>
void* mblock_alloc(mblock_t* mblock, int ms);

/// <summary>
/// 返回空闲链表的长度
/// </summary>
/// <param name="mblock"></param>
/// <returns>一个int值，为空闲链表的长度</returns>
int mblock_free_cnt(mblock_t* mblock);

/// <summary>
/// 释放内存块
/// </summary>
/// <param name="mblock"></param>
/// <param name="block">需要释放的内存块</param>
void mblock_free(mblock_t* mblock,void *block);


/// <summary>
/// 销毁mblock
/// </summary>
/// <param name="mblock"></param>
void mblock_destroy(mblock_t* mblock);

#endif // !MBLOCK_H
