#ifndef NET_CFG_H
#define NET_CFG_H

// dbg
#define DBG_MBLOCK DBG_LEVEL_INFO
#define DBG_FIXQ DBG_LEVEL_INFO
#define DBG_EXMSG DBG_LEVEL_INFO
#define DBG_PKTBUF DBG_LEVEL_INFO

// mlocker
#define EXMSG_LOCKER NLOCKER_THREAD

// exmsg
#define EXMSG_MSG_CNT 10

// pktbuf
#define PKTBUF_BLK_SIZE 128
#define PKTBUF_BLK_CNT 100
#define PKTBUF_BUF_CNT 100

#endif // !NET_CFG_H
