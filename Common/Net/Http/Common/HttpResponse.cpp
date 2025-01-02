//
// Created by yjz on 2022/10/27.
//

#include"HttpResponse.h"
#include"Util/File/DirectoryHelper.h"
#include"Lua/Engine/UserDataParameter.h"
#include "Util/Tools/Math.h"
#include "Log/Common/CommonLogDef.h"

namespace http
{
	Response::Response()
	{
		this->mContSize = 0;
		this->mBody = nullptr;
		this->mVersion = http::Version;
		this->mCode = (int)HttpStatus::OK;
		this->mParseState = tcp::Decode::None;
	}

	int Response::OnRecvMessage(std::istream& buffer, size_t size)
	{
		if (size == 0)
		{
			if(this->mBody!= nullptr)
			{
				this->mBody->OnDecode();
			}
			return tcp::ReadDone;
		}
		if (this->mParseState == tcp::Decode::None)
		{
			buffer >> this->mVersion >> this->mCode;
			std::getline(buffer, this->mError);
			if (!this->mError.empty() && this->mError.back() == '\r')
			{
				this->mError.pop_back();
			}
			this->mParseState = tcp::Decode::MessageHead;
		}
		if (this->mParseState == tcp::Decode::MessageHead && !buffer.eof())
		{
			switch (this->mHead.OnRecvMessage(buffer, size))
			{
				case tcp::ReadOneLine:
					return tcp::ReadOneLine;
				case tcp::ReadError:
					return tcp::ReadError;
				case tcp::ReadDone:
					this->mParseState = tcp::Decode::MessageBody;
					break;
			}
//			if(this->mCode != (int)HttpStatus::OK)
//			{
//				return tcp::ReadDone;
//			}
			if(this->mBody == nullptr)
			{
				std::string content_type;
				this->mHead.GetContentType(content_type);
				if(content_type.find(http::Header::JSON) != std::string::npos)
				{
					this->mBody = std::make_unique<http::JsonContent>();
				}
				else if (content_type.find(http::Header::XML) != std::string::npos)
				{
					this->mBody = std::make_unique<http::XMLContent>();
				}
				else
				{
					this->mBody = std::make_unique<http::TextContent>();
				}
			}
			this->mContSize = 0;
			if(!this->mHead.GetContentLength(this->mContSize))
			{
				if(this->mHead.Has(http::Header::TransferEncoding))
				{
					return tcp::ReadOneLine;
				}
				return tcp::ReadDecodeError;
			}
		}
		if (this->mParseState == tcp::Decode::MessageBody)
		{
			if(this->mHead.Has(http::Header::TransferEncoding))
			{
				if(this->mContSize == 0)
				{
					std::string line;
					if (!std::getline(buffer, line))
					{
						return tcp::ReadDecodeError;
					}
					line.pop_back();
					if(line.empty())
					{
						return tcp::ReadDone;
					}
					this->mContSize = std::stoi(line, nullptr, 16);
					if(this->mContSize == 0)
					{
						return tcp::ReadDone;
					}
					if (this->mContSize > 0)
					{
						return this->mContSize;
					}
				}
				this->mBody->OnRecvMessage(buffer, size);
				{
					this->mContSize = 0;
					return tcp::ReadOneLine;
				}
			}
			int flag = this->mBody->OnRecvMessage(buffer, size);
			if(this->mContSize > 0 && this->mBody->ContentLength() >= this->mContSize)
			{
				flag = tcp::ReadDone;
			}
			return flag;
		}
		return tcp::ReadError;
	}

	void Response::Json(const std::string& json)
	{
		this->Json(json.c_str(), json.size());
	}

	void Response::Json(const char* json, size_t size)
	{
		this->mBody = std::make_unique<http::JsonContent>(json, size);
	}

	void Response::Text(const char* text, size_t size)
	{
		std::unique_ptr<http::TextContent> custom = std::make_unique<http::TextContent>();
		{
			custom->SetContent(http::Header::TEXT, text, size);
		}
		this->mBody = std::move(custom);
	}

