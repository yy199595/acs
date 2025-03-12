//
// Created by yjz on 2022/10/27.
//
#include<sstream>
#include"httpHead.h"
#include"Util/Tools/Math.h"
#include"Proto/Message/IProto.h"
#include"Yyjson/Document/Document.h"
#include "Util/Tools/String.h"

namespace http
{
	Head::Head()
	{
		this->mCounter = 0;
		this->mKeepAlive = false;
	}

	bool Head::GetContentType(std::string& type) const
	{
		if (!this->Get("content-type", type))
		{
			return false;
		}
		size_t pos = type.find(';');
		if (pos != std::string::npos)
		{
			type = type.substr(0, pos);
		}
		return true;
	}

	bool Head::GetContentLength(long long& length) const
	{
		if (!this->Get("content-length", length))
		{
			return false;
		}
		return true;
	}

	void Head::Clear()
	{
		this->mCounter = 0;
		this->mHeader.clear();
	}

	bool Head::KeepAlive() const
	{
		std::string type;
		if(!this->Get(http::Header::Connection, type))
		{
			return false;
		}
		return type == http::Header::KeepAlive;
	}

    int Head::OnRecvMessage(std::istream& buffer, size_t size)
    {
        std::string lineData, key, value;
        if(std::getline(buffer, lineData))
        {
			this->mCounter = 0;
			if(lineData == "\r")
            {
				return tcp::ReadDone;
            }
			if(lineData.back() == '\r')
			{
				lineData.pop_back();
			}
            size_t pos = lineData.find(':');
            if (pos != std::string::npos)
            {
                key.assign(lineData.c_str(), pos);
				if(lineData[pos + 1] == ' ')
				{
					pos++;
				}
				value.clear();
				help::Str::Tolower(key);
				if(lineData.size() > pos + 1)
				{
					value.assign(lineData.c_str() + pos + 1);
				}
				this->Add(key, value);
            }
			return tcp::ReadOneLine;
		}
		this->mCounter++;
		if(this->mCounter >= 20)
		{
			return tcp::ReadError;
		}
		return tcp::ReadOneLine;
    }

    int Head::OnSendMessage(std::ostream& buffer)
    {
		if(!this->Has(http::Header::Connection))
		{
			const char * connect = this->mKeepAlive ?
								   http::Header::KeepAlive : http::Header::Close;
			this->Add(http::Header::Connection, connect);
		}

		auto iter = this->mHeader.begin();
        for(; iter != this->mHeader.end(); iter++)
        {
            const std::string & key = iter->first;
            const std::string & value = iter->second;
            buffer << key << ": " << value << CRLF;
        }
		buffer << CRLF;
        return 0;
    }

	std::string Head::ToString()
	{
		json::w::Document document;
		auto iter = this->mHeader.begin();
		for(; iter != this->mHeader.end(); iter++)
		{
			const std::string& key = iter->first;
			const std::string& value = iter->second;
			document.Add(key.c_str(), value);
		}
		return document.JsonString(true);
	}
}