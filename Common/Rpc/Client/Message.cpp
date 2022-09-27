//
// Created by zmhy0073 on 2022/9/27.
//

#include"Message.h"
#include"Math/MathHelper.h"

namespace Rpc
{
    bool Head::Add(const std::string& key, int value)
    {
        auto iter = this->find(key);
        if(iter != this->end())
        {
            return false;
        }
        this->emplace(key, std::to_string(value));
        return true;
    }

    bool Head::Get(const std::string &key, int &value)
    {
        auto iter = this->find(key);
        if(iter == this->end())
        {
            return false;
        }
        value = std::stoi(iter->second);
        return true;
    }

    bool Head::Get(const std::string &key, long long &value)
    {
        auto iter = this->find(key);
        if(iter == this->end())
        {
            return false;
        }
        value = std::stoll(iter->second);
        return true;
    }

    bool Head::Remove(const std::string &key)
    {
        auto iter = this->find(key);
        if(iter == this->end())
        {
            return false;
        }
        this->erase(iter);
        return true;
    }

    bool Head::Get(const std::string& key, std::string& value)
    {
        auto iter = this->find(key);
        if(iter == this->end())
        {
            return false;
        }
        value = iter->second;
        return true;
    }

    size_t Head::Parse(std::istream& os)
    {
        size_t len = 0;
        std::string line, key, value;
        while (std::getline(os, line))
        {
            len += (line.size() + 1);
            if (line.empty())
            {
                break;
            }
            size_t pos = line.find('=');
            if (pos == std::string::npos)
            {
                return false;
            }
            key = line.substr(0, pos);
            value = line.substr(pos + 1);
            this->emplace(key, value);
            line.clear();
        }
        return len;
    }

    bool Head::Serialize(std::ostream& os)
    {
        auto iter = this->begin();
        for(; iter != this->end(); iter++)
        {
            const std::string & key = iter->first;
            const std::string & val = iter->second;
            os << key << '=' << val << "\n";
        }
        os << "\n";
        return true;
    }

    bool Head::Add(const std::string& key, long long value)
    {
        auto iter = this->find(key);
        if(iter != this->end())
        {
            return false;
        }
        this->emplace(key, std::to_string(value));
        return true;
    }

    bool Head::Add(const std::string& key, const std::string& value)
    {
        auto iter = this->find(key);
        if(iter != this->end())
        {
            return false;
        }
        this->emplace(key, value);
        return true;
    }

    int Data::ParseLen(std::istream &is)
    {
        union {
            int len;
            char buf[sizeof(int)];
        } buffer;
        is.readsome(buffer.buf, sizeof(int));

        this->mLen = buffer.len;
        this->mType = (Tcp::Type)is.get();
        this->mProto = (Tcp::Porto)is.get();
        return this->mLen;
    }

    bool Data::Parse(std::istream &os, size_t size)
    {
        if (size != this->mLen)
        {
            return false;
        }
        char buffer[128] = { 0 };
        this->mLen -= this->mHead.Parse(os);
        if(this->mLen > 0)
        {
            this->mBody.reserve(this->mLen);
        }
        while(this->mLen > 0)
        {
            size_t len = Helper::Math::Min(
                this->mLen, (int)sizeof(buffer));
            if(os.readsome(buffer, len) != len)
            {
                return false;
            }
            this->mLen -= len;
            this->mBody.append(buffer, len);
        }
        return true;
    }

    int Data::Serailize(std::ostream &os)
    {
        union
        {
            int len = 0;
            char buf[sizeof(int)];
        } buffer;
        buffer.len = this->mHead.GetLength() + this->mBody.size();
        os.write(buffer.buf, sizeof(int));
        os << (char) this->mType << (char) this->mProto;
        os.write(this->mBody.c_str(), this->mBody.size());
        return 0;
    }
}