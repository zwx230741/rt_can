#include "app_can.h"

#define CAN_ID 0x12

/* can_device句柄 */
static rt_device_t can_device = RT_NULL;

/* 用于接收消息的消息队列*/
static rt_mq_t rx_mq;

static int count = 0;

void can_set(const rt_uint16_t c)
{
    struct can_configure cfg = {0};
    rt_uint32_t arg = CAN1MBaud;
    rt_device_control(can_device, RT_CAN_CMD_SET_BAUD, &arg);
}

static rt_err_t rx_ind(rt_device_t dev, rt_size_t size)
{
    /* 发送消息到消息队列中*/
    rt_mq_send(rx_mq, &size, sizeof(size));

    //rt_kprintf("rx_ind size %d.\n", size);

    return RT_EOK;
}

rt_err_t can_recvmsg(struct rt_can_msg *msg)
{
    rt_size_t size = 0;
    struct rt_can_msg can_msg = {0};
    rt_err_t result = RT_EOK;

    rt_uint32_t rx_length;

    /* 从消息队列中读取消息*/
    result = rt_mq_recv(rx_mq, &size, sizeof(size), 0);

    //rt_kprintf("can_recvmsg size %d.\n", size);

    if(result == -RT_ETIMEOUT)
    {
        /* 接收超时*/
        //rt_kprintf("timeout count:%d\n", ++count);
        return -RT_ETIMEOUT;
    }

    /* 成功收到消息*/
    if(result == RT_EOK)
    {
        count = 0;


        /* 读取消息*/
        rx_length = rt_device_read(can_device, 0, &can_msg,
                sizeof(can_msg));

        if(rx_length > 0)
        {
#if 0
            rt_kprintf("can_recvmsg can_msg.id %x\n", can_msg.id);
            rt_kprintf("can_recvmsg can_msg.ide %d\n", can_msg.ide);
            rt_kprintf("can_recvmsg can_msg.len %d\n", can_msg.len);

            int i = 0;
            rt_kprintf("can_recvmsg can_msg.data: ");

            for(i = 0; i < can_msg.len; i++)
            {
                rt_kprintf("%x ", can_msg.data[i]);
            }

            rt_kprintf("\n");
#endif

            rt_memcpy(msg, &can_msg, sizeof(can_msg));
            //rt_kprintf("can_msg %08x\n",can_msg.id);
            return rx_length;
        }
    }

    return -RT_ERROR;
}

rt_size_t can_sendmsg(const struct rt_can_msg *data)
{
    struct rt_can_msg msg = {0};

    rt_memcpy(&msg, data, sizeof(msg));
    msg.ide = RT_CAN_EXTID;
    msg.rtr = 0;

    return rt_device_write(can_device, 0, &msg, sizeof(msg));
}

rt_err_t can_open(const char *name)
{
    rt_err_t res;

    /* 查找系统中的can设备 */
    can_device = rt_device_find(name);

    /* 查找到设备后将其打开 */
    if(can_device != RT_NULL)
    {

        rx_mq = rt_mq_create("rx_mq", sizeof(rt_size_t), 100, RT_IPC_FLAG_FIFO);

        if(rx_mq == RT_NULL)  /* create failed */
        {
            rt_set_errno(RT_ERROR);
            return -RT_ERROR;
        }

        res = rt_device_set_rx_indicate(can_device, rx_ind);

        /* 检查返回值 */
        if(res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n", name, res);
            return -RT_ERROR;
        }

        /* 打开设备，以可读写、中断方式 */
        res = rt_device_open(can_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);

        /* 检查返回值 */
        if(res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n", name, res);
            return -RT_ERROR;
        }
    }
    else
    {
        rt_kprintf("can't find %s device.\n", name);
        return -RT_ERROR;
    }

    return RT_EOK;
}

