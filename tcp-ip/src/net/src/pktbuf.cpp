#include "pktbuf.h"
#include "dbg.h"
#include "mblock.h"
#include "nlocker.h"

static nlocker_t locker;
static pktblk_t block_buffer[PKTBUF_BLK_CNT];
static mblock_t block_list;
static pktbuf_t pktbuf_buffer[PKTBUF_BUF_CNT];
static mblock_t pktbuf_list;

static inline int curr_blk_tail_free(pktblk_t* blk) {
	return (blk->payload + PKTBUF_BLK_SIZE) - (blk->data + blk->size);
}

/// <summary>
/// 从当前位置到数据包结束可以写入多少数据
/// </summary>
/// <param name="buf">数据包</param>
/// <returns>可写入的数据大小</returns>
static inline int total_blk_remain(pktbuf_t* buf) {
	return buf->total_size - buf->pos;
}

/// <summary>
/// 当前数据块中还可以写入多少数据
/// </summary>
/// <param name="buf">数据包</param>
/// <returns>可写入的数据大小</returns>
static inline int curr_blk_remain(pktbuf_t* buf) {
	pktblk_t* block = buf->curr_blk;
	if (block == nullptr) {
		return 0;
	}
	return block->data + block->size - buf->blk_offset;
}

#if DBG_DISP_ENABLED(DBG_PKTBUF)
static void display_check_buf(pktbuf_t* buf) {
	if (buf == nullptr) {
		dbg_error(DBG_PKTBUF, "invalod buf. buf == 0.");
	}

	plat_printf("check buf %p: size %d\n", buf, buf->total_size);
	pktblk_t* curr;
	int index = 0, total_size = 0;
	for (curr = pktbuf_first_blk(buf); curr != nullptr;curr = pktbuf_blk_next(curr)) {
		plat_printf("%4d: ", index++);

		if ((curr->data < curr->payload) || (curr->data >= curr->payload + PKTBUF_BLK_SIZE)) {
			dbg_error(DBG_PKTBUF, "bad block data .\n");
		}
		int pre_size = (curr->data - curr->payload);
		plat_printf("pre: %4d b, used: %4d b, ", pre_size, curr->size);

		int free_size = curr_blk_tail_free(curr);
		plat_printf("free: %4d b\n", free_size);

		int blk_total = pre_size + curr->size + free_size;
		if (blk_total != PKTBUF_BLK_SIZE) {
			dbg_error(DBG_PKTBUF, "bad block size: %d != %d .\n", blk_total, PKTBUF_BLK_SIZE);
		}

		total_size += curr->size;
	}

	if (total_size != buf->total_size) {
		dbg_error(DBG_PKTBUF, "bad buf size: %d != %d .\n", total_size, buf->total_size);
	}
}
#else
#define display_check_buf(buf)
#endif
net_err_t pktbuf_init()
{
	dbg_info(DBG_PKTBUF, "init pktbuf.");
	
	net_err_t err = nlocker_init(&locker, NLOCKER_THREAD);
	if (err < 0) {
		dbg_error(DBG_PKTBUF, "init locker failed.");
		return err;
	}
	err = mblock_init(&block_list, block_buffer, sizeof(pktblk_t), PKTBUF_BLK_CNT, NLOCKER_THREAD);
	if (err < 0) {
		dbg_error(DBG_PKTBUF, "init block_list failed.");
		goto block_list_failed;
	}
	err = mblock_init(&pktbuf_list, pktbuf_buffer, sizeof(pktbuf_t), PKTBUF_BUF_CNT, NLOCKER_THREAD);
	if (err < 0) {
		dbg_error(DBG_PKTBUF, "init pktbuf_buffer failed.");
		goto pktbuf_buffer_failed;
	}
	return NET_ERR_OK;
	mblock_destroy(&pktbuf_list);
pktbuf_buffer_failed:
	mblock_destroy(&block_list);
block_list_failed:
	nlocker_destroy(&locker);
	return err;
}


/// <summary>
/// 分配一个pktblk块
/// </summary>
/// <returns>pktblk块的地址</returns>
static pktblk_t* pktblock_alloc() {
	pktblk_t* block = (pktblk_t*)mblock_alloc(&block_list, -1);

	if (block != nullptr) {
		block->size = 0;
		block->data = nullptr;
		nlist_node_init(&block->node);
	}

	return block;
}

