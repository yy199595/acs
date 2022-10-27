//
// Created by yjz on 2022/10/27.
//

#include "HttpResponse.h"

namespace Http
{
    Response::Response()
    {
        this->mVersion = HttpVersion;
        this->mParseState = State::None;
    }
    int Response::OnRead(std::istream &buffer)
    {
        if(this->mParseState == State::None)
        {
            buffer >> this->mVersion;
            buffer >> this->mCode;
            buffer >> this->mError;
            this->mParseState = State::Head;
        }
        if(this->mParseState == State::Head)
        {
            if(this->mHead.OnRead(buffer) != 0)
            {
                return -1;
            }
            this->mParseState = State::Body;
        }
        if(this->mParseState == State::Body)
        {
            char buff[128] = {0};
            size_t size = buffer.readsome(buff, sizeof(buff));
            while(size > 0)
            {
                this->mContent.append(buff, size);
                size = buffer.readsome(buff, sizeof(buff));
            }
            return -1;
        }
        return -1;
    }

    int Response::OnWrite(std::ostream &buffer)
    {
        buffer << this->mVersion << ' ' << (int)this->mCode << ' '
                << HttpStatusToString((HttpStatus)this->mCode) << "\r\n";
        this->mHead.OnWrite(buffer);

        buffer << "\r\n";
        buffer.write(this->mContent.c_str(), this->mContent.size());
        return 0;
    }

    void Response::Json(HttpStatus code, const std::string &json)
    {
       this->Json(code, json.c_str(), json.size());
    }

    void Response::Json(HttpStatus code, const char *str, size_t len)
    {
        this->mCode = (int)code;
        this->mContent.append(str, len);
        this->mHead.Add("content-type", "application/json");
    }
}