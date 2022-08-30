//
// Created by zmhy0073 on 2022/8/25.
//

#ifndef APP_MYSQLMESSAGE_H
#define APP_MYSQLMESSAGE_H

#include "Network/Proto/ProtoMessage.h"

namespace Tcp
{
    class MysqlMessage : public ProtoMessage
    {
    public:
        MysqlMessage(int index);
        int Serailize(std::ostream &os) final;
    public:
        void Add(char value);
        void Add(unsigned int value);
        void Add(const char * value);
        void Add(const char * value, int size);
    private:
        int mIndex;
        std::string mBuffer;
    };

    class MysqlAuthMessage : public ProtoMessage
    {
    public:
        int Serailize(std::ostream &os) final { return 0; }
    };
}


#endif //APP_MYSQLMESSAGE_H
