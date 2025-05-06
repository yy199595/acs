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

    const std::string& Head::GetStr(const std::string& key) const
	{
		static std::string empty;
		auto iter = std::find_if(this->mHeader.begin(), this->mHeader.end(),
				[&key](const std::pair<std::string, std::string>& item)
				{
					return item.first == key;
				});
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
            this->mHeader.emplace_back(key, value);
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

    int Message::OnRecvMessage(std::istream& os, size_t size)
    {
		switch(this->mMsg)
		{
			case rpc::msg::bin:
			case rpc::msg::text:
			{
				int len = this->mHead.OnRecvMessage(os, size);
				if (len <= 0)
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
			case rpc::msg::json:
			{
				this->mBody.resize(size);
				char * buffer = (char*)this->mBody.data();
				if(os.readsome(buffer, (int)size) != size)
				{
					return tcp::read::error;
				}
				json::r::Document document;
				if(!document.Decode(buffer, size))
				{
					return tcp::read::decode_error;
				}
				document.Get("type", this->mProtoHead.Type);
				document.Get("proto", this->mProtoHead.Porto);
				document.Get("rpc_id", this->mProtoHead.RpcId);
				std::unique_ptr<json::r::Value> jsonValue;
				if(!document.Get("head", jsonValue))
				{
					return tcp::read::decode_error;
				}
				std::string value;
				for(const char * key : jsonValue->GetAllKey())
				{
					value.clear();
					if(!jsonValue->Get(key, value))
					{
						return tcp::read::decode_error;
					}
					this->mHead.Add(key, value);
				}
				if(document.Get("data", jsonValue))
				{
					this->mBody = jsonValue->ToString();
					return 0;
				}
				document.Get("data", this->mBody);
				break;
			}
		}
		return 0;
    }

    int Message::GetCode(int code) const
    {
        this->mHead.Get(rpc::Header::code, code);
        return code;
    }

    int Message::OnSendMessage(std::ostream &os)
	{
		switch(this->mMsg)
		{
			case rpc::msg::bin:
			{
				this->mProtoHead.Len = this->mHead.GetLength() + this->mBody.size();
				tcp::Data::WriteHead(os, this->mProtoHead, true);

				this->mHead.OnSendMessage(os);
				os.write(this->mBody.c_str(), (int)this->mBody.size());
				break;
			}
			case rpc::msg::text:
			{
				tcp::Data::WriteHead(os, this->mProtoHead, false);
				this->mHead.OnSendMessage(os);
				os.write(this->mBody.c_str(), (int)this->mBody.size());
				break;
			}
			case rpc::msg::json:
			{
				json::w::Document document;
				document.Add("type", this->mProtoHead.Type);
				document.Add("proto", this->mProtoHead.Porto);
				std::unique_ptr<json::w::Value> headObject = document.AddObject("head");
				for(auto iter = this->mHead.Begin(); iter != this->mHead.End(); iter++)
				{
					headObject->Add(iter->first.c_str(), iter->second);
				}
				if(this->mProtoHead.Porto == rpc::Proto::Json)
				{
					document.AddObject("data", this->mBody);
				}
				else
				{
					document.Add("data", this->mBody);
				}
				std::string message = document.JsonString();
				os.write(message.c_str(), (int)message.size());
				break;
			}
		}

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
			message->SetMsg(this->mMsg);
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
			case rpc::Proto::Protobuf:
				if (message->ParseFromString(this->mBody))
				{
					this->mBody.clear();
					return true;
				}
				return false;
			case rpc::Proto::Json:
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
		if(this->mProtoHead.Porto != rpc::Proto::Json)
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

	bool Message::WriteMessage(const json::w::Document* message)
	{
		this->mBody.clear();
		this->SetProto(rpc::Proto::Json);
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
			case rpc::Proto::String:
				jsonWriter.Add("data", this->mBody);
				break;
			case rpc::Proto::Json:
			{
				jsonWriter.AddObject("data", this->mBody);
				break;
			}
			case rpc::Proto::Protobuf:
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
			case rpc::Proto::None:
			case rpc::Proto::Protobuf:
				this->SetProto(rpc::Proto::Protobuf);
				return message->SerializeToString(&mBody);
			case rpc::Proto::Json:
			case rpc::Proto::String:
				return pb::util::MessageToJsonString(*message, &mBody).ok();
		}
		return message->SerializeToString(&mBody);
	}

	Message::Message() noexcept
	{
		this->mSockId = 0;
		this->mBody.clear();
		this->mHead.Clear();
		this->mNet = rpc::Net::Tcp;
		this->mMsg = rpc::msg::bin;
		memset(&this->mProtoHead, 0, sizeof(this->mProtoHead));
	}
}