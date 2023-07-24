#ifndef NET_ERR_H
#define NET_ERR_H

typedef enum _net_err_t {
	NET_ERR_MIN = -255,
	NET_ERR_SIZE,
	NET_ERR_TMO,
	NET_ERR_FULL,
	NET_ERR_MEM,
	NET_ERR_SYS,
	NET_ERR_PARAM,
	NET_ERR_OK = 0,

} net_err_t;

#endif // !NET_ERR_H
