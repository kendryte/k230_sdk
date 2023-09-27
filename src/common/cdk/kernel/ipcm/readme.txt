IPCM(Inter-Processor Communication Module)用于多核之间的通信。

1. 配置说明

	配置文件目录：arch/xxx/configs,xx为芯片平台
	各个配置项说明如下:
	platform ,芯片平台（如k230）
	node_id ,节点编号（主节点为0）
	arch_type ,架构类型（根据芯片平台选择）
	node_name, 节点名称
	os_type, OS类型（根据芯片平台选择）
	top_role, 节点类型（master 或 slave, node_id为0时为master）
	ipcm_irq, 中断号
	cdev 支持设备节点操作，节点名为/dev/ipcm
	shm_phys_XtoY, 配置的共享内存区域基址（从X发送到Y）
	shm_size_XtoY, 配置的共享内存区域大小（从X发送到Y）
	cross_compile，编译器
	cc_flags, 编译增加选项
	ld_flags, 链接增加选项
	kernel_dir, 依赖OS编译环境

==================================================================================================================
2. 编译方法

1). 编译，执行以下操作
	make PLATFORM=xxx CFG=xxx_xxxxx_config all
	xxx为所要选择的芯片，xxx_xxxxx_config 为所要选择的配置文件
	例如:
	make PLATFORM=k230  CFG=k230_riscv_rtsmart_config all
	(编译完成后，在out/node/下面生成对应节点目录生成目标文件，.a文件或.ko文件等)
	注意：先要产生kernel_dir指定的OS依赖编译环境
2). 清除编译信息，执行以下操作
	make PLATFORM=xxx  CFG=xxx_xxxxx_linux_config clean
	清除编译整个信息，执行以下操作
	make clean
==================================================================================================================
3. 虚拟文件系统设备节点操作

	命令，参数及返回值等放在include/ipcm_userdev.h中，用户需要包含此头文件
1). rt-smart调用_ipcm_vdd_init初始化，Linux用命令insmod k_ipcm.ko 加载
2). 通过/dev/ipcm设备节点对IPCM进行操作
	a. open          打开设备节点/dev/ipcm
	b. ioctl         使用命令K_IPCM_IOC_ATTR_INIT初始化ipcm_handle_attr配置属性
	c. ioctl         使用命令K_IPCM_IOC_CONNECT/K_IPCM_IOC_TRY_CONNECT建立端口连接
	c. read/write    数据接收与发送(read支持select)
	d. ioctl         使用命令K_IPCM_IOC_DISCONNECT断开端口连接
	e. close         关闭设备节点

	struct ipcm_handle_attr 配置说明：
    __________________________________________________________________________________________________________
    成员        |说明
    ____________|_____________________________________________________________________________________________
    target      |建立端口连接时用于配置目标节点ID
    port        |建立端口连接时用于配置一个端口号（端口号支持0~1023）
    priority    |优先级，配置为中断(HANDLE_MSG_PRIORITY,高优先级)或查询(HANDLE_MSG_NORMAL,低优先级)方式通信
    remote_ids  |用于获得各节点状态(1:就绪，0:离线).准备就绪的节点可以建立连接.
    ____________|_____________________________________________________________________________________________

    ioctl 命令说明：
    ______________________________________________________________________________________________________________
    命令                        |参数                       |返回值         |说明
    ____________________________|___________________________|_______________|_____________________________________
    K_IPCM_IOC_CONNECT          |struct ipcm_handle_attr *  |0              |配置target、port建立端口连接，阻塞型
    K_IPCM_IOC_TRY_CONNECT      |struct ipcm_handle_attr *  |0              |配置target、port建立端口连接，非阻塞型
    K_IPCM_IOC_CHECK            |NULL                       |handle当前状态 |handle的状态为handle_state其中一种
    K_IPCM_IOC_DISCONNECT       |NULL                       |0              |断开通信连接
    K_IPCM_IOC_GET_LOCAL_ID     |NULL                       |本地设备ID     |获得本地设备ID
    K_IPCM_IOC_GET_REMOTE_ID    |struct ipcm_handle_attr *  |节点数         |获得节点个数及各节点状态
    K_IPCM_IOC_GET_REMOTE_STS   |id                         |节点状态       |获得id号节点的就绪状态
    K_IPCM_IOC_ATTR_INIT        |struct ipcm_handle_attr *  |0              |初始化一个handle节点配置属性
    ____________________________|_________________________________________________________________________________

==================================================================================================================
4. 使用限制

1).配置共享内存内存区域应该保证基址和长度都按4KBytes对齐.
2).需要一个主节点，一个或多个从节点，但最多支持8个节点.
3).消息发送长度不能大于1MBytes.
4).建立连接后，如果一端断开连接，则两端都等同断开，再次建立连接时需要两端都发起连接.
==================================================================================================================
