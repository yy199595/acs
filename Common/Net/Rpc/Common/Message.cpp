//
// Created by zmhy0073 on 2022/9/27.
//

#include"Message.h"
#include"Proto/Include/Message.h"
#include"Yyjson/Document/Document.h"
#include"Util/Tools/String.h"

namespace rpc
{
#ifdef __MEMORY_POOL_OPERATOR__
	 std::mutex Message::sAllocLock;
	 std::vector<void *> Message::sAllocQueue;
	void* Message::operator new(std::size_t size)
	{
		std::lock_guard<std::mutex> lock(sAllocLock);
		if(!sAllocQueue.empty())
		{
			void * ptr = sAllocQueue.back();
			sAllocQueue.pop_back();
			return ptr;
		}
		return std::malloc(size);
	}

	void Message::operator delete(void* ptr)
	{
		std::lock_guard<std::mutex> lock(sAllocLock);
		if(sAllocQueue.size() >= 100)
		{
			std::free(ptr);
			return;
		}
		sAllocQueue.emplace_back(ptr);
	}

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

	void Message::Init(const rpc::ProtoHead & protoHead)
	{
		this->mProtoHead = protoHead;
	}

	bool Message::Decode(const char* message, int len)
	{
		if(len <= RPC_PACK_HEAD_LEN)
		{
			return false;
		}

		tcp::Data::Read(message, this->mProtoHead);
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

	bool Message::EncodeToJson(std::string& json)
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
		document.Add("t", this->mProtoHead.Type);
		document.Add("p", this->mProtoHead.Porto);
		return document.Encode(&json);
	}

	bool Message::DecodeFromJson(json::r::Value& document)
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
		document.Get("t", this->mProtoHead.Type);
		document.Get("p", this->mProtoHead.Porto);
		return true;
	}

	bool Message::DecodeFromJson(const char* message, size_t len)
	{
		json::r::Document document;
		if(!document.Decode(message, len))
		{
			return false;
		}
		return this->DecodeFromJson(document);
	}

    int Message::OnRecvMessage(std::istream& os, size_t size)
    {
		if(size < this->mProtoHead.Len)
		{
			return tcp::read::error;
		}

        int len = this->mHead.OnRecvMessage(os, size);
        if (len == -1)
        {
			return tcp::read::error;
		}
		this->mBody.clear();
		this->mProtoHead.Len -= len;
		if (this->mProtoHead.Len == 0)
		{
			return tcp::read::done;
		}
		this->mBody.resize(this->mProtoHead.Len);
		char * buffer = const_cast<char*>(this->mBody.c_str());
		size_t count = os.readsome(buffer, this->mProtoHead.Len);
		if(count != this->mProtoHead.Len)
		{
			return tcp::read::decode_error;
		}
        return tcp::read::done;
    }

    int Message::GetCode(int code) const
    {
        this->mHead.Get("code", code);
        return code;
    }

    int Message::OnSendMessage(std::ostream &os)
	{
		this->mProtoHead.Len = this->mHead.GetLength() + this->mBody.size();
		tcp::Data::Write(os, this->mProtoHead);

		this->mHead.OnSendMessage(os);
		os.write(this->mBody.c_str(), this->mBody.size());
		return 0;
	}

    void Message::SetContent(const std::string & content)
    {
        this->mBody = content;
    }

	void Message::SetContent(char proto, const std::string& content)
	{
		this->mBody = content;
		this->mProtoHead.Porto = proto;
	}

    std::unique_ptr<Message> Message::Clone() const
    {
        std::unique_ptr<Message> message = std::make_unique<Message>();
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

    bool Message::ParseMessage(pb::Message* message)
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

	bool Message::ParseMessage(json::r::Document* message)
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

	bool Message::WriteMessage(json::w::Document* message)
	{
		this->mBody.clear();
		this->SetProto(rpc::Porto::Json);
		return message->Encode(&this->mBody);
	}

	void Message::Clear()
	{
		this->mSockId = 0;
		this->mHead.Clear();
		this->mBody.clear();
		this->mTempHead.Clear();
		this->mNet = rpc::Net::Tcp;
		memset(&this->mProtoHead, 0, sizeof(this->mProtoHead));
	}

	std::string Message::ToString()
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
				jsonWriter.AddObject("data", this->mBody);
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

    bool Message::WriteMessage(const pb::Message* message)
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

	Message::Message() noexcept
	{
		this->mSockId = 0;
		this->mTimeout = 0;
		this->mBody.clear();
		this->mHead.Clear();
		this->mNet = rpc::Net::Tcp;
		memset(&this->mProtoHead, 0, sizeof(this->mProtoHead));
	}
}