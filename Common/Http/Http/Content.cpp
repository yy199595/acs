//
// Created by yjz on 2022/10/27.
//

#include "Content.h"

namespace Http
{

    JsonContent::JsonContent(const std::string &json)
        : mJson(json) { }

    JsonContent::JsonContent(const char *json, size_t len)
        : mJson(json, len) { }

    bool JsonContent::OnRead(std::istream &buffer)
    {
        char buff[128] = { 0};
        size_t size = buffer.readsome(buff, sizeof(buff));
        if(size > 0)
        {
            this->mJson.append(buff, size);
        }
        return true;
    }

    int JsonContent::OnWrite(std::ostream &buffer)
    {
        const char * json = this->mJson.c_str();
        const size_t length = this->mJson.size();
        buffer.write(json, length);
        return 0;
    }
}