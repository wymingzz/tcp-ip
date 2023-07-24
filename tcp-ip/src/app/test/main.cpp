#include <stdio.h>
#include "sys_plat.h"
#include "echo/tcp_echo_client.h"
#include "echo/tcp_echo_server.h"
#include "net.h"
#include "netif_pcap.h"
#include "dbg.h"
#include "nlist.h"
#include "mblock.h"
#include "pktbuf.h"

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

void pktbuf_test() {
	pktbuf_t* buf = pktbuf_alloc(2000);
	pktbuf_free(buf);

	//buf = pktbuf_alloc(2000);
	//for (int i = 0; i < 16; i++) {
	//	pktbuf_add_header(buf, 33, true);
	//}

	//for (int i = 0; i < 16; i++) {
	//	pktbuf_remove_header(buf, 33);
	//}

	//for (int i = 0; i < 16; i++) {
	//	pktbuf_add_header(buf, 33, false);
	//}

	//for (int i = 0; i < 16; i++) {
	//	pktbuf_remove_header(buf, 33);
	//}

	//pktbuf_free(buf);

	//buf = pktbuf_alloc(8);
	//pktbuf_resize(buf, 32);
	//pktbuf_resize(buf, 288);
	//pktbuf_resize(buf, 4922);
	//pktbuf_resize(buf, 1921);
	//pktbuf_resize(buf, 288);
	//pktbuf_resize(buf, 32);
	//pktbuf_resize(buf, 8);
	//pktbuf_resize(buf, 0);
	//pktbuf_free(buf);

	//buf = pktbuf_alloc(689);
	//pktbuf_t* sbuf = pktbuf_alloc(892);
	//pktbuf_join(buf, sbuf);
	//pktbuf_free(buf);

	//buf = pktbuf_alloc(32);
	//pktbuf_join(buf, pktbuf_alloc(4));
	//pktbuf_join(buf, pktbuf_alloc(16));
	//pktbuf_join(buf, pktbuf_alloc(54));
	//pktbuf_join(buf, pktbuf_alloc(32));
	//pktbuf_join(buf, pktbuf_alloc(38));

	//pktbuf_set_cont(buf, 44);
	//pktbuf_set_cont(buf, 60);
	//pktbuf_set_cont(buf, 64);
	//pktbuf_set_cont(buf, 128);
	//pktbuf_set_cont(buf, 135);
	//pktbuf_free(buf);

	buf = pktbuf_alloc(32);
	pktbuf_join(buf, pktbuf_alloc(4));
	pktbuf_join(buf, pktbuf_alloc(16));
	pktbuf_join(buf, pktbuf_alloc(54));
	pktbuf_join(buf, pktbuf_alloc(32));
	pktbuf_join(buf, pktbuf_alloc(38));
	pktbuf_join(buf, pktbuf_alloc(512));
	pktbuf_join(buf, pktbuf_alloc(1000));

	pktbuf_reset_acc(buf);

	static uint16_t temp[1000];
	for (int i = 0; i < 1000; i++) {
		temp[i] = i;
	}
	pktbuf_write(buf, (uint8_t*)temp, buf->total_size);
	
	static uint16_t read_temp[1000];
	plat_memset(read_temp, 0, sizeof read_temp);
	
	pktbuf_reset_acc(buf);
	pktbuf_read(buf, (uint8_t*)read_temp, buf->total_size);
	if (plat_memcmp(temp, read_temp, buf->total_size) != 0) {
		plat_printf("net equal\n");
		return;
	}

	//plat_memset(read_temp, 0, sizeof read_temp);
	//pktbuf_seek(buf, 18 * 2);
	//pktbuf_read(buf, (uint8_t*)read_temp, 56);
	//if (plat_memcmp(temp+18, read_temp, 56) != 0) {
	//	plat_printf("net equal\n");
	//	return;
	//}

	//plat_memset(read_temp, 0, sizeof read_temp);
	//pktbuf_seek(buf, 85 * 2);
	//pktbuf_read(buf, (uint8_t*)read_temp, 256);
	//if (plat_memcmp(temp + 85, read_temp, 256) != 0) {
	//	plat_printf("net equal\n");
	//	return;
	//}

	pktbuf_t* dest = pktbuf_alloc(1024);
	pktbuf_seek(dest, 600);
	pktbuf_seek(buf, 200);
	pktbuf_copy(dest, buf, 122);

	plat_memset(read_temp, 0, sizeof read_temp);
	pktbuf_seek(dest, 600);
	pktbuf_read(dest, (uint8_t*)read_temp, 122);

	if (plat_memcmp(temp + 100, read_temp, 122) != 0) {
		plat_printf("net equal\n");
		return;
	}

	pktbuf_reset_acc(dest);
	pktbuf_fill(dest, 53, dest->total_size);
	plat_memset(read_temp, 0, sizeof read_temp);
	pktbuf_seek(dest, 0);
	pktbuf_read(dest, (uint8_t*)read_temp, dest->total_size);

	char* ptr = (char*)read_temp;
	for (int i = 0; i < dest->total_size; i++) {
		if (*ptr++ != 53) {
			plat_printf("net equal\n");
			return;
		}
	}

	pktbuf_free(dest);
	pktbuf_free(buf);
}

void basic_test() {
	//nlist_test();
	//mblock_test();
	pktbuf_test();
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