/// <summary>
/// 创建pktbuf_t链表
/// </summary>
/// <param name="size">链表的总长度</param>
/// <param name="add_front">是否采用头插法</param>
/// <returns>一个链表</returns>
static pktbuf_t* pktblock_alloc_list(int size, bool add_front) {
	pktbuf_t* new_list = (pktbuf_t*)mblock_alloc(&pktbuf_list,-1);
	if (new_list == nullptr) {
		dbg_warning(DBG_PKTBUF, "pktbuf no buffer");
		return nullptr;
	}
	new_list->total_size = 0;
	nlist_init(&new_list->blk_list);
	nlist_node_init(&new_list->node);
	while (size) {
		pktblk_t* new_block = pktblock_alloc();
		if (new_block == nullptr) {
			dbg_error(DBG_PKTBUF, "no buffer for alloc(%d)", size);
			pktbuf_free(new_list);
			return nullptr;
		}

		int curr_size = size > PKTBUF_BLK_SIZE ? PKTBUF_BLK_SIZE : size;
		new_block->size = curr_size;
		
		if (add_front) {
			new_block->data = new_block->payload + PKTBUF_BLK_SIZE - curr_size;

			nlist_insert_first(&new_list->blk_list,&new_block->node);
		}
		else {
			new_block->data = new_block->payload;
			
			nlist_insert_last(&new_list->blk_list, &new_block->node);
		}

		new_list->total_size += curr_size;
		size -= curr_size;
	}
	return new_list;
}

pktbuf_t* pktbuf_alloc(int size)
{
	pktbuf_t* buf = (pktbuf_t*)mblock_alloc(&pktbuf_list,-1);
	if (buf == nullptr) {
		dbg_warning(DBG_PKTBUF, "no buffer.");
		return nullptr;
	}

	buf->total_size = 0;
	nlist_init(&buf->blk_list);
	nlist_node_init(&buf->node);

	if (size) {
		pktbuf_t* list = pktblock_alloc_list(size, 1);
		if (list == nullptr) {
			mblock_free(&pktbuf_list, buf);
			return nullptr;
		}
		nlist_insert_list(&buf->blk_list, &list->blk_list, 0);
		buf->total_size += list->total_size;
		pktbuf_free(list);
		list = nullptr;
	}


	pktbuf_reset_acc(buf);
	display_check_buf(buf);
	return buf;
}

/// <summary>
/// 回收数据块
/// </summary>
/// <param name="block"></param>
static void pktblock_free(pktblk_t* block) {
	mblock_free(&block_list, block);
}

/**
 * 回收block链
 */
static void pktblock_free_list(pktblk_t* first) {
	while (first != nullptr) {
		pktblk_t* next = pktbuf_blk_next(first);
		pktblock_free(first);
		first = next;
	}
}


void pktbuf_free(pktbuf_t* buf)
{
	nlocker_lock(&locker);
	pktblock_free_list(pktbuf_first_blk(buf));
	mblock_free(&pktbuf_list, buf);
	nlocker_unlock(&locker);
}

net_err_t pktbuf_add_header(pktbuf_t* buf, int size, bool need_cont)
{
	pktblk_t* block = pktbuf_first_blk(buf);

	int alloc_size = (block->data - block->payload) > size ? size : (block->data - block->payload);
	int resv_size = 0;
	if ((!need_cont && alloc_size != 0)||(alloc_size >= size)) {
		block->size += alloc_size;
		block->data -= alloc_size;

		buf->total_size += alloc_size;
		resv_size += alloc_size;
		display_check_buf(buf);
	}

	if (need_cont && size > PKTBUF_BLK_SIZE) {
		dbg_error(DBG_PKTBUF, "set cont, size to big: %d > %d", size, PKTBUF_BLK_SIZE);
		return NET_ERR_SIZE;
	}

	while (size > resv_size) {
		alloc_size = (size - resv_size) > PKTBUF_BLK_SIZE ? PKTBUF_BLK_SIZE : (size - resv_size);
		pktbuf_t *new_buf = pktblock_alloc_list(alloc_size, 1);
		if (new_buf == nullptr) {
			dbg_error(DBG_PKTBUF, "no buffer (size: %d).", alloc_size);
			pktbuf_remove_header(buf, resv_size);
			return NET_ERR_SIZE;
		}

		nlist_insert_list(&buf->blk_list, &new_buf->blk_list, true);
		buf->total_size += new_buf->total_size;
		pktbuf_free(new_buf);
		display_check_buf(buf);
		resv_size += alloc_size;
	}

	return NET_ERR_OK;
}

