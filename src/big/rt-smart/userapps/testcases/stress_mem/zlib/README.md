# zlib for RT-Thread 通用数据压缩库 

## 1、介绍

Zlib 是一款免费的、通用的、合法的、不受任何限制的无损数据压缩库，使用 zlib 库可以轻松实现 gzip 格式压缩。该库提供多种不同压缩方式，如最快速度和最大压缩比压缩，各种配置选项可以参考 `zconf.h` 文件。

本软件包是 RT-thread 针对 [zlib](https://github.com/madler/zlib) 官方 1.2.11 版本的移植， 更多信息请参阅[官方文档](http://www.zlib.net/) 。

## 2、获取方式

```
RT-Thread online package -> 
     miscellaneous package -> 
         [*] zlib: general purpose data compression library.  --->
```

## 3、示例介绍

### 3.1 运行示例

该示例为一个简单的文件压缩和解压的例程，需要依赖文件系统，用到的 `zlib_test` 命令有两个 `-c` 和  `-d` 两个参数。使用 `-c` 参数将会压缩一个文件到另一个文件，`-d` 命令解压一个压缩文件到另一个文件。 

使用方式：

压缩命令： `zlib_test -c test test_com`  

注意：压缩的过程需要大约 270 k 内存，否则由于内存不够而压缩失败。

```c
msh />zlib_test -c test test_com
msh />ls
Directory /:
test                145          //压缩前文件大小为 145 字节 
test_com            77           //压缩后文件大小为 77 字节
```
解压命令： `zlib_test -d test_com test_decom  `

```c
msh />zlib_test -d test_com test_decom
msh />ls
Directory /:
test                145          
test_com            77          //压缩后文件大小为 77 字节
test_decom          145         //解压后文件大小为 145 字节
```

## 4、常见问题

注意，压缩文件需要大约 270 k 内存，如果内存不够会出现压缩失败的情况。

解压文件大约需要 18 k 内存占用。

## 5、维护
维护：[Meco Man](https://github.com/mysterywolf)

主页：<https://github.com/RT-Thread-packages/zlib>