	void Response::Json(json::w::Document& document)
	{
		std::unique_ptr<http::JsonContent> jsonData = std::make_unique<http::JsonContent>();
		{
			jsonData->Write(document);
		}
		this->mBody = std::move(jsonData);
	}

	void Response::SetContent(const std::string & type, const std::string& str)
	{
		std::unique_ptr<http::TextContent> customData = std::make_unique<http::TextContent>();
		{
			customData->SetContent(type, str.c_str(), str.size());
		}
		this->mBody = std::move(customData);
	}

	bool Response::File(const std::string & type, const std::string& path)
	{
		this->mCode = (int)HttpStatus::OK;
		std::unique_ptr<http::FileContent> fileData(new http::FileContent());
		{
			if(!fileData->OpenFile(path, type))
			{
                this->mBody = nullptr;
                this->SetCode(HttpStatus::NOT_FOUND);
				return false;
			}
			this->mBody = std::move(fileData);
		}
		return true;
	}

	bool Response::OpenOrCreateFile(const std::string& path)
	{
		if(!help::dir::IsValidPath(path))
		{
			return false;
		}
		std::string dir;
		help::dir::GetDirByPath(path, dir);
		if(!help::dir::DirectorIsExist(dir))
		{
			help::dir::MakeDir(dir);
		}
		std::unique_ptr<http::FileContent> fileData = std::make_unique<http::FileContent>();
		{
			if(!fileData->MakeFile(path))
			{
				return false;
			}
			this->mBody = std::move(fileData);
		}
		return true;
	}

	void Response::Clear()
	{
		this->mHead.Clear();
		this->mError.clear();
		this->mVersion.clear();
		this->mBody = nullptr;
		this->mCode = (int)HttpStatus::OK;
		this->mParseState = tcp::Decode::None;
	}

	void Response::SetCode(HttpStatus code)
	{
		this->mCode = (int)code;
		this->mError = HttpStatusToString(code);
	}

	std::string Response::ToString()
	{
		std::string json;
		json::w::Document js;
		js.Add("status", this->mCode);
		js.Add("status_text", this->mError);
		js.AddJson("header", this->mHead.ToString());
		if(this->mBody != nullptr)
		{
			js.Add("data", this->mBody->ToStr());
		}
		js.Encode(&json, true);
		return json;
	}

	int Response::WriteToLua(lua_State* lua) const
	{
		lua_createtable(lua, 0, 4);
		{
			lua_pushstring(lua, "code");
			lua_pushinteger(lua, this->mCode);
			lua_rawset(lua, -3);
		}
		{
			lua_pushstring(lua, "status");
			lua_pushstring(lua, this->mError.c_str());
			lua_rawset(lua, -3);
		}
		{
			std::string value;
			lua_pushstring(lua, "head");
			tcp::IHeader::WriteLua(lua, this->mHead);
			lua_rawset(lua, -3);
		}
		if (this->mBody == nullptr)
		{
			return 1;
		}
		lua_pushstring(lua, "body");
		this->mBody->WriteToLua(lua);
		lua_rawset(lua, -3);
		return 1;
	}

	int Response::OnSendMessage(std::ostream& buffer)
	{
		HttpStatus code = (HttpStatus)this->mCode;
		if(this->mParseState == tcp::Decode::None)
		{
			buffer << http::Version << ' ' << this->mCode << ' '
				   << HttpStatusToString(code) << "\r\n";
			this->mParseState = tcp::Decode::MessageHead;
		}
		if(this->mParseState == tcp::Decode::MessageHead)
		{
			if(this->mBody != nullptr)
			{
				this->mBody->OnWriteHead(buffer);
			}
			this->mHead.OnSendMessage(buffer);
			this->mParseState = tcp::Decode::MessageBody;
		}
		if(this->mBody != nullptr)
		{
			return this->mBody->OnWriteBody(buffer);
		}
		return 0;
	}
}