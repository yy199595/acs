//
// Created by zmhy0073 on 2022/9/27.
//

#include"Message.h"
#include"Util/Math/MathHelper.h"
#include"Proto/Include/Message.h"
#include"Util/Json/JsonWriter.h"
namespace Msg
{
    bool Head::Add(const std::string& key, int value)
    {
        auto iter = this->mItems.find(key);
        if(iter != this->mItems.end())
        {
            return false;
        }
        this->mItems.emplace(key, std::to_string(value));
        return true;
    }

    bool Head::Get(std::vector<std::string> &keys) const
    {
        auto iter = this->mItems.begin();
        for(; iter != this->mItems.end(); iter++)
        {
            keys.emplace_back(iter->first);
        }
        return !keys.empty();
    }


    bool Head::Get(const std::string &key, int &value) const
    {
        auto iter = this->mItems.find(key);
        if(iter == this->mItems.end())
        {
            return false;
        }
        const std::string& str = iter->second;
        if (!str.empty() && std::all_of(str.begin(), str.end(), ::isdigit))
        {
            value = std::stoi(str);
            return true;
        }
        return false;
    }

    bool Head::Get(const std::string &key, long long &value) const
    {
        auto iter = this->mItems.find(key);
        if(iter == this->mItems.end())
        {
            return false;
        }
        const std::string& str = iter->second;
        if (!str.empty() && std::all_of(str.begin(), str.end(), ::isdigit))
        {
            value = std::stoll(str);
            return true;
        }
        return false;
    }

    bool Head::Has(const std::string &key) const
    {
        auto iter = this->mItems.find(key);
        return iter != this->mItems.end();
    }

    bool Head::Remove(const std::string &key)
    {
        auto iter = this->mItems.find(key);
        if(iter == this->mItems.end())
        {
            return false;
        }
        this->mItems.erase(iter);
        return true;
    }

    bool Head::Get(const std::string& key, std::string& value) const
    {
        auto iter = this->mItems.find(key);
        if(iter == this->mItems.end())
        {
            return false;
        }
        value = iter->second;
        return true;
    }

    const std::string& Head::GetStr(const std::string& key) const
    {
        static std::string empty;
        auto iter = this->mItems.find(key);
        return iter != this->mItems.end() ? iter->second : empty;
    }

    size_t Head::Parse(std::istream& os)
    {
        size_t len = 0;
		this->mItems.clear();
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
            this->mItems.emplace(key, value);
            line.clear();
        }
        return len;
    }

    size_t Head::GetLength() const
    {
        size_t len = 0;
        auto iter = this->mItems.begin();
        for (; iter != this->mItems.end(); iter++)
        {
            len += (iter->first.size() + 1);
            len += (iter->second.size() + 1);
        }
        return len + 1;
    }

    bool Head::Serialize(std::ostream& os) const
    {
        auto iter = this->mItems.begin();
        for(; iter != this->mItems.end(); iter++)
        {
            os.write(iter->first.c_str(), iter->first.size()) << '=';
            os.write(iter->second.c_str(), iter->second.size()) << '\n';
        }
        os << '\n';
        return true;
    }

    bool Head::Add(const std::string& key, long long value)
    {
        auto iter = this->mItems.find(key);
        if(iter != this->mItems.end())
        {
            return false;
        }
        this->mItems.emplace(key, std::to_string(value));
        return true;
    }

    bool Head::Add(const std::string& key, const std::string& value)
    {
        auto iter = this->mItems.find(key);
        if(iter != this->mItems.end())
        {
            return false;
        }
        this->mItems.emplace(key, value);
        return true;
    }

    bool Packet::ParseLen(std::istream &is, int & len)
    {
        union
        {
            int len;
            char buf[sizeof(int)];
        } buffer{};
        if (is.readsome(buffer.buf, sizeof(int)) != sizeof(int))
        {
            return false;
        }

        this->mType = is.get();
        this->mProto = is.get();
        len = buffer.len;
        this->mLen = buffer.len;
        return true;
    }

    bool Packet::Parse(const std::string& address, std::istream &os, size_t size)
    {
        if (size != this->mLen)
        {
            return false;
        }
        this->mBody.clear();
        char buffer[128] = {0};
        this->mFrom = address;
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
            len = Helper::Math::Min(
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

    int Packet::GetCode(int code) const
    {
        int value = (int)code;
        if(this->mHead.Get("code", value))
        {
            return value;
        }
        return code;
    }

    bool Packet::GetMethod(std::string &service, std::string &method) const
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

    int Packet::Serialize(std::ostream &os)
	{
		char buff[sizeof(this->mLen)] = { };
		this->mLen = this->mHead.GetLength() + this->mBody.size();

		memcpy(buff, &this->mLen, sizeof(this->mLen));
		os.write(buff, sizeof(this->mLen));
		os << (char)this->mType << (char)this->mProto;

		this->mHead.Serialize(os);
		os.write(this->mBody.c_str(), this->mBody.size());
		return 0;
	}

    void Packet::SetContent(const std::string & content)
    {
        this->mBody = content;
    }

    std::shared_ptr<Packet> Packet::Clone() const
    {
        std::shared_ptr<Packet> message = std::make_shared<Packet>();
        {
            message->SetType(this->mType);
            message->SetProto(this->mProto);

            message->mBody = this->mBody;
            message->mHead = this->mHead;
        }
        return message;
    }

    bool Packet::ParseMessage(pb::Message* message)
	{
		switch (this->mProto)
		{
			case Msg::Porto::Protobuf:
				if (message->ParseFromString(this->mBody))
				{
					this->mBody.clear();
					return true;
				}
				return false;
			case Msg::Porto::Json:
				if(pb_json::JsonStringToMessage(this->mBody, message).ok())
				{
					this->mBody.clear();
					return true;
				}
				return false;
			default:
			LOG_ERROR("unknown message proto : " << this->mProto);
				return false;
		}
	}

	std::string Packet::ToString()
	{
		Json::Writer jsonWriter;
		jsonWriter.BeginObject("head");
		for(const auto & item : this->mHead)
		{
			jsonWriter.Add(item.first).Add(item.second);
		}
		jsonWriter.EndObject();
		switch(this->mProto)
		{
			case Msg::Porto::String:
				jsonWriter.Add("data").Add(this->mBody);
				break;
			case Msg::Porto::Json:
			{
				rapidjson::Document document;
				document.Parse(this->mBody.c_str(), this->mBody.size());
				jsonWriter.Add("data").Add(document);
				break;
			}
			case Msg::Porto::Protobuf:
			{
				break;
			}
		}
		return jsonWriter.JsonString();
	}


    bool Packet::WriteMessage(const pb::Message* message)
	{
		if (message == nullptr)
		{
			return true;
		}
		this->mBody.clear();
		switch (this->mProto)
		{
			case Msg::Porto::String:
				break;
			case Msg::Porto::Protobuf:
				return message->SerializeToString(&mBody);
			case Msg::Porto::Json:
				return pb_json::MessageToJsonString(*message, &mBody).ok();
		}
		CONSOLE_LOG_FATAL("proto error write error");
		return false;
	}
}