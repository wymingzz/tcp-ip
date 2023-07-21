#include <stdio.h>
#include "sys_plat.h"
#include "echo/tcp_echo_client.h"
#include "echo/tcp_echo_server.h"
#include "net.h"
#include "netif_pcap.h"
#include "dbg.h"

static sys_sem_t sem;
static int counter;
static sys_mutex_t latch;

static char buffer[100];
static int write_index, read_index;
static sys_sem_t read_sem,write_sem;

void thread1_entry(void* arg) {
	for (int i = 0; i < 10 * sizeof(buffer); i++) {
		sys_sem_wait(read_sem, 0);
		char data = buffer[read_index++];
		sys_sem_notify(write_sem);
		if (read_index >= sizeof(buffer)) {
			read_index = 0;
		}
		plat_printf("thread1: read data = %d\n", (int)data);
		sys_sleep(1000);
	}
}

void thread2_entry(void* arg) {
	for (int i = 0; i < 10 * sizeof(buffer); i++) {
		sys_sem_wait(write_sem, 0);
		buffer[write_index++] = i;
		sys_sem_notify(read_sem);
		if (write_index >= sizeof(buffer)) {
			write_index = 0;
		}
		plat_printf("thread2: write data = %d\n", i);
	}
}

net_err_t netdev_init(void) {
	netif_pcap_open();

	return NET_ERR_OK;
}

#define DBG_TEST DBG_LEVEL_INFO

int main(void) {
	dbg_info(DBG_TEST, "info");
	dbg_warning(DBG_TEST, "warning");
	dbg_error(DBG_TEST, "error");

	net_init();

	net_start();

	netdev_init();
	
	while (1) {
		sys_sleep(10);
	}
	return 0;
}