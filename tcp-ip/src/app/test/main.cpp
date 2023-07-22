#include <stdio.h>
#include "sys_plat.h"
#include "echo/tcp_echo_client.h"
#include "echo/tcp_echo_server.h"
#include "net.h"
#include "netif_pcap.h"
#include "dbg.h"
#include "nlist.h"
#include "mblock.h"

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

typedef struct _tnode_t {
	nlist_node_t node;
	int id;
	_tnode_t() :id(0) {};
}tnode_t;

void nlist_test() {
#define NODE_CNT 4
	tnode_t node[NODE_CNT];
	nlist_t list;

	plat_printf("insert first\n");
	for (int i = 0; i < NODE_CNT; i++) {
		node[i].id = i;
		nlist_insert_first(&list, &node[i].node);
	}
	nlist_node_t* p;
	plat_printf("remove first\n");
	for (int i = 0; i < NODE_CNT; i++) {
		p = nlist_remove_first(&list);
		tnode_t* tnode = nlist_entry(p, tnode_t, node);
		plat_printf("id = %d\n", tnode ? tnode->id : -1);
	}
	plat_printf("list count: %d\n", list.count);

	plat_printf("insert last\n");

	for (int i = 0; i < NODE_CNT; i++) {
		node[i].id = i;
		nlist_insert_last(&list, &node[i].node);
	}

	plat_printf("remove first\n");
	for (int i = 0; i < NODE_CNT; i++) {
		p = nlist_remove_first(&list);
		tnode_t* tnode = nlist_entry(p, tnode_t, node);
		plat_printf("id = %d\n", tnode ? tnode->id : -1);
	}

	plat_printf("insert after\n");

	for (int i = 0; i < NODE_CNT; i++) {
		node[i].id = i;
		nlist_insert_after(&list, list.first, &node[i].node);
	}

	nlist_for_each(p, &list) {
		tnode_t* tnode = nlist_entry(p, tnode_t, node);
		plat_printf("id: %d\n", tnode->id);
	}
}

void mblock_test() {
	mblock_t blist;
	static uint8_t buffer[100][10];

	mblock_init(&blist, &buffer, 100, 10, NLOCKER_THREAD);
	
	void* temp[10];
	for (int i = 0; i < 10; i++) {
		temp[i]= mblock_alloc(&blist, 0);
		plat_printf("block: %p, free_count: %d\n", temp[i], mblock_free_cnt(&blist));
	}

	for (int i = 0; i < 10; i++) {
		mblock_free(&blist, temp[i]);
		plat_printf("free_count: %d\n", mblock_free_cnt(&blist));
	}

	mblock_destroy(&blist);
}

void basic_test() {
	nlist_test();
	mblock_test();
}

int main(void) {

	net_init();

	basic_test();

	net_start();

	netdev_init();
	
	while (1) {
		sys_sleep(10);
	}
	return 0;
}