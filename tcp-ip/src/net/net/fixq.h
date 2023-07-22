#ifndef FIXQ_H
#define FIXQ_H

#include "nlocker.h"
#include "sys.h"

typedef struct _fixq_t {
	int size;
	int in, out, cnt;
	
	void** buf;

	nlocker_t locker;
	sys_sem_t recv_sem, send_sem;
}fixq_t;

/// <summary>
/// 定长消息队列的初始化函数
/// </summary>
/// <param name="q">定长消息队列的指针</param>
/// <param name="buf">缓存数据的起始地址</param>
/// <param name="size">数组元素的数量</param>
/// <param name="type">锁的类型</param>
/// <returns>是否初始化成功</returns>
net_err_t fixq_init(fixq_t* q, void** buf, int size, nlocker_type_t type);

/// <summary>
/// 往消息队列中写入消息
/// </summary>
/// <param name="q">消息队列</param>
/// <param name="msg">发送的消息</param>
/// <param name="ms">等待时间</param>
/// <returns></returns>
net_err_t fixq_send(fixq_t* q, void* msg, int ms);

/// <summary>
/// 从消息队列中取出消息
/// </summary>
/// <param name="q">消息队列</param>
/// <param name="ms">可接受的等待时间</param>
/// <returns>消息地址</returns>
void* fixq_recv(fixq_t* q, int ms);

/// <summary>
/// 销毁消息队列
/// </summary>
/// <param name="q">需要销毁的消息队列</param>
void fixq_destroy(fixq_t* q);

/// <summary>
/// 获取消息队列中的消息数量
/// </summary>
/// <param name="q">消息队列</param>
/// <returns></returns>
int fixq_count(fixq_t* q);

#endif // !FIXQ_H
