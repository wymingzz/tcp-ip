#ifndef SYS_H
#define SYS_H

#include "sys_plat.h"

sys_sem_t sys_sem_create(int init_count);
void sys_sem_free(sys_sem_t sem);
int sys_sem_wait(sys_sem_t sem, uint32_t ms);
void sys_sem_notify(sys_sem_t sem);

// �����ź������ɾ���ƽ̨ʵ��
sys_mutex_t sys_mutex_create(void);
void sys_mutex_free(sys_mutex_t mutex);
void sys_mutex_lock(sys_mutex_t mutex);
void sys_mutex_unlock(sys_mutex_t mutex);
int sys_mutex_is_valid(sys_mutex_t mutex);

// �߳���أ��ɾ���ƽ̨ʵ��
typedef void (*sys_thread_func_t)(void* arg);
sys_thread_t sys_thread_create(sys_thread_func_t entry, void* arg);
void sys_thread_exit(int error);
void sys_sleep(int ms);
sys_thread_t sys_thread_self(void);

void sys_plat_init(void);

#endif // !SYS_H
