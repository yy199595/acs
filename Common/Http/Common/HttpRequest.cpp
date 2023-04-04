//
// Created by yjz on 2022/10/27.
//

#include"HttpRequest.h"
#include<regex>
#include"Util/Json/Lua/values.hpp"
#include"Util/String/StringHelper.h"
#include"Util/File/DirectoryHelper.h"
namespace Http
{
    Parameter::Parameter(const std::string & content)
    {
        std::vector<std::string> result;
        if(Helper::Str::Split(content, "&", result) > 0)
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

    bool Parameter::Get(std::vector<std::string> &keys)
    {
        if(this->mParameters.empty())
        {
            return false;
        }
        keys.reserve(this->mParameters.size());
        auto iter = this->mParameters.begin();
        for(; iter != this->mParameters.end(); iter++)
        {
            keys.emplace_back(iter->first);
        }
        return true;
    }

    bool Parameter::Get(const std::string &key, std::string &value)
    {
        auto iter = this->mParameters.find(key);
        if(iter == this->mParameters.end())
        {
            return false;
        }
        value = iter->second;
        return true;
    }
}

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
    int Request::OnRead(std::istream &buffer)
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
            if (this->mHead.OnRead(buffer) == HTTP_READ_LINE)
            {
                return HTTP_READ_LINE;
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
    int GetRequest::OnReadContent(std::istream &buffer)
    {
		this->mPath = this->mUrl;
        size_t pos = this->mUrl.find('?');
        if (pos != std::string::npos)
        {
            this->mContent = this->mUrl.substr(pos + 1);
        }
        return HTTP_READ_COMPLETE;
    }

    bool GetRequest::WriteLua(lua_State* lua) const
    {
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
        std::vector<std::string> keys;
        Http::Parameter parameter(this->mContent);
        if(!parameter.Get(keys))
        {
            return false;
        }
        std::string str;
        for(const std::string & tmp : keys)
        {
            const char* key = tmp.c_str();
            parameter.Get(tmp, str);
            const char* value = str.c_str();
            document->AddMember(rapidjson::StringRef(key),
                                rapidjson::StringRef(value), document->GetAllocator());
        }
        document->SetObject();
        document->EndObject(keys.size());
        return true;
    }

    int GetRequest::OnWriteContent(std::ostream &buffer)
    {
        return 0;
    }
}

namespace Http
{
    int PostRequest::OnReadContent(std::istream &buffer)
    {
		char buff[128] = {0};
		int length = this->mHead.ContentLength();
		if(length > 0)
		{
			size_t size = buffer.readsome(buff, sizeof(buff));
			while(size > 0)
			{
				this->mContent.append(buff, size);
				size = buffer.readsome(buff, sizeof(buff));
			}
			int len = length - this->mContent.size();
			return len <= 512 ? len : 512;
		}
		size_t size = buffer.readsome(buff, sizeof(buff));
		while(size > 0)
		{
			this->mContent.append(buff, size);
			size = buffer.readsome(buff, sizeof(buff));
		}
		return HTTP_READ_SOME;
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
        this->mContent.assign(str);
		this->mHead.Add(Http::HeadName::ContentType, Http::ContentName::TEXT);
		this->mHead.Add(Http::HeadName::ContentLength, this->mContent.size());
    }

    void PostRequest::Json(const std::string &json)
    {
        this->Json(json.c_str(),json.size());
    }

    void PostRequest::Json(const char *str, size_t size)
    {
        this->mContent.assign(str, size);
		this->mHead.Add(Http::HeadName::ContentType, Http::ContentName::JSON);
		this->mHead.Add(Http::HeadName::ContentLength, this->mContent.size());
    }

    int PostRequest::OnWriteContent(std::ostream &buffer)
    {
        buffer.write(this->mContent.c_str(), this->mContent.size());
        return 0;
    }

	void PostRequest::OnComplete()
	{
		this->mPath = this->mUrl;
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