#include <stdio.h>
#include "sys_plat.h"

void thread1_entry(void* arg) {
	while (1) {
		plat_printf("this is thread1: %s\n", (char*)arg);
		sys_sleep(1000);
	}
}

void thread2_entry(void* arg) {
	while (1) {
		plat_printf("this is thread2: %s\n", (char*)arg);
	}
}

int main(void) {
	sys_thread_create(thread1_entry, (void*)"aaa");
	sys_thread_create(thread2_entry, (void*)"bbb");
	pcap_t* pcap = pcap_device_open(netdev0_phy_ip, netdev0_hwaddr);
	while (pcap) {

	}
	return 0;
}