#include "MD5.h"      
#include "../include/csapp.h"
#include <string>
using namespace std;
class Md5 { /* 封装一下c的MD5 */
    static const int MD5_SIZE = 16;
    uint8_t          bytes[MD5_SIZE]; /* md5字节数组 */
public:
    void constructor(const char* _str, int _len) { /* 构造 */
        md5((const uint8_t*)_str, _len, bytes);
    }

    Md5& copy_constructor(const Md5& obj) { /* 复制构造 */
        if (&obj == this) return *this;
        memcpy(bytes, obj.c_str(), sizeof(bytes));
        return *this;
    }

    /*-----begin---------------------------------
    构造，运算符，析构等等*/

    Md5(const char* _str, int _len) { /* 构造函数1 */
        constructor(_str, _len);
    }

    Md5(string _str = "") { /* 构造函数2，顺便替代无参构造 */
        constructor(_str.c_str(), _str.size());
    }

    Md5(const Md5& obj) { /* 复制构造函数 */
        copy_constructor(obj);
    }

    Md5& operator=(const Md5& obj) { /* 赋值函数 */
        return copy_constructor(obj);
    }

    Md5(const uint8_t* obj) { /* 赋值函数 */
        memcpy(bytes, obj, sizeof(bytes));
    }

    bool operator==(const Md5& obj) { return !md5cmp_base(bytes, obj.c_str()); }

    bool operator<(const Md5& obj) { /* 重载<是考虑到可能会用STL的排序算法 */
        return md5cmp_base(bytes, obj.c_str()) < 0;
    }

    /*构造，运算符，析构等等
    -------end--------------------------------*/

    /*-----begin---------------------------------
    其他函数*/

    uint8_t* c_str() const { /* 返回md5字节数组 */
        return (uint8_t*)bytes;
    }

    operator string() { /* 打印MD5的值 */
        char buf[MD5_SIZE * 2 + 1];
        for (int i = 0; i < MD5_SIZE; ++i)
            sprintf(buf + i * 2, "%2.2x", bytes[i]);
        buf[MD5_SIZE * 2] = 0;
        return string(buf);
    }

    /* Compare md5 bytes1 and bytes2 */
    int md5cmp_base(uint8_t* _d1, uint8_t* _d2) {
        int8_t i = -1;
        while (++i < 16) {
            if (_d1[i] != _d2[i]) {
                return _d1[i] < _d2[i] ? -1 : 1;
            }
        }
        return 0;
    }

    /*其他函数
    -------end--------------------------------*/
};
