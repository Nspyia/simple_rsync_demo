#include "../include/csapp.h"
class Hash32 { /* 设计一个的32位hash */
    uint code = (1 << 31);

public:
    Hash32(const char* _str, int len) {
        for (int i = 0; i < len; ++i) push(_str[i]);
    }

    Hash32(const uint& _code = (1 << 31)) : code(_code) {}

    /* F(c)将8b散成一个32b，防止加减操作陷入一个正态分布而使得Hash不够分散*/
    uint F(const char& c) { /* 将8bit 散列到32 bit*/
        const uint base = 8668309;
        return c * base;
    }

    operator uint() { return code; }

    inline uint push(const char& byte) { return code += F(byte) << 5; }
    inline uint push(const char* _str, int len) {
        for (int i = 0; i < len; ++i) push(_str[i]);
    }
    inline uint pop(const char& byte) { return code -= F(byte) << 5; }
};
