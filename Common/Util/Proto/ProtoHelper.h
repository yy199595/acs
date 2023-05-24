//
// Created by zmhy0073 on 2022/8/29.
//

#ifndef APP_PROTOHELPER_H
#define APP_PROTOHELPER_H

#include"Proto/Include/Message.h"
namespace Helper
{
    namespace Protocol
    {
        bool GetJson(const pb::Message & message, std::string * json);
        bool FromJson(pb::Message * message, const std::string & json);
        bool GetMember(const std::string & key, const pb::Message & message, std::string & value);
    }
};


#endif //APP_PROTOHELPER_H
