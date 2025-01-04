#ifndef APP_MD5_H
#define APP_MD5_H

#include <string>
#include <fstream>

/* Type define */
typedef unsigned long ulong;

using std::string;
using std::ifstream;

class MD5
{
public:
    MD5();

    MD5(const void *input, size_t length);

    explicit MD5(const string &str);

    explicit MD5(ifstream &in);

    void update(const void *input, size_t length);

    void update(const string &str);

    void update(ifstream &in);

    const unsigned char*digest();

    string toString();

    void reset();

	void encode(const ulong *input, unsigned char*output, size_t length);

	void decode(const unsigned char*input, ulong *output, size_t length);

private:
    void update(const unsigned char*input, size_t length);

    void final();

    void transform(const unsigned char block[64]);



    string bytesToHexString(const unsigned char*input, size_t length);

    /* class uncopyable */
    MD5(const MD5 &);

private:
    ulong _state[4];    /* state (ABCD) */
    ulong _count[2];    /* number of bits, modulo 2^64 (low-order word first) */
    unsigned char _buffer[64];    /* input buffer */
    unsigned char _digest[16];    /* message digest */
    bool _finished;        /* calculate finished ? */

    static const unsigned char PADDING[64];    /* padding for calculate */
    static const char HEX[16];
    static const size_t BUFFER_SIZE = 1024;
};

namespace help
{
    namespace md5
    {
        extern std::string GetMd5(ifstream &in);
		extern std::string GetMd5(const std::string & str);
        extern std::string GetMd5(const char * str, size_t size);
    }
}



#endif/*APP_MD5_H*/