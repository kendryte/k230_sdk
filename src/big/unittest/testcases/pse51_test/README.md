# PSE51 测试用例

`conformance/interfaces` 文件夹下包含 PSE51 官方的 [linux-test-project](https://github.com/linux-test-project/ltp) 和我们自己编写的测试用例。

我们自己编写的测试用例文件夹使用 **C库_h** （例：stdio_h）命名，文件夹内部均为单独的.c文件。

## 使用说明

### 1.

首先进入 `pse51_test/` 目录下，使用`scons --dir=testcases/pse51_test`编译测试用例：

```shell
cd software/userapps
scons --dir=testcases/ pse51_test
```

编译完成后，会在` userapps/testcases/pse51_test/bin`目录下生成可执行文件，而这些文件数量较多，就需要分批进行测试。

所有这些文件会自动分成11个组，例如：

```bash
bin/
├── ctype_h
├── mman_tc
├── other_tc
├── pthread_tc
├── sched_tc
├── semaphore_tc
├── signal_tc
├── std_tc
├── str_tc
├── time_tc
└── timer_tc
```

每一类测试用例，都有对应的python脚本用来进行自动化测试。目前共有如下几个自动测试脚本：  
在 userapps/testcases/pse51_test/qemu 目录下
`run_ctype_tc.py`  
`run_pthread_tc.py`    
`run_signal_tc.py`  
`run_time_tc.py`  
`run_mman_tc.py`   
`run_sched_tc.py`      
`run_std_tc.py`     
`run_timer_tc.py`  
`run_other_tc.py`  
`run_semaphore_tc.py`  
`run_str_tc.py`  

### 2.

生成可执行文件后，将其中一个文件夹，复制到`userapps/root/bin`目录下，来生成`romfs.c`

示例(以`pthread_tc`文件夹为例)：
```bash
# 在该目录下：software/userapps
# 将`pthread_tc`文件夹复制到`userapps/root/bin/`目录
cp -r testcases/pse51_test/bin/pthread_tc root/bin/

# 必须在software/userapps目录下执行该命令
for file in `find root/bin -type f`; do riscv64-unknown-linux-musl-strip -v $file; done
```

### 3.

和正常编译烧录程序步骤相同，在使用mkromfs.py生成`romfs.c`

```bash
python3 ../tools/mkromfs.py root ../kernel/bsp/maix3/applications/romfs.c
```

### 4.

进入到 `software/kernel/bsp/maix3` 目录中，运行 scons 进行编译：

```bash
# 在software/kernel/bsp/maix3目录下
scons
```

### 5.

再进入`software/opensbi`目录，运行`build_env.sh`

```bash
# 在software/opensbi目录下
./build_env.sh
```

所需文件 `fw_payload.bin` 或 `fw_payload.elf` 就生成在 `build/platform/kendryte/fpgac908/firmware` 目录中；

### 6.

修改`userapps/testcases/pse51_test/qemu`目录下的`qemubox.py`文件，

找到文件中的 `spawn_cmd` 变量，将其内容改为打开串口调试终端的命令，例如`sudo minicom -D /dev/ttyUSB2 -b 38400`

```py
spawn_cmd = 'sudo minicom -D /dev/ttyUSB2 -b 38400'
```

如果串口终端打开时需要输入密码：
- 需要将`index = child.expect(['ubuntu'])` 中的`ubuntu`改为`password`或者当前用户的用户名
- 再将`child.send('canaan\n')` 中的`canaan`替换为需要的密码。

```py
index = child.expect(['ubuntu']) # `ubuntu`改为`password`或者当前用户的用户名
if index == 0:
    child.send('canaan\n') # `canaan`替换为需要的密码
else :
    child.send('\n')
```

### 7.

然后和正常下载程序步骤相同，用 `gdb` 工具加载 `fw_payload.elf` 到 `FPGA` 中。

### 8.

等待加载完成，直到系统开始正常运行(可以正常使用msh)，可以先手动打开串口终端检查是否开始正常运行，再关闭串口终端。

然后将userapps/testcases/pse51_test/qemu中的自动测试脚本  
qemubox.py和run_pthread_tc.py放在任意**可以打开串口终端**的同一目录即可。

再运行自动化测试脚本`run_pthread_tc.py`，该脚本就会自动开启串口终端并监听，自动向串口发送测试指令并检查测试结果。  
使用的测试脚本必须与测试用例对应，否则不能正常进行测试。

```bash
python3 run_pthread_tc.py

# 自动测试脚本运行时，会输出与下方类似的日志
/bin/pthread_mutex_unlock/3-1...Test PASSED
/bin/pthread_mutex_unlock/1-1...Test PASSED
/bin/pthread_key_create/2-1...Test PASSED
/bin/pthread_key_create/3-1...Test PASSED
/bin/pthread_key_create/1-2...Test PASSED
/bin/pthread_key_create/1-1...Test PASSED

# 在结束时会输出测试结果
Total Testcases: 50
         Passed: 50
    Pass Rate: 1.0
```
