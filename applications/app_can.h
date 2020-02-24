#ifndef __APP_CAN_H__
#define __APP_CAN_H__

#include <rtdevice.h>

rt_err_t can_open(const char *name);
rt_err_t can_recvmsg(struct rt_can_msg *msg);
rt_size_t can_sendmsg(const struct rt_can_msg *data);

#endif

