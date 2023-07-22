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
/// ��ʼ��mblock
/// </summary>
/// <param name="mblock">��Ҫ��ʼ����mblock</param>
/// <param name="mem">�������ʼλ��</param>
/// <param name="blk_size">����Ԫ�صĴ�С</param>
/// <param name="cnt">����������</param>
/// <param name="locker_type">locker������</param>
/// <returns>�Ƿ��ʼ���ɹ�</returns>
net_err_t mblock_init(mblock_t* mblock, void* mem, int blk_size, int cnt, nlocker_type_t locker_type);

/// <summary>
/// ��������ڴ��
/// </summary>
/// <param name="mblock"></param>
/// <param name="ms">�޷���ȡmblockʱϣ���ĵȴ�ʱ�䣬ms&lt;0���ȴ���ms=0���ȣ�����ȴ�x����</param>
/// <returns>�п����ڴ淵���ڴ��ַ�����򷵻�nullptr</returns>
void* mblock_alloc(mblock_t* mblock, int ms);

/// <summary>
/// ���ؿ�������ĳ���
/// </summary>
/// <param name="mblock"></param>
/// <returns>һ��intֵ��Ϊ��������ĳ���</returns>
int mblock_free_cnt(mblock_t* mblock);

/// <summary>
/// �ͷ��ڴ��
/// </summary>
/// <param name="mblock"></param>
/// <param name="block">��Ҫ�ͷŵ��ڴ��</param>
void mblock_free(mblock_t* mblock,void *block);


/// <summary>
/// ����mblock
/// </summary>
/// <param name="mblock"></param>
void mblock_destroy(mblock_t* mblock);

#endif // !MBLOCK_H
