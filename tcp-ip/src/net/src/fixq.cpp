#include "fixq.h"
#include "dbg.h"

net_err_t fixq_init(fixq_t* q, void** buf, int size, nlocker_type_t type)
{
	q->size = size;
	q->in = q->out = q->cnt = 0;
	q->buf = buf;

	net_err_t err = nlocker_init(&q->locker, type);
	if (err < 0) {
		dbg_error(DBG_FIXQ, "init locker failed.");
		return err;
	}

	q->send_sem = sys_sem_create(size);
	if (q->send_sem == SYS_SEM_INVALID) {
		dbg_error(DBG_FIXQ, "create send sem failed.");
		err = NET_ERR_SYS;
		goto send_failed;
	}
	q->recv_sem = sys_sem_create(0);
	if (q->recv_sem == SYS_SEM_INVALID) {
		dbg_error(DBG_FIXQ, "create recv sem failed.");
		err = NET_ERR_SYS;
		goto recv_failed;
	}
	return NET_ERR_OK;
	sys_sem_free(q->recv_sem);
recv_failed:
	sys_sem_free(q->send_sem);
send_failed:
	nlocker_destroy(&q->locker);
	return err;
}

net_err_t fixq_send(fixq_t* q, void* msg, int ms)
{
	nlocker_lock(&q->locker);
	if ((ms < 0) && (q->cnt >= q->size)) {
		nlocker_unlock(&q->locker);
		return NET_ERR_FULL;
	}
	nlocker_unlock(&q->locker);

	if (sys_sem_wait(q->send_sem, ms) < 0) {
		return NET_ERR_TMO;
	}

	nlocker_lock(&q->locker);
	q->buf[q->in++] = msg;
	if (q->in >= q->size) {
		q->in = 0;
	}
	q->cnt++;
	nlocker_unlock(&q->locker);

	sys_sem_notify(q->recv_sem);
	return NET_ERR_OK;
}

void* fixq_recv(fixq_t* q, int ms)
{
	nlocker_lock(&q->locker);
	if (!q->cnt && (ms < 0)) {
		nlocker_unlock(&q->locker);
		return nullptr;
	}
	nlocker_unlock(&q->locker);

	if (sys_sem_wait(q->recv_sem, ms) < 0) {
		return nullptr;
	}

	nlocker_lock(&q->locker);
	void* msg = q->buf[q->out++];
	if (q->out >= q->size) {
		q->out = 0;
	}
	q->cnt--;
	nlocker_unlock(&q->locker);

	sys_sem_notify(q->send_sem);
	return msg;
}

void fixq_destroy(fixq_t* q)
{
	nlocker_destroy(&q->locker);
	sys_sem_free(q->recv_sem);
	sys_sem_free(q->send_sem);
}

int fixq_count(fixq_t* q)
{
	nlocker_lock(&q->locker);
	int count = q->cnt;
	nlocker_unlock(&q->locker);
	return count;
}
