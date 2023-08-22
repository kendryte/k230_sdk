# 1.简介
此代码可以实现对单个单词的续写功能（仅限英文）

# 2.应用使用说明
（仅限小核运行，大核不能执行）
## 2.1 使用帮助

```
"Usage: " << llama_run << "<checkpoint_file> [temperature] [steps] [word]"
各参数释义如下：
checkpoint_file  模型路径
temperature      激活值
steps                 生成单词数
word                 需续写单词（一个单词）

#推理示例：（llama_build.sh）
./llama_run llama.bin 0.9 100 love

单独执行步骤：
    1. 进入sdk构建的容器存放run.cc的目录下 执行 /mnt/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin/riscv64-unknown-linux-gnu-g++ -O3 llama_run.cc -o llama_run -lm 编译llama_run.cc 文件。
    2. 将llama_run执行文件导入到k230板小核上 ，并且在同级目录导入 ai_poc/utils下的llama.bin与tokenizer.bin
    3. 进入llama_run执行文件所在目录 即可执行
 ```


