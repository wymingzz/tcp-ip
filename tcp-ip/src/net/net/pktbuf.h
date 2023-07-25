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
/// Ϊpktbuf����ռ�
/// </summary>
/// <param name="size">��Ҫ����Ĵ�С</param>
/// <returns>������ĵ�ַ</returns>
pktbuf_t* pktbuf_alloc(int size);

/// <summary>
/// �ͷ�������Ŀռ�
/// </summary>
/// <param name="buf">��Ҫ�ͷŵ�pktbuf</param>
void pktbuf_free(pktbuf_t* buf);

/// <summary>
/// ������ǰ�����ͷ���ṹ
/// </summary>
/// <param name="buf">����</param>
/// <param name="size">ͷ����С</param>
/// <param name="need_cont">ͷ���Ƿ���Ҫ����</param>
/// <returns>�Ƿ���ӳɹ�</returns>
net_err_t pktbuf_add_header(pktbuf_t* buf, int size, bool need_cont);

/// <summary>
/// �Ƴ�����ǰ���ͷ���ṹ
/// </summary>
/// <param name="buf">����</param>
/// <param name="size">ͷ����С</param>
/// <returns>�Ƿ��Ƴ��ɹ�</returns>
net_err_t pktbuf_remove_header(pktbuf_t* buf, int size);

/// <summary>
/// �������ݵĴ�С
/// </summary>
/// <param name="buf">����</param>
/// <param name="size">������Ĵ�С</param>
/// <returns>�Ƿ�ɹ�</returns>
net_err_t pktbuf_resize(pktbuf_t* buf, int size);

/// <summary>
/// �Ƴ����ݺ���������ṹ
/// </summary>
/// <param name="buf">����</param>
/// <param name="size">��Ҫ�Ƴ��Ĵ�С</param>
/// <returns>�Ƿ��Ƴ��ɹ�</returns>
net_err_t pktbuf_remove_tail(pktbuf_t* buf, int size);

/// <summary>
/// �ϲ��������ݰ�
/// </summary>
/// <param name="dest">��󱣴�����ݰ�</param>
/// <param name="src">��Ҫ��������ݰ�(�����걻�ͷ�)</param>
/// <returns>�Ƿ�ϲ��ɹ�</returns>
net_err_t pktbuf_join(pktbuf_t* dest, pktbuf_t* src);

/// <summary>
/// �����ݰ�ͷ���ṹ������һ���ڵ���
/// </summary>
/// <param name="buf">��Ҫ���������ݰ�</param>
/// <param name="size">ͷ����С</param>
/// <returns></returns>
net_err_t pktbuf_set_cont(pktbuf_t* buf, int size);

/// <summary>
/// ˢ��posλ�ã����õ���ͷ
/// </summary>
/// <param name="buf">���ݰ�</param>
void pktbuf_reset_acc(pktbuf_t* buf);

/// <summary>
/// �����ݰ���д������
/// </summary>
/// <param name="buf">���ݱ�</param>
/// <param name="src">����Դ</param>
/// <param name="size">д������ݴ�С</param>
/// <returns>�Ƿ�д��ɹ�</returns>
net_err_t pktbuf_write(pktbuf_t* buf, uint8_t* src, int size);

/// <summary>
/// �����ݰ��ĵ�ǰλ�ÿ�ʼ��ǰ��ȡ����
/// </summary>
/// <param name="buf">���ݰ�</param>
/// <param name="dest">��Ҫд�������</param>
/// <param name="size">��ȡ�Ĵ�С</param>
/// <returns>�Ƿ��ȡ�ɹ�</returns>
net_err_t pktbuf_read(pktbuf_t* buf, uint8_t* dest, int size);

/// <summary>
/// �ض�λ���ݰ��Ķ�дλ��
/// </summary>
/// <param name="buf">���ݰ�</param>
/// <param name="offset">�ض�λ�����ڿ�ͷ��ƫ����</param>
/// <returns>�Ƿ��ض�λ�ɹ�</returns>
net_err_t pktbuf_seek(pktbuf_t* buf, int offset);

/// <summary>
/// ��һ�����ݰ������ݿ�������һ�����ݰ���
/// </summary>
/// <param name="dest">Ŀ�����ݰ�</param>
/// <param name="src">Դ���ݰ�</param>
/// <param name="size">��Ҫ�����Ĵ�С</param>
/// <returns>�Ƿ񿽱��ɹ�</returns>
net_err_t pktbuf_copy(pktbuf_t* dest, pktbuf_t* src, int size);

/// <summary>
/// ������ݰ��е�����
/// </summary>
/// <param name="buf">���ݰ�</param>
/// <param name="v">�Ѹ��ֽڽ������</param>
/// <param name="size">���Ĵ�С</param>
/// <returns>�Ƿ����ɹ�</returns>
net_err_t pktbuf_fill(pktbuf_t* buf, uint8_t v, int size);

/// <summary>
/// ���ݰ�����
/// </summary>
/// <param name="buf">�����õ����ݰ�</param>
void pktbuf_inc_ref(pktbuf_t* buf);

/**
 * ��ȡ��ǰblock����һ���Ӱ�
 */
static inline pktblk_t* pktbuf_blk_next(pktblk_t* blk) {
	nlist_node_t* next = blk->node.next;
	return nlist_entry(next, pktblk_t, node);
}

/**
 * ��ȡ��ǰblock����һ���Ӱ�
 */
static inline pktblk_t* pktbuf_blk_pre(pktblk_t* blk) {
	nlist_node_t* pre = blk->node.pre;
	return nlist_entry(pre, pktblk_t, node);
}

/**
 * ȡbuf�ĵ�һ�����ݿ�
 * @param buf ���ݻ���buf
 * @return ��һ�����ݿ�
 */
static inline pktblk_t* pktbuf_first_blk(pktbuf_t* buf) {
	nlist_node_t* first = buf->blk_list.first;
	return nlist_entry(first, pktblk_t, node);
}

/**
 * ȡbuf�����һ�����ݿ�
 * @param buf ���ݻ���buf
 * @return ���һ�����ݿ�
 */
static inline pktblk_t* pktbuf_last_blk(pktbuf_t* buf) {
	nlist_node_t* first = buf->blk_list.last;
	return nlist_entry(first, pktblk_t, node);
}

/// <summary>
/// �������ݰ��е�һ�����ݿ��data��ʼָ��
/// </summary>
/// <param name="buf">���ݰ�</param>
/// <returns>���ݰ��е�һ�����ݿ��data��ʼָ��</returns>
static inline uint8_t* pktbuf_data(pktbuf_t* buf) {
	pktblk_t* first = pktbuf_first_blk(buf);
	return first == nullptr ? nullptr : first->data;
}

#endif // !PKTBUF_H
