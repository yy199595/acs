//
// Created by zmhy0073 on 2022/8/25.
//

#include "MysqlMessage.h"

namespace Tcp
{
    MysqlMessage::MysqlMessage(int index)
    {
        this->mIndex = index;
    }
    int MysqlMessage::Serailize(std::ostream &os)
    {
        union {
            unsigned int num;
            char buff[sizeof(unsigned int)];
        } buffer;
        buffer.num = this->mBuffer.size();
        std::cout << this->mBuffer << std::endl;
        os.write(buffer.buff, 3);
        os.write((char *)&this->mIndex, 1);
        os.write(this->mBuffer.c_str(), this->mBuffer.size());
        return 0;
    }

    void MysqlMessage::Add(unsigned int value)
    {
        union {
            unsigned int num;
            char buff[sizeof(unsigned int)];
        } buffer;
        buffer.num = value;
        this->mBuffer.append(buffer.buff, sizeof(int));
    }

    void MysqlMessage::Add(const char *value)
    {
        this->mBuffer.append(value);
        this->mBuffer.append("\0");
    }

    void MysqlMessage::Add(const char *value, int size)
    {
        this->mBuffer.append(value, size);
    }

    void MysqlMessage::Add(char value)
    {
        this->mBuffer += value;
    }
}