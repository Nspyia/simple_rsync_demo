#include <iostream>

#include "rsync.hh"
using namespace std;

char *PORT = "9997";

/* Split file to chunks, return vector<Chunk> */
vector<Chunk> file_to_chunk(char *filepath) {
    int         fd = Open(filepath, O_RDWR, 0);
    struct stat fs;
    Fstat(fd, &fs);
    int n_chunk = fs.st_size / CHUNK_SIZE; /* compute n_chunk */
    cout << n_chunk << endl;
    vector<Chunk> cks(n_chunk);
    char          buf[CHUNK_SIZE];
    for (int i = 0; i < n_chunk; ++i) {  // i : chunk_number
        memset(buf, 0, sizeof(buf));
        Read(fd, buf, CHUNK_SIZE);
        cks[i].init(buf, i);
    }
    return cks;
}  //

/* 写入n个Chunk 到fd */
void send_chunk(int fd, const vector<Chunk> &cks, uint n) {
    Rio_writen(fd, &n, sizeof(n)); /* 先传输cks的个数, 约定为4B的int */
    Rio_writen(fd, (void *)cks.data(),
               sizeof(Chunk) * n); /* 然后将chunk转换成字节流，传输 */
}

/* 接受来自sfd的patches,  old_file_path：老文件, new_file_path:新文件temp，
 * 后期可以删除老文件，改名新文件 */
void recv_patches(int sfd, char *old_file_path, char *new_file_path) {
    int         old_fd = Open(old_file_path, O_RDONLY, 0);
    int         new_fd = Open(new_file_path, O_WRONLY | O_CREAT | O_TRUNC, 777);
    struct stat old_stat;
    Fstat(old_fd, &old_stat);

    /* step 3.1 先接收文件大小 */
    off_t new_file_size;
    Rio_readn(sfd, &new_file_size, sizeof(new_file_size));
    cout << "来自于服务端的新文件大小:" << new_file_size / 1024.0 / 1024 << "MB"
         << endl;

    off_t i = 0;
    char  buf[CHUNK_SIZE];
    while (1) {
        int index;
        if (!Rio_readn(sfd, &index, sizeof(index))) break;
        if (index >= 0) { /* 此时index代表块号 */
            Lseek(old_fd, index * CHUNK_SIZE, SEEK_SET);
            Rio_readn(old_fd, buf, CHUNK_SIZE);
            Rio_writen(new_fd, buf, CHUNK_SIZE);
            i += CHUNK_SIZE;
        } else { /* 此时|index|代表patch的大小 */
            int nn = -index;
            while (nn > 0) {
                int n = min(nn, CHUNK_SIZE);
                Rio_readn(sfd, buf, n);
                Rio_writen(new_fd, buf, n);
                nn -= n;
            }
        }
    }
}

/* usage : <服务端ip> <本地filepath> <服务端filepath>*/
int main(int argc, char const *argv[]) {
    char *server_ip_str = "127.0.0.1";
    char *old_file_path = "file";
    char *new_file_path = "file_tmp";
    /*
        if (argc == 1) {
            server_ip_str = (char *)argv[0];
        } else if (argc == 2) {
            local_file_path  = (char *)argv[0];
            server_file_path = (char *)argv[1];
        } else if (argc == 3) {
            server_ip_str    = (char *)argv[0];
            local_file_path  = (char *)argv[1];
            server_file_path = (char *)argv[2];
        }
     */
    /* 打开服务端fd */
    int sfd = Open_clientfd(server_ip_str, PORT);
    /*
        客户端流程：
        1.文件分块获取chunks  （每个chunk带有20B的信息）
        2.发送chunks给服务端，（服务端处理chunks）
        3.接受来自服务端的patches, 重组文件
     */
    /* 先接受文件的大小吧 */

    /* ----step 1---- */
    vector<Chunk> cks = file_to_chunk(old_file_path);
    printf("文件分块...完成, 一共%d个块\n", cks.size());

    /* ----step 2---- */
    send_chunk(sfd, cks, cks.size());
    printf("发送chunk...完成\n");

    /* ----step 3---- */
    recv_patches(sfd, old_file_path, new_file_path);
    struct stat statbuff;
    Stat(new_file_path, &statbuff);
    printf("文件重组完成，新文件:%s, 大小:%fMB\n", new_file_path,
           statbuff.st_size / 1024.0 / 1024);
    return 0;
}