#include "mblock.h"
#include "dbg.h"

net_err_t mblock_init(mblock_t* mblock, void* mem, int blk_size, int cnt, nlocker_type_t locker_type)
{
	uint8_t* buf = (uint8_t*)mem;

	nlist_init(&mblock->free_list);
	for (int i = 0; i < cnt; i++,buf += blk_size) {
		nlist_node_t* block = (nlist_node_t*)buf;
		nlist_node_init(block);
		nlist_insert_last(&mblock->free_list, block);
	}

	if (nlocker_init(&mblock->locker, locker_type) == NET_ERR_SYS) {
		dbg_error(DBG_MBLOCK, "create locker failed.");
		return NET_ERR_SYS;
	}

	if (locker_type != NLOCKER_NONE) {
		mblock->alloc_sem = sys_sem_create(cnt);
		if (mblock->alloc_sem == SYS_SEM_INVALID) {
			nlocker_destroy(&mblock->locker);
			dbg_error(DBG_MBLOCK, "create sem failed.");
			return NET_ERR_SYS;
		}
	}

	return NET_ERR_OK;
}

void* mblock_alloc(mblock_t* mblock, int ms)
{
	if ((ms < 0) || (mblock->locker.type == NLOCKER_NONE))
	{
		nlocker_lock(&mblock->locker);
		int count = mblock->free_list.count;

		if (count == 0) {
			nlocker_unlock(&mblock->locker);
			return nullptr;
		}

		nlist_node_t* block = nlist_remove_first(&mblock->free_list);
		nlocker_unlock(&mblock->locker);
		return block;
	}

	if (sys_sem_wait(mblock->alloc_sem, ms) < 0) {
		return nullptr;
	}

	nlocker_lock(&mblock->locker);
	nlist_node_t* block = nlist_remove_first(&mblock->free_list);
	nlocker_unlock(&mblock->locker);

	return block;
}

int mblock_free_cnt(mblock_t* mblock)
{
	nlocker_lock(&mblock->locker);
	int count = mblock->free_list.count;
	nlocker_unlock(&mblock->locker);
	return count;
}

void mblock_free(mblock_t* mblock, void* block)
{
	nlocker_lock(&mblock->locker);
	nlist_insert_last(&mblock->free_list, (nlist_node_t*)block);
	nlocker_unlock(&mblock->locker);
	
	if (mblock->locker.type != NLOCKER_NONE) {
		sys_sem_notify(mblock->alloc_sem);
	}
}

void mblock_destroy(mblock_t* mblock)
{
	if (mblock->locker.type != NLOCKER_NONE) {
		sys_sem_free(mblock->alloc_sem);
	}
	nlocker_destroy(&mblock->locker);
}
