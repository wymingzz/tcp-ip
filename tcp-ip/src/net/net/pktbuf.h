#ifndef PKTBUF_H
#define PKTBUF_H

#include "nlist.h"
#include "net_cfg.h"
#include "net_err.h"
#include <cstdint>

typedef struct _pktblk_t {
	nlist_node_t node;
	int size;
	uint8_t* data;
	uint8_t payload[PKTBUF_BLK_SIZE];
}pktblk_t;

typedef struct _pktbuf_t {
	int total_size;
	nlist_t blk_list;
	nlist_node_t node;

	int ref;
	int pos;
	pktblk_t* curr_blk;
	uint8_t* blk_offset;
}pktbuf_t;

net_err_t pktbuf_init();

/// <summary>
/// 为pktbuf分配空间
/// </summary>
/// <param name="size">需要分配的大小</param>
/// <returns>所分配的地址</returns>
pktbuf_t* pktbuf_alloc(int size);

/// <summary>
/// 释放所分配的空间
/// </summary>
/// <param name="buf">需要释放的pktbuf</param>
void pktbuf_free(pktbuf_t* buf);

/// <summary>
/// 在数据前面添加头部结构
/// </summary>
/// <param name="buf">数据</param>
/// <param name="size">头部大小</param>
/// <param name="need_cont">头部是否需要连续</param>
/// <returns>是否添加成功</returns>
net_err_t pktbuf_add_header(pktbuf_t* buf, int size, bool need_cont);

/// <summary>
/// 移除数据前面的头部结构
/// </summary>
/// <param name="buf">数据</param>
/// <param name="size">头部大小</param>
/// <returns>是否移除成功</returns>
net_err_t pktbuf_remove_header(pktbuf_t* buf, int size);

/// <summary>
/// 调整数据的大小
/// </summary>
/// <param name="buf">数据</param>
/// <param name="size">调整后的大小</param>
/// <returns>是否成功</returns>
net_err_t pktbuf_resize(pktbuf_t* buf, int size);

/// <summary>
/// 移除数据后面的新增结构
/// </summary>
/// <param name="buf">数据</param>
/// <param name="size">需要移除的大小</param>
/// <returns>是否移除成功</returns>
net_err_t pktbuf_remove_tail(pktbuf_t* buf, int size);

/// <summary>
/// 合并两个数据包
/// </summary>
/// <param name="dest">最后保存的数据包</param>
/// <param name="src">需要插入的数据包(插入完被释放)</param>
/// <returns>是否合并成功</returns>
net_err_t pktbuf_join(pktbuf_t* dest, pktbuf_t* src);

/// <summary>
/// 将数据包头部结构调整到一个节点中
/// </summary>
/// <param name="buf">需要调整的数据包</param>
/// <param name="size">头部大小</param>
/// <returns></returns>
net_err_t pktbuf_set_cont(pktbuf_t* buf, int size);

/// <summary>
/// 刷新pos位置，设置到开头
/// </summary>
/// <param name="buf">数据包</param>
void pktbuf_reset_acc(pktbuf_t* buf);

/// <summary>
/// 向数据包中写入数据
/// </summary>
/// <param name="buf">数据报</param>
/// <param name="src">数据源</param>
/// <param name="size">写入的数据大小</param>
/// <returns>是否写入成功</returns>
net_err_t pktbuf_write(pktbuf_t* buf, uint8_t* src, int size);

/// <summary>
/// 从数据包的当前位置开始向前读取数据
/// </summary>
/// <param name="buf">数据包</param>
/// <param name="dest">需要写入的数据</param>
/// <param name="size">读取的大小</param>
/// <returns>是否读取成功</returns>
net_err_t pktbuf_read(pktbuf_t* buf, uint8_t* dest, int size);

/// <summary>
/// 重定位数据包的读写位置
/// </summary>
/// <param name="buf">数据包</param>
/// <param name="offset">重定位到对于开头的偏移量</param>
/// <returns>是否重定位成功</returns>
net_err_t pktbuf_seek(pktbuf_t* buf, int offset);

/// <summary>
/// 将一个数据包的数据拷贝到另一个数据包中
/// </summary>
/// <param name="dest">目标数据包</param>
/// <param name="src">源数据包</param>
/// <param name="size">需要拷贝的大小</param>
/// <returns>是否拷贝成功</returns>
net_err_t pktbuf_copy(pktbuf_t* dest, pktbuf_t* src, int size);

/// <summary>
/// 填充数据包中的数据
/// </summary>
/// <param name="buf">数据包</param>
/// <param name="v">已该字节进行填充</param>
/// <param name="size">填充的大小</param>
/// <returns>是否填充成功</returns>
net_err_t pktbuf_fill(pktbuf_t* buf, uint8_t v, int size);

/// <summary>
/// 数据包引用
/// </summary>
/// <param name="buf">被引用的数据包</param>
void pktbuf_inc_ref(pktbuf_t* buf);

/**
 * 获取当前block的下一个子包
 */
static inline pktblk_t* pktbuf_blk_next(pktblk_t* blk) {
	nlist_node_t* next = blk->node.next;
	return nlist_entry(next, pktblk_t, node);
}

/**
 * 获取当前block的上一个子包
 */
static inline pktblk_t* pktbuf_blk_pre(pktblk_t* blk) {
	nlist_node_t* pre = blk->node.pre;
	return nlist_entry(pre, pktblk_t, node);
}

/**
 * 取buf的第一个数据块
 * @param buf 数据缓存buf
 * @return 第一个数据块
 */
static inline pktblk_t* pktbuf_first_blk(pktbuf_t* buf) {
	nlist_node_t* first = buf->blk_list.first;
	return nlist_entry(first, pktblk_t, node);
}

/**
 * 取buf的最后一个数据块
 * @param buf 数据缓存buf
 * @return 最后一个数据块
 */
static inline pktblk_t* pktbuf_last_blk(pktbuf_t* buf) {
	nlist_node_t* first = buf->blk_list.last;
	return nlist_entry(first, pktblk_t, node);
}

/// <summary>
/// 返回数据包中第一个数据块的data开始指针
/// </summary>
/// <param name="buf">数据包</param>
/// <returns>数据包中第一个数据块的data开始指针</returns>
static inline uint8_t* pktbuf_data(pktbuf_t* buf) {
	pktblk_t* first = pktbuf_first_blk(buf);
	return first == nullptr ? nullptr : first->data;
}

#endif // !PKTBUF_H
