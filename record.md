开发记录
# ZigBee程序大概的运行流程（含ZigBee协议栈）
|步骤|相关函数|
|:---|---:|
|1. 关闭中断| ```osal_int_disable( INTS_ALL );```|
|2. 初始化开发版的一些资源|```HAL_BOARD_INIT();```|
|3. 检测电源电压是否合适|```zmain_vdd_check();```|
|4. 初始化开发版IO|```InitBoard( OB_COLD );```|
|5. 初始化硬件驱动|```HalDriverInit();```|
|6. 初始化NV系统|```osal_nv_init( NULL );```|
|7. 初始化MAC|```ZMacInit();```|
|8. 判断扩展地址|```zmain_ext_addr();```|
|9. 初始化NV|```zgInit();```|
|10. 初始化操作系统抽象层|```osal_init_system();```|
|11. 使能中断|```osal_int_enable( INTS_ALL );```|
|12. 初始化开发版|```InitBoard( OB_READY );	```|
|13. 如果有看门狗，则使能看门狗|
|14. 初始化自己的配置|```customInit(); ```|
|15. 启动操作系统|```osal_start_system();```在这个函数添加自己的任务，如果没有任务则OS会进入休眠|

# ZigBee模块引脚连接图

|引脚|内容|
|:---|:---|
|P07|继电器正极|
|P02|UART0_RX|
|P03|UART0_TX|
