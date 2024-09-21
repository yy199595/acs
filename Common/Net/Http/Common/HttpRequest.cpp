//
// Created by yjz on 2022/10/27.
//
#include"HttpRequest.h"
#include"Util/Tools/Math.h"
#include"Util/Tools/TimeHelper.h"
#include"Log/Common/CommonLogDef.h"
#include"Yyjson/Document/Document.h"
#include"Lua/Engine/UserDataParameter.h"

namespace http
{
    Request::Request() : mSockId(0), mBody(nullptr)
    {
		this->mConeSize = 0;
		this->mTimeout = 0;
		this->mDecodeStatus = tcp::Decode::None;
    }

	Request::Request(const char* method)
		: mSockId(0) , mBody(nullptr), mUrl(method)
	{
		this->mConeSize = 0;
		this->mTimeout = 0;
		this->mDecodeStatus = tcp::Decode::None;
	}

	bool Request::GetIp(std::string& ip) const
	{
		if(this->mHead.Get("X-Forwarded-For", ip))
		{
			return true;
		}
		return this->mHead.Get(http::Header::RealIp, ip);
	}

    bool Request::SetUrl(const std::string &url)
	{
		return this->mUrl.Decode(url);
	}

	bool Request::SetUrl(const std::string& url, const http::FromContent& fromData)
	{
		std::string query = fromData.Serialize();
		return this->mUrl.Decode(fmt::format("{}?{}", url, query));
	}

    int Request::OnRecvMessage(std::istream& buffer, size_t size)
	{
		if(this->mDecodeStatus == tcp::Decode::None)
		{
			int code = this->mUrl.OnRecvMessage(buffer, size);
			if(code != tcp::ReadDone)
			{
				return code;
			}
			this->mDecodeStatus = tcp::Decode::MessageHead;
			return tcp::ReadOneLine;
		}
		if(this->mDecodeStatus == tcp::Decode::MessageHead)
		{
			int code = this->mHead.OnRecvMessage(buffer, size);
			if(code != tcp::ReadDone)
			{
				return code;
			}
			this->mConeSize = 0;
			std::string contentType;
			this->mDecodeStatus = tcp::Decode::MessageBody;
			if(this->mHead.GetContentLength(this->mConeSize))
			{
				if(this->mConeSize == 0)
				{
					return tcp::ReadDecodeError;
				}
			}
			return tcp::ReadPause;
		}
		if(this->mDecodeStatus == tcp::Decode::MessageBody)
		{
			if(size == 0)
			{
				if(this->mBody != nullptr && !this->mBody->OnDecode())
				{
					LOG_ERROR("parse http body fail : {}", this->mUrl.ToString());
					return tcp::ReadDecodeError;
				}
				return tcp::ReadDone;
			}
			int flag = this->mBody->OnRecvMessage(buffer, size);
			if(this->mConeSize > 0 && this->mBody->ContentLength() >= this->mConeSize)
			{
				flag = tcp::ReadDone;
			}
			if(flag == 0 && !this->mBody->OnDecode())
			{
				return tcp::ReadDecodeError;
			}
			return flag;
		}
		return tcp::ReadDone;
	}

	void Request::Clear()
	{
		this->mUrl.Clear();
		this->mHead.Clear();
		this->mTimeout = 0;
		this->mBody = nullptr;
		this->mDecodeStatus = tcp::Decode::None;
	}

    int Request::OnSendMessage(std::ostream &os)
    {
		//this->mHead.SetKeepAlive(true);
		if(this->mDecodeStatus == tcp::Decode::None)
		{
			this->mUrl.OnSendMessage(os);
			this->mHead.Add("Host", this->mUrl.Host());
			this->mDecodeStatus = tcp::Decode::MessageHead;
		}
		if(this->mDecodeStatus == tcp::Decode::MessageHead)
		{
			if(this->mBody != nullptr)
			{
				this->mBody->OnWriteHead(os);
			}
			this->mHead.OnSendMessage(os);
			this->mDecodeStatus = tcp::Decode::MessageBody;
		}
		if(this->mBody && this->mDecodeStatus == tcp::Decode::MessageBody)
		{
			return this->mBody->OnWriteBody(os);
		}
		return 0;
    }

	int Request::WriteToLua(lua_State* lua) const
	{
		lua_createtable(lua, 0, 3);
		{
			lua_pushstring(lua, "head");
			tcp::IHeader::WriteLua(lua, this->mHead);
			lua_rawset(lua, -3);
		}
		{
			lua_pushstring(lua, "query");
			const http::FromContent& fromData = this->mUrl.GetQuery();
			(const_cast<http::FromContent&>(fromData)).WriteToLua(lua);
			lua_rawset(lua, -3);
		}
		{
			lua_pushstring(lua, "data");
			this->WriteMessageToLua(lua);
			lua_rawset(lua, -3);
		}
		return 1;
	}

	std::string Request::ToString()
	{
		std::string str;
		json::w::Document jsonWriter;
		jsonWriter.Add("url", this->mUrl.ToStr());
		jsonWriter.Add("method", this->mUrl.Method());
		jsonWriter.Add("version", this->mUrl.Version());

		if(this->mHead.Get(http::Header::RealIp, str))
		{
			jsonWriter.Add("ip", str);
		}
		if(this->mHead.Get(http::Header::Auth, str))
		{
			jsonWriter.Add("token", str);
		}

		if(this->mUrl.GetQuery().Size() > 0)
		{
			jsonWriter.Add("data", this->mUrl.Query()->ToStr());
		}
		else if(this->mBody != nullptr)
		{
			jsonWriter.Add("data", this->mBody->ToStr());
		}
		std::string json;
		jsonWriter.Encode(&json, true);
		return json;
	}

	void Request::WriteMessageToLua(lua_State* lua) const
	{
		if(this->mBody == nullptr)
		{
			lua_pushnil(lua);
			return;
		}
		this->mBody->WriteToLua(lua);
	}

	bool Request::IsMethod(const std::string& method) const
	{
		if(method.empty()) return true;
		const std::string & m = this->mUrl.Method();
		return m.find(method) != std::string::npos;
	}

	void Request::SetContent(const char* t, const std::string& content)
	{
		this->SetContent(t,  content.c_str(), content.size());
	}

	void Request::SetContent(const json::w::Document& document)
	{
		std::unique_ptr<http::JsonContent> jsonContent = std::make_unique<http::JsonContent>();
		{
			jsonContent->Write(document);
			this->mBody = std::move(jsonContent);
		}
	}

	void Request::SetContent(const char* t, const char* content, size_t size)
	{
		std::unique_ptr<http::TextContent> customData(new http::TextContent());
		{
			customData->SetContent(t, content, size);
			this->mBody = std::move(customData);
		}
	}
}