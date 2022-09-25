//
// Created by zmhy0073 on 2022/8/25.
//
#include"Rpc.h"
namespace Tcp
{
    BinMessage::BinMessage()
    {
        this->mSize = 0;
        this->mBuffer = nullptr;
        this->mType = Tcp::Type::None;
        this->mProto = Tcp::Porto::None;
    }

    BinMessage::~BinMessage()
    {
        if(this->mBuffer != nullptr)
        {
            delete []this->mBuffer;
        }
    }

    int BinMessage::DecodeHead(std::istream &is)
    {
        union {
            int len;
            char buf[sizeof(int)];
        } buffer;
        is.readsome(buffer.buf, sizeof(int));

        this->mSize = buffer.len;
        this->mType = (Tcp::Type)is.get();
        this->mProto = (Tcp::Porto)is.get();
        return this->mSize;
    }

    bool BinMessage::DecodeBody(std::istream &is)
    {
        if (this->mBuffer == nullptr)
        {
            if(this->mType == Tcp::Type::UnitRequest)
            {
                this->mSize -= sizeof(long long);
                union{
                    long long id;
                    char buf[sizeof(long long)];
                } bb;
                is.readsome(bb.buf, sizeof(long long));
            }
            this->mBuffer = new char[this->mSize];
        }
        return is.readsome(this->mBuffer, this->mSize) == this->mSize;
    }

    const char *BinMessage::GetData(int &size) const
    {
        size = this->mSize;
        return this->mBuffer;
    }
}

namespace Tcp
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

	bool Head::Parse(std::istream& os)
	{
		std::string line, key, value;
		this->mType = (Tcp::Type)os.get();
		this->mProto = (Tcp::Porto)os.get();
		while (std::getline(os, line))
		{
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
		return true;
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

	bool Head::Get(const std::string& key, std::string& value)
	{

	}
}