net_err_t pktbuf_remove_header(pktbuf_t* buf, int size)
{
	pktblk_t* block = pktbuf_first_blk(buf);
	while (size) {
		if (size < block->size) {
			block->data += size;
			block->size -= size;
			buf->total_size -= size;
			break;
		}

		int curr_size = block->size;
		pktblk_t* next_blk = pktbuf_blk_next(block);
		nlist_remove_first(&buf->blk_list);
		pktblock_free(block);

		size -= curr_size;
		buf->total_size -= curr_size;
		block = next_blk;
	}

	display_check_buf(buf);
	return NET_ERR_OK;
}

net_err_t pktbuf_resize(pktbuf_t* buf, int size)
{
	if (size == buf->total_size) {
		return NET_ERR_OK;
	}

	size -= buf->total_size;
	if (size > 0) {
		pktblk_t* tail_blk = pktbuf_last_blk(buf);
		int remain_size = curr_blk_tail_free(tail_blk);
		int resv_size = 0, alloc_size = remain_size > size ? size : remain_size;

		if (alloc_size != 0) {
			tail_blk->size += alloc_size;
			buf->total_size += alloc_size;
			resv_size += alloc_size;
			display_check_buf(buf);
		}

		while (size > resv_size) {
			alloc_size = (size - resv_size) > PKTBUF_BLK_SIZE ? PKTBUF_BLK_SIZE : (size - resv_size);
			pktbuf_t* new_buf = pktblock_alloc_list(alloc_size, false);
			if (new_buf == nullptr) {
				dbg_error(DBG_PKTBUF, "no buffer (size: %d.", alloc_size);
				pktbuf_remove_tail(buf, resv_size);
				return NET_ERR_SIZE;
			}

			nlist_insert_list(&buf->blk_list, &new_buf->blk_list,false);
			buf->total_size += new_buf->total_size;
			pktbuf_free(new_buf);
			resv_size += alloc_size;
			display_check_buf(buf);
		}
	}
	else {
		pktbuf_remove_tail(buf, -size);
	}

	return NET_ERR_OK;
}

net_err_t pktbuf_remove_tail(pktbuf_t* buf, int size)
{
	pktblk_t* block = pktbuf_last_blk(buf);
	while (size) {
		if (size < block->size) {
			block->size -= size;
			buf->total_size -= size;
			break;
		}

		int curr_size = block->size;
		pktblk_t* pre_blk = pktbuf_blk_pre(block);
		nlist_remove_last(&buf->blk_list);
		pktblock_free(block);

		size -= curr_size;
		buf->total_size -= curr_size;
		block = pre_blk;
	}

	display_check_buf(buf);
	return NET_ERR_OK;
}

net_err_t pktbuf_join(pktbuf_t* dest, pktbuf_t* src)
{
	nlist_insert_list(&dest->blk_list, &src->blk_list, false);
	dest->total_size += src->total_size;
	pktbuf_free(src);
	src = nullptr;

	display_check_buf(dest);
	return NET_ERR_OK;
}

net_err_t pktbuf_set_cont(pktbuf_t* buf, int size)
{
	if (size > buf->total_size) {
		dbg_error(DBG_PKTBUF, "size %d > total_size %d.", size, buf->total_size);
		return NET_ERR_SIZE;
	}

	if (size > PKTBUF_BLK_SIZE) {
		dbg_error(DBG_PKTBUF, "size too big: size %d > block_size %d.", size, PKTBUF_BLK_SIZE);
		return NET_ERR_SIZE;
	}

	pktblk_t* first_blk = pktbuf_first_blk(buf);
	if (size <= first_blk->size) {
		return NET_ERR_OK;
	}

	uint8_t* dest = first_blk->payload;
	for (int i = 0; i < first_blk->size; i++) {
		*dest++ = first_blk->data[i];
	}
	first_blk->data = first_blk->payload;

	pktblk_t* curr_blk = pktbuf_blk_next(first_blk);
	int remain_size = size - first_blk->size;
	while (remain_size && curr_blk != nullptr) {
		int curr_size = (curr_blk->size > remain_size) ? remain_size : curr_blk->size;

		plat_memcpy(dest, curr_blk->data, curr_size);
		dest += curr_size;
		curr_blk->data += curr_size;
		curr_blk->size -= curr_size;
		first_blk->size += curr_size;
		remain_size -= curr_size;

		if (curr_blk->size == 0) {
			pktblk_t* next_blk = pktbuf_blk_next(curr_blk);
			nlist_remove(&buf->blk_list, &curr_blk->node);
			pktblock_free(curr_blk);
			curr_blk = next_blk;
		}
	}

	display_check_buf(buf);
	return NET_ERR_OK;
}

