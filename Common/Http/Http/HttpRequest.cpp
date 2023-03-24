//
// Created by yjz on 2022/10/27.
//

#include"HttpRequest.h"
#include<regex>
#include"Json/Lua/values.hpp"
#include"String/StringHelper.h"


namespace Http
{
    Request::Request(const char *method, const std::string & from)
        : mMethod(method)
    {
		this->mFrom = from;
		this->mVersion = HttpVersion;
        this->mState = DecodeState::None;
    }

    bool Request::SetUrl(const std::string &url)
	{
		this->mUrl = url;
		std::cmatch what;
		std::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/.*)?");
		if (std::regex_match(url.c_str(), what, pattern))
		{
			this->mHost = std::string(what[2].first, what[2].second);
			this->mPath = std::string(what[4].first, what[4].second);
			this->mProtocol.append(what[1].first, what[1].second);
			this->mPort = std::string(what[3].first, what[3].second);

			if (0 == this->mPort.length())
			{
				this->mPort = "http" == this->mProtocol ? "80" : "443";
			}
			return true;
		}
		return false;
	}
    bool Request::OnRead(std::istream &buffer)
    {
        if(this->mState == DecodeState::None)
        {
            buffer >> this->mUrl;
            buffer >> this->mVersion;
            buffer.ignore(2); //忽略\r\n
            this->mState = DecodeState::Head;
        }
        if(this->mState == DecodeState::Head)
        {
            if(!this->mHead.OnRead(buffer))
            {
                return false;
            }
            this->mState = DecodeState::Body;
        }
        return this->OnReadContent(buffer);
    }

    int Request::Serialize(std::ostream &os)
    {
        return this->OnWrite(os);
    }

    int Request::OnWrite(std::ostream &os)
    {
        os << this->mMethod << " " << this->mPath << " " << this->mVersion << "\r\n";
        os << "Host:" << this->mHost << "\r\n";
        this->mHead.OnWrite(os);
        os << "Connection: close\r\n\r\n";
        return this->OnWriteContent(os);
    }
}

namespace Http
{
    bool GetRequest::OnReadContent(std::istream &buffer)
    {
		this->mPath = this->mUrl;
        size_t pos = this->mUrl.find('?');
        if (pos != std::string::npos)
        {
            std::vector<std::string> result;
            this->mPath = this->mUrl.substr(0, pos);
            std::string parameter = this->mUrl.substr(pos + 1);
            if(Helper::Str::Split(parameter, "&", result) > 0)
            {
                for (const std::string &filed: result)
                {
                    size_t pos1 = filed.find('=');
                    if(pos1 != std::string::npos)
                    {
                        std::string key = filed.substr(0, pos1);
                        std::string val = filed.substr(pos1 + 1);
                        this->mParameters.emplace(key, val);
                    }
                }
            }
        }
        return true;
    }

    bool GetRequest::WriteLua(lua_State* lua) const
    {
        if (this->mParameters.empty())
        {
            return false;
        }
        rapidjson::Document document;
        if (this->WriteDocument(&document))
        {
            values::pushValue(lua, document);
            return true;
        }
        return false;         
    }

    bool GetRequest::WriteDocument(rapidjson::Document* document) const
    {
        if (this->mParameters.empty())
        {
            return false;
        }       
        document->SetObject();
        auto iter = this->mParameters.begin();
        for (; iter != this->mParameters.end(); iter++)
        {
            const char* key = iter->first.c_str();
            const char* value = iter->second.c_str();
            document->AddMember(rapidjson::StringRef(key), rapidjson::StringRef(value), document->GetAllocator());
        }
        document->EndObject(this->mParameters.size());
        return true;
    }

    bool GetRequest::GetParameter(const std::string & key, std::string & value)
    {
        auto iter = this->mParameters.find(key);
        if(iter == this->mParameters.end())
        {
            return false;
        }
        value = iter->second;
        return true;
    }

    int GetRequest::OnWriteContent(std::ostream &buffer)
    {
        return 0;
    }
}

namespace Http
{
    bool PostRequest::OnReadContent(std::istream &buffer)
    {
        char buff[128] = {0};
        this->mPath = this->mUrl;
        size_t size = buffer.readsome(buff, sizeof(buff));
        while(size > 0)
        {
            this->mContent.append(buff, size);
            size = buffer.readsome(buff, sizeof(buff));
        }
        return false;
    }

    bool PostRequest::WriteLua(lua_State* lua) const
    {
        const char * str = this->mContent.c_str();
        const size_t length = this->mContent.size();
        if (str != nullptr && length > 0)
        {
            rapidjson::extend::StringStream s(str, length);
            values::pushDecoded(lua, s);
            return true;
        }
        return false;
    }

    bool PostRequest::WriteDocument(rapidjson::Document* document) const
    {
        const char* str = this->mContent.c_str();
        const size_t length = this->mContent.size();
        if (str == nullptr || length == 0)
        {
            return false;
        }
        return !document->Parse(str, length).HasParseError();
    }

    void PostRequest::Str(const std::string &str)
    {
        this->mContent = str;
    }

    void PostRequest::Json(const std::string &json)
    {
        this->Json(json.c_str(),json.size());
    }

    void PostRequest::Json(const char *str, size_t size)
    {
        this->mContent.append(str, size);
        this->mHead.Add("content-type", "applocation/json");
    }

    int PostRequest::OnWriteContent(std::ostream &buffer)
    {
        buffer.write(this->mContent.c_str(), this->mContent.size());
        return 0;
    }
}

namespace Http
{
    std::shared_ptr<Http::Request> New(const std::string &method, const std::string & from)
    {
        if(method == "GET")
        {
            return std::make_shared<GetRequest>(from);
        }
        if(method == "POST")
        {
            return std::make_shared<PostRequest>(from);
        }
        return nullptr;
    }
}