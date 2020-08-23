# Demo运行：

1. 准备file(老文件)和file_new(待同步的新文件)到src里面
2. `cd ./src`
3. BASH窗口1: `g++ server.cc -o server -w -lpthread`
4. BASH窗口2: `g++ client.cc -o client -w -lpthread`
5. BASH窗口1: `./server`
6. BASH窗口2: `./client`

CSDN : https://blog.csdn.net/yinxiaobao97/article/details/108181980
