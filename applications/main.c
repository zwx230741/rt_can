/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-09     RT-Thread    first version
 */

#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include "app_can.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* PLEASE DEFINE the LED0 pin for your board, such as: PA5 */
#define LED0_PIN    GET_PIN(C, 0)

void processTxRxOnce(void)
{
    struct rt_can_msg msg = {0};

    msg.id = 0x78;              /* ID 为 0x78 */
    msg.ide = RT_CAN_STDID;     /* 标准格式 */
    msg.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg.len = 8;                /* 数据长度为 3 */
    /* 待发送的 3 字节数据 */
    msg.data[0] = 0x00;
    msg.data[1] = 0x11;
    msg.data[2] = 0x22;
    msg.data[3] = 0x00;
    msg.data[4] = 0x11;
    msg.data[5] = 0x00;
    msg.data[6] = 0x11;
    msg.data[7] = 0x22;

    const rt_size_t tx_res = can_sendmsg(&msg);

    if(tx_res == 0)           // Failure - drop the frame and report
    {
        //canardPopTxQueue(&canard_);
        rt_kprintf("Transmit error %d, frame dropped\n", tx_res);
    }

    return;
}

void canard_thread_entry(void* parameter)
{
    if (can_open("can1") != RT_EOK)
    {
        rt_kprintf("can1 open error.\n");
        while (1)
        {
            rt_thread_delay(10);
        }
    }

    for(;;)
    {
        processTxRxOnce();
        //rt_kprintf("processTxRxOnce\n");
        rt_thread_mdelay(5);
    }
}

int main(void)
{
    rt_thread_t canard_id;

    /* 创建canard线程 */
    canard_id = rt_thread_create("canard",
            canard_thread_entry,
            RT_NULL,
            512,
            2,
            15);

    /* 创建成功则启动线程 */
    if (canard_id != RT_NULL)
        rt_thread_startup(canard_id);

    int count = 1;
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

    while (count++)
    {
        /* set LED0 pin level to high or low */
        rt_pin_write(LED0_PIN, count % 2);
        LOG_D("Hello RT-Thread!");
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}
