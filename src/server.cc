#include <iostream>

#include "rsync.hh"
using namespace std;

char* PORT = "9997";

vector<Chunk> recv_chunk(int fd) {
    uint n;
    Rio_readn(fd, &n, sizeof(n)); /* 1.先接受块的个数 */
    printf("接收到%d个chunk, 一共接受%d个字节\n", n, n * 24);
    vector<Chunk> cks;
    char          buf[sizeof(Chunk)];
    while (n--) { /* 读取n个chunk */
        Rio_readn(fd, buf, sizeof(Chunk));
        cks.push_back(*(Chunk*)buf); /* 每读一个chunk，加入到cks */
    }
    return cks;
}  //

/*     if index>=0:  index  represented a chunk number,
  else if index< 0: |index| represented a patch size */
void send_patch(int clientfd, int index, char* data) {
    Rio_writen(clientfd, &index, sizeof(index));
    cnt_send_byte += 4;
    if (index < 0) {
        Rio_writen(clientfd, data, -index);
        cnt_send_byte += -index;
    }
}

/* 输入接收到的Chunk，本地文件 , 发送pathes到clientfd */
void rolling_compare(vector<Chunk>& dest, const char* filename, int clientfd) {
    unordered_map<int, int> mp; /* 查找hash32对应的index */
    int         ffd = Open(filename, O_RDONLY, 0); /* 文件描述符 */
    struct stat fstat;                             /* 文件信息 */
    char*       file_begin_ptr;                    /* 文件起始地址 */

    for (Chunk& ck : dest) mp[ck.hash32] = ck.index;
    Fstat(ffd, &fstat);
    /* 先发送新文件的大小过去吧 */
    Rio_writen(clientfd, &fstat.st_size, sizeof(fstat.st_size));
    printf("file size: %.2fMB\n", fstat.st_size / 1024.0 / 1024);
    file_begin_ptr =
        (char*)Mmap(NULL, fstat.st_size, PROT_READ, MAP_SHARED, ffd, 0);

    off_t i = 0; /* 文件偏移量 */
    while (i + CHUNK_SIZE < fstat.st_size) {
        off_t  last = i;
        Hash32 hs(file_begin_ptr + i, CHUNK_SIZE);
        i += CHUNK_SIZE;
        while (1) {
            /* 判断是否匹配到分块： 先比较hash32， 再比较md5，
             * 如果hash32不同则不需要计算md5
             */
            if (mp.count((int)hs) && Md5(file_begin_ptr + i - CHUNK_SIZE,
                                         CHUNK_SIZE) == dest[mp[(int)hs]].md5) {
                if (i - CHUNK_SIZE - last > 0)
                    send_patch(clientfd, -(i - CHUNK_SIZE - last),
                               &file_begin_ptr[last]); /* 发送patch */
                send_patch(clientfd, dest[mp[(int)hs]].index,
                           nullptr); /* 发送chunk_number */
                break;
            }
            if (i == fstat.st_size) { /* 指针到末尾 */
                send_patch(clientfd, -(i - last), &file_begin_ptr[last]);
                break;
            }
            hs.push(file_begin_ptr[i]);
            hs.pop(file_begin_ptr[i - CHUNK_SIZE]);
            ++i;
        };
    }
    if (i < fstat.st_size) /* 补充未发送的 */
        send_patch(clientfd, -(fstat.st_size - i), &file_begin_ptr[i]);
}

int main(int argc, char const* argv[]) {
    char* filename = "file_new";
    int   listenfd = Open_listenfd(PORT);
    int   connfd   = Accept(listenfd, (struct sockaddr*)0, 0);

    /*
        服务端流程：
        1.接受服务端的chunks(与客户端step2对应)
        2.rolling匹配出相同分块，将patches发送给服务端(与客户端step3对应)

     */
    /* step 1 */
    vector<Chunk> cks = recv_chunk(connfd);
    /* step 2 */
    rolling_compare(cks, filename, connfd);

    printf("一共发送了%d个字节\n", cnt_send_byte);
}