void pktbuf_reset_acc(pktbuf_t* buf)
{
	if (buf != nullptr) {
		buf->pos = 0;
		buf->curr_blk = pktbuf_first_blk(buf);
		buf->blk_offset = buf->curr_blk == nullptr ? nullptr : buf->curr_blk->data;
	}
}

/// <summary>
/// 将当前写入的位置向前移动
/// </summary>
/// <param name="buf">数据包</param>
/// <param name="size">需要移动的大小</param>
static void move_forward(pktbuf_t* buf, int size) {
	buf->pos += size;
	buf->blk_offset += size;
	
	pktblk_t* curr = buf->curr_blk;
	if (buf->blk_offset >= curr->data+curr->size) {
		buf->curr_blk = pktbuf_blk_next(curr);
		if (buf->curr_blk != nullptr) {
			buf->blk_offset = buf->curr_blk->data;
		}
		else {
			buf->blk_offset = nullptr;
		}
	}
}

net_err_t pktbuf_write(pktbuf_t* buf, uint8_t* src, int size)
{
	if (src == nullptr || size == 0) {
		return NET_ERR_PARAM;
	}

	int remain_size = total_blk_remain(buf);
	if (remain_size < size) {
		dbg_error(DBG_PKTBUF, "size error: size %d > remain_size %d.", size, remain_size);
		return NET_ERR_SIZE;
	}

	while (size) {
		int blk_size = curr_blk_remain(buf);

		int curr_copy = size > blk_size ? blk_size : size;
		plat_memcpy(buf->blk_offset, src, curr_copy);

		src += curr_copy;
		size -= curr_copy;

		move_forward(buf, curr_copy);
	}

	return NET_ERR_OK;
}

net_err_t pktbuf_read(pktbuf_t* buf, uint8_t* dest, int size)
{
	if (dest == nullptr || size == 0) {
		return NET_ERR_PARAM;
	}

	int remain_size = total_blk_remain(buf);
	if (remain_size < size) {
		dbg_error(DBG_PKTBUF, "size error: size %d > remain_size %d.", size, remain_size);
		return NET_ERR_SIZE;
	}

	while (size) {
		int blk_size = curr_blk_remain(buf);

		int curr_copy = size > blk_size ? blk_size : size;
		plat_memcpy(dest, buf->blk_offset, curr_copy);

		dest += curr_copy;
		size -= curr_copy;

		move_forward(buf, curr_copy);
	}

	return NET_ERR_OK;
}

net_err_t pktbuf_seek(pktbuf_t* buf, int offset)
{
	if (buf->pos == offset) {
		return NET_ERR_OK;
	}

	if ((offset < 0) || (offset >= buf->total_size)) {
		return NET_ERR_SIZE;
	}

	int move_bytes = 0;
	if (offset < buf->pos) {
		pktbuf_reset_acc(buf);
		move_bytes = offset;
	}
	else {
		move_bytes = offset - buf->pos;
	}

	while (move_bytes) {
		int remain_size = curr_blk_remain(buf);
		int curr_move = move_bytes > remain_size ? remain_size : move_bytes;

		move_forward(buf, curr_move);
		move_bytes -= curr_move;
	}
	return NET_ERR_OK;
}

net_err_t pktbuf_copy(pktbuf_t* dest, pktbuf_t* src, int size)
{
	if ((total_blk_remain(dest) < size) || (total_blk_remain(src) < size)) {
		return NET_ERR_SIZE;
	}

	while (size) {
		int dest_remain = curr_blk_remain(dest);
		int src_remain = curr_blk_remain(src);
		int copy_size = dest_remain > src_remain ? src_remain : dest_remain;
		copy_size = copy_size > size ? size : copy_size;

		plat_memcpy(dest->blk_offset, src->blk_offset, copy_size);

		move_forward(dest, copy_size);
		move_forward(src, copy_size);
		size -= copy_size;
	}

	return NET_ERR_OK;
}

net_err_t pktbuf_fill(pktbuf_t* buf, uint8_t v, int size)
{
	if (size == 0) {
		return NET_ERR_PARAM;
	}

	int remain_size = total_blk_remain(buf);
	if (remain_size < size) {
		dbg_error(DBG_PKTBUF, "size error: size %d > remain_size %d.", size, remain_size);
		return NET_ERR_SIZE;
	}

	while (size) {
		int blk_size = curr_blk_remain(buf);

		int curr_fill = size > blk_size ? blk_size : size;
		plat_memset(buf->blk_offset, v, curr_fill);

		size -= curr_fill;

		move_forward(buf, curr_fill);
	}

	return NET_ERR_OK;
}
