//
// Created by zmhy0073 on 2022/9/27.
//

#include"Message.h"
#include"Proto/Include/Message.h"
#include"Yyjson/Document/Document.h"
#include"Util/Tools/String.h"


#if RPC_PACKET_COUNTER == 1
#include "Util/Tools/TimeHelper.h"
#endif
namespace rpc
{
#if RPC_PACKET_COUNTER == 1
	std::mutex Packet::mutex;
	std::unordered_set<rpc::Packet *> Packet::NewMessageSet;
#endif

    bool Head::GetKeys(std::vector<std::string> &keys) const
    {
        auto iter = this->mHeader.begin();
        for(; iter != this->mHeader.end(); iter++)
        {
            keys.emplace_back(iter->first);
        }
        return !keys.empty();
    }

    const std::string& Head::GetStr(const std::string& key) const
    {
        static std::string empty;
        auto iter = this->mHeader.find(key);
        return iter != this->mHeader.end() ? iter->second : empty;
    }

    int Head::OnRecvMessage(std::istream& os, size_t size)
    {
        size_t len = 0;
		this->mHeader.clear();
		std::string line, key, value;
        while (std::getline(os, line))
        {
            len += (line.size() + 1);
            if (line.empty())
            {
                break;
            }
			if(help::Str::Split(line, ':', key, value) != 0)
			{
				return -1;
			}
            this->mHeader.emplace(key, value);
            line.clear();
        }
        return (int)len;
    }

    size_t Head::GetLength() const
    {
        size_t len = 0;
        auto iter = this->mHeader.begin();
        for (; iter != this->mHeader.end(); iter++)
        {
            len += (iter->first.size() + 1);
            len += (iter->second.size() + 1);
        }
        return len + 1;
    }

    int Head::OnSendMessage(std::ostream& os)
    {
        auto iter = this->mHeader.begin();
        for(; iter != this->mHeader.end(); iter++)
        {
			const std::string & key = iter->first;
			const std::string & val = iter->second;
            os.write(key.c_str(), (int)key.size()) << ':';
            os.write(val.c_str(), (int)val.size()) << '\n';
        }
        os << '\n';
        return 0;
    }

	Packet::Packet()
	{
#if RPC_PACKET_COUNTER == 1
		std::lock_guard<std::mutex> lock(mutex);
		NewMessageSet.insert(this);
#endif
		this->mSockId = 0;
		this->mTimeout = 0;
		this->mBody.clear();
		this->mHead.Clear();
		this->mNet = rpc::Net::Tcp;
		memset(&this->mProtoHead, 0, sizeof(this->mProtoHead));
	}

	Packet::~Packet()
	{
#if RPC_PACKET_COUNTER == 1

		std::lock_guard<std::mutex> lock(mutex);
		auto iter = NewMessageSet.find(this);
		if(iter != NewMessageSet.end())
		{
			NewMessageSet.erase(iter);
		}
#endif
	}

#if RPC_PACKET_COUNTER == 1

	size_t Packet::PacketCount()
	{
		std::lock_guard<std::mutex> lock(mutex);
		for(const Packet * packet : NewMessageSet)
		{

		}
		return NewMessageSet.size();
	}
#endif

	void Packet::Init(const rpc::ProtoHead & protoHead)
	{
		this->mProtoHead = protoHead;
	}

	bool Packet::Decode(const char* message, int len)
	{
		if(len <= RPC_PACK_HEAD_LEN)
		{
			return false;
		}
		std::string data(message, len);
		memcpy(&mProtoHead, message, RPC_PACK_HEAD_LEN);

		len -= RPC_PACK_HEAD_LEN;
		if(len != this->mProtoHead.Len)
		{
			return false;
		}
		size_t count = 0;
		const char* line_start = message + RPC_PACK_HEAD_LEN;
		const char* line_end = nullptr;
		while (true)
		{
			line_end = strchr(line_start, '\n');

			if (line_end == nullptr) {
				break;
			}
			size_t line_length = line_end - line_start;

			if (line_length == 0) {
				count++;
				line_start++;
				break;
			}

			const char* colon = strchr(line_start, ':');
			if (colon != nullptr && colon < line_end)
			{
				size_t key_length = colon - line_start;
				size_t value_length = line_end - colon - 1;
				std::string key(line_start, key_length);
				std::string value(colon + 1, value_length);
				this->mHead.Add(key, value);
				count = count + key_length + value_length + 1;
			}
			count++;
			line_start = line_end + 1;  // 跳过 '\n'
		}
		int bodyCount = len - (int)count;
		if(bodyCount > 0)
		{
			this->mBody.assign(line_start, bodyCount);
		}
		return true;
	}

	bool Packet::EncodeToJson(std::string& json)
	{
		json::w::Document document;
		auto headObject = document.AddObject("head");
		{
			for(auto iter = this->mHead.Begin(); iter != this->mHead.End(); iter++)
			{
				headObject->Add(iter->first.c_str(), iter->second);
			}
		}
		if(this->mBody.empty())
		{
			return true;
		}
		document.Add("data", this->mBody);
		document.Add("_t", this->mProtoHead.Type);
		document.Add("_p", this->mProtoHead.Porto);
		return true;
	}

	bool Packet::DecodeFromJson(json::r::Value& document)
	{
		std::unique_ptr<json::r::Value> headValue;
		if(!document.Get("head", headValue))
		{
			return false;
		}
		std::vector<const char *> keys;
		if(!headValue->GetKeys(keys))
		{
			return false;
		}
		for(const char * key : keys)
		{
			std::string value;
			if(headValue->Get(key, value))
			{
				this->mHead.Add(key, value);
			}
		}
		document.Get("data", this->mBody);
		document.Get("_t", this->mProtoHead.Type);
		document.Get("_p", this->mProtoHead.Porto);
		return true;
	}

