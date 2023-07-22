#ifndef EXMSG_H
#define EXMSG_H

#include "net_err.h"
#include "nlist.h"
enum msg_enmu
{
	NET_EXMSG_NETIF_IN,
};

typedef struct _exmsg_t {
	nlist_node_t node;
	enum msg_enmu type;

	int id;
}exmsg_t;

net_err_t exmsg_init(void);
net_err_t exmsg_start(void);
/// <summary>
/// 网卡将网络中的数据包传给工作线程
/// </summary>
/// <returns></returns>
net_err_t exmsg_netif_in();

#endif // !EXMSG_H
