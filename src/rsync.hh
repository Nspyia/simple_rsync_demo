#include <bits/stdc++.h>

#include "../include/csapp.h"
#include "Hash32.hh"
#include "MD5.hh"
using namespace std;

#define CHUNK_SIZE 4096
struct Chunk {     /* Chunk内容 */
    Md5    md5;    /* MD5 16B */
    int    index;  /* Chunk number 4B*/
    Hash32 hash32; /* Hash32 4B*/
    void   init(const char* buf, int number = 0) {
        index = number;
        hash32.push(buf, CHUNK_SIZE);
        md5.constructor(buf, CHUNK_SIZE);
    }
};  //

int cnt_send_byte = 0; /* 一个计数器，统计发送字节数 */


/* Read fd2 and wirte it to fd1, using rsync*/