	bool Packet::DecodeFromJson(const char* message, int len)
	{
		json::r::Document document;
		if(!document.Decode(message, len))
		{
			return false;
		}
		return this->DecodeFromJson(document);
	}

    int Packet::OnRecvMessage(std::istream& os, size_t size)
    {
		thread_local static char buffer[128] = {0};
		if(size < this->mProtoHead.Len)
		{
			return tcp::ReadError;
		}

        int len = this->mHead.OnRecvMessage(os, size);
        if (len == -1)
        {
			return tcp::ReadError;
		}
		this->mBody.clear();
		this->mProtoHead.Len -= len;
		if (this->mProtoHead.Len == 0)
		{
			return tcp::ReadDone;
		}
		this->mBody.reserve(this->mProtoHead.Len);
        while (this->mProtoHead.Len > 0)
		{
			len = sizeof(buffer);
			if(this->mProtoHead.Len < len)
			{
				len = this->mProtoHead.Len;
			}
			int count = (int)os.readsome(buffer, len);
			if (count <= 0)
			{
				return tcp::ReadError;
			}
			this->mProtoHead.Len -= count;
			this->mBody.append(buffer, count);
		}
        return tcp::ReadDone;
    }

    int Packet::GetCode(int code) const
    {
        this->mHead.Get("code", code);
        return code;
    }

    int Packet::OnSendMessage(std::ostream &os)
	{
		this->mProtoHead.Len = this->mHead.GetLength() + this->mBody.size();
		tcp::Data::Write<rpc::ProtoHead>(os, this->mProtoHead);

		this->mHead.OnSendMessage(os);
		os.write(this->mBody.c_str(), this->mBody.size());
		return 0;
	}

    void Packet::SetContent(const std::string & content)
    {
        this->mBody = content;
    }

	void Packet::SetContent(char proto, const std::string& content)
	{
		this->mBody = content;
		this->mProtoHead.Porto = proto;
	}

    std::unique_ptr<Packet> Packet::Clone() const
    {
        std::unique_ptr<Packet> message = std::make_unique<Packet>();
        {
			message->SetNet(this->mNet);
			for(auto iter = this->mHead.Begin(); iter != this->mHead.End(); iter++)
			{
				message->mHead.Add(iter->first, iter->second);
			}
			message->mBody = this->mBody;
			message->mProtoHead = this->mProtoHead;
		}
        return message;
    }

    bool Packet::ParseMessage(pb::Message* message)
	{
		switch (this->mProtoHead.Porto)
		{
			case rpc::Porto::Protobuf:
				if (message->ParseFromString(this->mBody))
				{
					this->mBody.clear();
					return true;
				}
				return false;
			case rpc::Porto::Json:
				if(pb_json::JsonStringToMessage(this->mBody, message).ok())
				{
					this->mBody.clear();
					return true;
				}
				return false;
			default:
			//LOG_ERROR("unknown message proto : {}", this->mProtoHead.Porto);
				return false;
		}
	}

	bool Packet::ParseMessage(json::r::Document* message)
	{
		if(this->mProtoHead.Porto != rpc::Porto::Json)
		{
			return false;
		}
		if(!message->Decode(this->mBody))
		{
			return false;
		}
		this->mBody.clear();
		return true;
	}

	bool Packet::WriteMessage(json::w::Document* message)
	{
		this->mBody.clear();
		this->SetProto(rpc::Porto::Json);
		return message->Encode(&this->mBody);
	}

	void Packet::Clear()
	{
		this->mSockId = 0;
		this->mHead.Clear();
		this->mBody.clear();
		this->mTempHead.Clear();
		this->mNet = rpc::Net::Tcp;
		memset(&this->mProtoHead, 0, sizeof(this->mProtoHead));
	}

	std::string Packet::ToString()
	{
		json::w::Document jsonWriter;
		switch(this->mProtoHead.Type)
		{
			case rpc::Type::Request:
				jsonWriter.Add("type", "request");
				break;
			case rpc::Type::Response:
				jsonWriter.Add("type", "response");
				break;
		}
		jsonWriter.Add("from", this->mSockId);
		auto data = jsonWriter.AddObject("head");
		for(auto iter = this->mHead.Begin(); iter != this->mHead.End(); iter++)
		{
			data->Add(iter->first.c_str(), iter->second);
		}
		switch(this->mProtoHead.Porto)
		{
			case rpc::Porto::String:
				jsonWriter.Add("data", this->mBody);
				break;
			case rpc::Porto::Json:
			{
				jsonWriter.AddJson("data", this->mBody);
				break;
			}
			case rpc::Porto::Protobuf:
			{
				break;
			}
		}
		std::string json;
		jsonWriter.Encode(&json);
		return json;
	}

    bool Packet::WriteMessage(const pb::Message* message)
	{
		if (message == nullptr)
		{
			return true;
		}
		this->mBody.clear();
		switch (this->mProtoHead.Porto)
		{
			case rpc::Porto::None:
			case rpc::Porto::Protobuf:
				this->SetProto(rpc::Porto::Protobuf);
				return message->SerializeToString(&mBody);
			case rpc::Porto::Json:
			case rpc::Porto::String:
				return pb::util::MessageToJsonString(*message, &mBody).ok();
		}
		return message->SerializeToString(&mBody);
	}
}