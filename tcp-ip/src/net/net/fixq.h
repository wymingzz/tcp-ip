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
/// ������Ϣ���еĳ�ʼ������
/// </summary>
/// <param name="q">������Ϣ���е�ָ��</param>
/// <param name="buf">�������ݵ���ʼ��ַ</param>
/// <param name="size">����Ԫ�ص�����</param>
/// <param name="type">��������</param>
/// <returns>�Ƿ��ʼ���ɹ�</returns>
net_err_t fixq_init(fixq_t* q, void** buf, int size, nlocker_type_t type);

/// <summary>
/// ����Ϣ������д����Ϣ
/// </summary>
/// <param name="q">��Ϣ����</param>
/// <param name="msg">���͵���Ϣ</param>
/// <param name="ms">�ȴ�ʱ��</param>
/// <returns></returns>
net_err_t fixq_send(fixq_t* q, void* msg, int ms);

/// <summary>
/// ����Ϣ������ȡ����Ϣ
/// </summary>
/// <param name="q">��Ϣ����</param>
/// <param name="ms">�ɽ��ܵĵȴ�ʱ��</param>
/// <returns>��Ϣ��ַ</returns>
void* fixq_recv(fixq_t* q, int ms);

/// <summary>
/// ������Ϣ����
/// </summary>
/// <param name="q">��Ҫ���ٵ���Ϣ����</param>
void fixq_destroy(fixq_t* q);

/// <summary>
/// ��ȡ��Ϣ�����е���Ϣ����
/// </summary>
/// <param name="q">��Ϣ����</param>
/// <returns></returns>
int fixq_count(fixq_t* q);

#endif // !FIXQ_H
