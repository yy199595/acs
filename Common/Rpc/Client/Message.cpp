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


    bool Head::Get(const std::string &key, int &value) const
    {
        auto iter = this->find(key);
        if(iter == this->end())
        {
            return false;
        }
        value = std::stoi(iter->second);
        return true;
    }

    bool Head::Get(const std::string &key, long long &value) const
    {
        auto iter = this->find(key);
        if(iter == this->end())
        {
            return false;
        }
        value = std::stoll(iter->second);
        return true;
    }

    bool Head::Has(const std::string &key) const
    {
        auto iter = this->find(key);
        return iter != this->end();
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

    bool Head::Get(const std::string& key, std::string& value) const
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
        this->clear();
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
                return -1;
            }
            key = line.substr(0, pos);
            value = line.substr(pos + 1);
            this->emplace(key, value);
            line.clear();
        }
        return len;
    }

    size_t Head::GetLength()
    {
        size_t len = 0;
        auto iter = this->begin();
        for (; iter != this->end(); iter++)
        {
            len += (iter->first.size() + 1);
            len += (iter->second.size() + 1);
        }
        return len + 1;
    }

    bool Head::Serialize(std::ostream& os)
    {
        auto iter = this->begin();
        for(; iter != this->end(); iter++)
        {
            os.write(iter->first.c_str(), iter->first.size()) << '=';
            os.write(iter->second.c_str(), iter->second.size()) << '\n';
        }
        os << '\n';
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
        this->mBody.clear();
        char buffer[128] = {0};
        size_t len = this->mHead.Parse(os);
        if (len == -1)
        {
            return false;
        }
        this->mLen -= len;
        if (this->mLen > 0)
        {
            this->mBody.reserve(this->mLen);
        }
        while (this->mLen > 0)
        {
            size_t len = Helper::Math::Min(
                this->mLen, (int) sizeof(buffer));
            size_t count = os.readsome(buffer, len);
            if (count > 0)
            {
                this->mLen -= count;
                this->mBody.append(buffer, count);
            }
        }
        return true;
    }

    XCode Data::GetCode(XCode code) const
    {
        int value = (int)code;
        if(this->mHead.Get("code", value))
        {
            return (XCode)value;
        }
        return code;
    }

    bool Data::GetMethod(std::string &service, std::string &method) const
    {
        std::string value;
        if(!this->mHead.Get("func", value))
        {
            return false;
        }
        size_t pos = value.find('.');
        if(pos == std::string::npos)
        {
            return false;
        }
        service = value.substr(0, pos);
        method = value.substr(pos + 1);
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

        this->mHead.Serialize(os);
        os.write(this->mBody.c_str(), this->mBody.size());
        return 0;
    }
}