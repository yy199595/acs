//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpAsyncRequest.h"
#include<regex>
#include"Http.h"
#include<iostream>
#include"Util/StringHelper.h"
#include"Script/Extension/Coroutine/LuaCoroutine.h"
namespace Sentry
{
    std::shared_ptr<HttpTask> HttpAsyncRequest::MakeTask(int timeout)
    {
        std::shared_ptr<HttpTask> httpTask = std::make_shared<HttpTask>(timeout);
        this->mTaskId = httpTask->GetRpcId();
        return httpTask;
    }

    std::shared_ptr<LuaHttpTask> HttpAsyncRequest::MakeLuaTask(lua_State *lua, int timeout)
    {
        std::shared_ptr<LuaHttpTask> luaHttpTask = std::make_shared<LuaHttpTask>(lua, timeout);
        this->mTaskId = luaHttpTask->GetRpcId();
        return luaHttpTask;
    }

    LuaHttpTask::LuaHttpTask(lua_State *lua, int timeout)
        : IRpcTask<HttpAsyncResponse>(timeout),mRef(0)
    {
        this->mLua = lua;
        lua_pushthread(lua);
        this->mTaskId = Helper::Guid::Create();
        if(lua_isthread(this->mLua, -1))
        {
            this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
        }
    }

    LuaHttpTask::~LuaHttpTask()
    {
        if(this->mRef != 0)
        {
            luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        }
    }

    void LuaHttpTask::OnTimeout()
    {
        constexpr HttpStatus code = HttpStatus::INTERNAL_SERVER_ERROR;
        std::shared_ptr<HttpAsyncResponse> response(new HttpDataResponse(code));
        this->OnResponse(response);
    }

    void LuaHttpTask::OnResponse(std::shared_ptr<HttpAsyncResponse> response)
    {
        lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        lua_State* coroutine = lua_tothread(this->mLua, -1);
        if(response != nullptr)
        {
            response->GetData().Writer(this->mLua);
            Lua::Coroutine::Resume(coroutine, this->mLua, 1);
            return;
        }
        Lua::Coroutine::Resume(coroutine, this->mLua, 0);
    }

    int LuaHttpTask::Await(std::shared_ptr<HttpRequestClient> client)
    {
        if(this->mRef == 0)
        {
            luaL_error(this->mLua, "not lua coroutine context yield failure");
            return 0;
        }
		this->mRequestClient = client;
        return lua_yield(this->mLua, 0);
    }
}

namespace Sentry
{
	HttpAsyncRequest::HttpAsyncRequest(const std::string& method)
		: mMethod(method)
	{
        this->mTaskId = 0;
	}

    bool HttpAsyncRequest::ParseUrl(const std::string &url)
    {
        std::cmatch what;
        std::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/.*)?");
        if (std::regex_match(url.c_str(), what, pattern))
		{
            this->mHost = std::string(what[2].first, what[2].second);
            this->mPath = std::string(what[4].first, what[4].second);
			this->mProtocol.append(what[1].first, what[1].second);
            this->mPort = std::string(what[3].first, what[3].second);

            if (0 == this->mPort.length()) {
                this->mPort = "http" == this->mProtocol ? "80" : "443";
            }
            return true;
        }
        return false;
    }

	bool HttpAsyncRequest::AddHead(const std::string& key, int value)
	{
		auto iter = this->mHeadMap.find(key);
		if(iter != this->mHeadMap.end())
		{
			return false;
		}
		this->mHeadMap.emplace(key, std::to_string(value));
		return true;
	}

	bool HttpAsyncRequest::AddHead(const std::string& key, const std::string& value)
	{
		auto iter = this->mHeadMap.find(key);
		if(iter != this->mHeadMap.end())
		{
			return false;
		}
		this->mHeadMap.emplace(key, value);
		return true;
	}
    int HttpAsyncRequest::Serailize(std::ostream& os)
	{
		os << this->mMethod << " " << this->mPath << " " << HttpVersion << "\r\n";
		os << "Host:" << this->mHost << "\r\n";
		for(auto iter = this->mHeadMap.begin(); iter != this->mHeadMap.end(); iter++)
		{
			const std::string & key = iter->first;
			const std::string & value = iter->second;
			os << key << ':' << value << "\r\n";
		}
		os << "Connection: close\r\n\r\n";
		this->WriteBody(os);
		return 0;
	}
}

namespace Sentry
{
    void HttpData::Add(const std::string &data)
    {
        this->mData.append(data);
    }

    void HttpData::Add(const char *data, size_t size)
    {
        this->mData.append(data, size);
    }

    void HttpData::Add(const std::string &key, const std::string &data)
    {
        this->mHead.emplace(key, data);
    }

    bool HttpData::Get(const std::string &key, std::string &value) const
    {
        auto iter = this->mHead.find(key);
        if(iter != this->mHead.end())
        {
            value = iter->second;
            return true;
        }
        return false;
    }

    void HttpData::WriterStatus(lua_State *lua) const
    {
        if(this->mStatus != 0)
        {
            lua_pushinteger(lua, this->mStatus);
            lua_setfield(lua, -2, "status");
            const char * err = HttpStatusToString((HttpStatus)this->mStatus);
            lua_pushstring(lua, err);
            lua_setfield(lua, -2, "error");
        }
    }

    void HttpData::Writer(lua_State *lua, const char *name, const std::string &str) const
    {
        if(!str.empty() && name != nullptr)
        {
            lua_pushlstring(lua, str.c_str(), str.size());
            lua_setfield(lua, -2, name);
        }
    }

    void HttpData::Writer(lua_State *lua) const
    {
        lua_createtable(lua, 0, 8);

        this->WriterStatus(lua);
        this->Writer(lua, "data", this->mData);
        this->Writer(lua, "path", this->mPath);
        this->Writer(lua, "method", this->mMethod);
        this->Writer(lua, "version", this->mVersion);
        this->Writer(lua, "address", this->mAddress);
        if(this->mHead.size() > 0)
        {
            lua_createtable(lua, 0, this->mHead.size());
            auto iter = this->mHead.begin();
            for (; iter != this->mHead.end(); iter++)
            {
                const std::string &key = iter->first;
                const std::string &value = iter->second;
                this->Writer(lua, key.c_str(), value);
            }
            lua_setfield(lua, -2, "head");
        }
    }
}


namespace Sentry
{
	HttpGetRequest::HttpGetRequest()
		: HttpAsyncRequest("GET")
	{

	}

	void HttpGetRequest::WriteBody(std::ostream& os) const
	{

	}

	std::shared_ptr<HttpGetRequest> HttpGetRequest::Create(const std::string& url)
	{
		std::shared_ptr<HttpGetRequest> request(new HttpGetRequest());
		return request->ParseUrl(url) ? request : nullptr;
	}
}

namespace Sentry
{
	HttpTask::HttpTask(int timeout)
        : IRpcTask<HttpAsyncResponse>(timeout)
	{
		this->mTaskId = Helper::Guid::Create();
	}

    void HttpTask::OnTimeout()
    {
        std::shared_ptr<HttpDataResponse> response(
                new HttpDataResponse(HttpStatus::REQUEST_TIMEOUT));
        this->OnResponse(response);
    }

	void HttpTask::OnResponse(std::shared_ptr<HttpAsyncResponse> response)
	{
		this->mTask.SetResult(response);
	}
}

namespace Sentry
{
	HttpPostRequest::HttpPostRequest()
		: HttpAsyncRequest("POST")
	{

	}

	void HttpPostRequest::AddBody(const std::string& content)
	{
		this->mBody = content;
		this->AddHead("content-length", (int)content.size());
	}

	void HttpPostRequest::WriteBody(std::ostream& os) const
	{
		os.write(this->mBody.c_str(), this->mBody.size());
	}

	std::shared_ptr<HttpPostRequest> HttpPostRequest::Create(const std::string& url)
	{
		std::shared_ptr<HttpPostRequest> request(new HttpPostRequest());
		return request->ParseUrl(url) ? request : nullptr;
	}

	void HttpPostRequest::AddBody(const char* data, size_t size)
	{
		this->mBody.append(data, size);
		this->AddHead("content-length", size);
	}
}

namespace Sentry
{
	HttpAsyncResponse::HttpAsyncResponse(HttpStatus code)
	{
        this->mHttpData.mStatus = (int)code;
        this->mState = HttpDecodeState::FirstLine;
	}

	int HttpAsyncResponse::OnReceiveLine(std::istream & streamBuffer)
	{
		if (this->mState == HttpDecodeState::FirstLine)
		{
			this->mState = HttpDecodeState::HeadLine;
			streamBuffer >> this->mHttpData.mVersion >> this->mHttpData.mStatus >> this->mHttpError;
            streamBuffer.ignore(2); //放弃\r\n
        }
		if (this->mState == HttpDecodeState::HeadLine)
		{
			std::string lineData;
			while (std::getline(streamBuffer, lineData))
			{
				if (lineData == "\r")
				{
					return 1; //再读
				}
				size_t pos = lineData.find(':');
				if (pos != std::string::npos)
				{
					size_t length = lineData.size() - pos - 2;
					std::string key = lineData.substr(0, pos);
					this->mHttpData.Add(key, lineData.substr(pos + 1, length));
				}
			}
		}
		return -1; //再读一行
	}
}

namespace Sentry
{
	HttpFileResponse::HttpFileResponse(std::fstream* fs, HttpStatus code)
        : HttpAsyncResponse(code)
	{
		this->mFstream = fs;
    }

	HttpFileResponse::~HttpFileResponse() noexcept
	{
		this->mFstream->close();
		delete this->mFstream;
	}

	int HttpFileResponse::OnReceiveSome(std::istream & streamBuffer)
	{
		size_t size = streamBuffer.readsome(this->mBuffer, 128);
		while(size > 0)
		{
			this->mFstream->write(this->mBuffer, size);
			size = streamBuffer.readsome(this->mBuffer, 128);
		}
		return 1;
	}

	int HttpDataResponse::OnReceiveSome(std::istream & streamBuffer)
	{
		char buffer[128] = {0};
		size_t size = streamBuffer.readsome(buffer, 128);
		while(size > 0)
		{
			this->mHttpData.mData.append(buffer, size);
			size = streamBuffer.readsome(buffer, 128);
		}
		return 1;
	}


    void HttpAsyncResponse::SetError(const asio::error_code &code)
    {
        this->mCode = code;
        this->mHttpError = code.message();
    }

	bool HttpAsyncResponse::GetHead(const std::string& key, int& value) const
	{
		auto iter = this->mHttpData.mHead.find(key);
		if(iter != this->mHttpData.mHead.end())
		{
			value = std::stoi(iter->second);
			return true;
		}
		return false;
	}

	bool HttpAsyncResponse::GetHead(const std::string& key, std::string& value) const
	{
		auto iter = this->mHttpData.mHead.find(key);
		if(iter != this->mHttpData.mHead.end())
		{
			value = iter->second;
			return true;
		}
		return false;
	}
}

namespace Sentry
{
    HttpHandlerRequest::HttpHandlerRequest(const std::string & address, const std::string & route)
    {
        this->mRoute = route;
        this->mHttpData.mStatus = 0;
		this->mHttpData.mAddress = address;
        this->mState = HttpDecodeState::FirstLine;
    }

    bool HttpHandlerRequest::GetHead(const std::string &key, int &value) const
    {
		auto iter = this->mHttpData.mHead.find(key);
		if(iter != this->mHttpData.mHead.end())
		{
			value = std::stoi(iter->second);
			return true;
		}
		return false;
    }

    bool HttpHandlerRequest::GetHead(const std::string &key, std::string &value) const
    {
		auto iter = this->mHttpData.mHead.find(key);
		if(iter != this->mHttpData.mHead.end())
		{
			value = iter->second;
			return true;
		}
		return false;
    }

    int HttpHandlerRequest::OnReceiveSome(std::istream &streamBuffer)
    {
        char buffer[128] = {0 };
        size_t size = streamBuffer.readsome(buffer, 128);
        while(size > 0)
        {
            this->mHttpData.Add(buffer, size);
            size = streamBuffer.readsome(buffer, size);
        }
        return 1;
    }

    int HttpHandlerRequest::OnReceiveLine(std::istream &streamBuffer)
    {
        if(this->mState == HttpDecodeState::FirstLine)
        {
            this->mState = HttpDecodeState::HeadLine;
			streamBuffer >> this->mHttpData.mMethod >> this->mUrl >> this->mHttpData.mVersion;
            if(this->mUrl.find(this->mRoute) == std::string::npos) //路由不匹配
            {
                return 2;
            }
			streamBuffer.ignore(2); //去掉\r\n
        }
        if(this->mState == HttpDecodeState::HeadLine)
		{
			std::string lineData;
			while (std::getline(streamBuffer, lineData))
			{
				if (lineData == "\r")
				{
					this->mState = HttpDecodeState::Content;
					break;
				}
				size_t pos = lineData.find(":");
				if (pos != std::string::npos)
				{
					size_t length = lineData.size() - pos - 2;
					std::string key = lineData.substr(0, pos);

					Helper::String::Tolower(key);
                    this->mHttpData.Add(key, lineData.substr(pos + 1, length));
				}
			}
			if (this->mState == HttpDecodeState::Content)
			{
				if (this->mHttpData.mMethod == "GET")
				{
					size_t pos = this->mUrl.find("?");
					if (pos != std::string::npos)
					{
                        this->mHttpData.mData = this->mUrl.substr(pos + 1);
                        this->mHttpData.mPath = this->mUrl.substr(0, pos);
                        return 0;
					}
					return 0;
				}
				else if (this->mHttpData.mMethod != "POST")
				{
					return -2;
				}
                this->mHttpData.mPath = this->mUrl.substr(this->mRoute.size() + 1);
                return 1; //读一部分
			}
		}
        return -1; //读一行
    }
}

namespace Sentry
{
	HttpHandlerResponse::HttpHandlerResponse()
	{
		this->mCount = 0;
		this->mContentSize = 0;
		this->mFstream = nullptr;
		this->mCode = HttpStatus::OK;
	}

	HttpHandlerResponse::~HttpHandlerResponse()
	{
		if(this->mFstream != nullptr)
		{
			this->mFstream->close();
			delete this->mFstream;
		}
	}

	int HttpHandlerResponse::Serailize(std::ostream& os)
	{
		if(this->mCount == 0)
		{
			if(this->mFstream == nullptr)
			{
				this->mContentSize = this->mContent.size();
			}
			this->AddHead("content-length", std::to_string(this->mContentSize));
			os << HttpVersion << ' ' << (int)this->mCode << ' ' << HttpStatusToString(this->mCode) << "\r\n";
			for (auto iter = this->mHeadMap.begin(); iter != this->mHeadMap.end(); iter++)
			{
				const std::string& key = iter->first;
				const std::string& value = iter->second;
				os << key << ":" << value << "\r\n";
			}
			os << "\r\n";
		}
		this->mCount++;
		if(this->mFstream != nullptr)
		{
			char buffer[512] = { 0 };
			this->mFstream->read(buffer, 512);
			if(this->mFstream->gcount() > 0)
			{
				os.write(buffer, this->mFstream->gcount());
				return this->mContentSize - this->mFstream->gcount();
			}
			return 0;
		}
		os.write(this->mContent.c_str(), this->mContent.size());
		return 0;
	}

    bool HttpHandlerResponse::AddHead(const char *key, int value)
    {
        return this->AddHead(key, std::to_string(value));
    }

	bool HttpHandlerResponse::AddHead(const char * key, const std::string& value)
	{
		auto iter = this->mHeadMap.find(key);
		if(iter != this->mHeadMap.end())
		{
			return false;
		}
		this->mHeadMap.emplace(key, value);
		return true;
	}

	void HttpHandlerResponse::SetCode(HttpStatus code)
	{
		this->mCode = code;
	}

	void HttpHandlerResponse::WriteString(const std::string& content)
    {
        this->mContentSize += content.size();
        this->mContent.append(content);
    }

    Json::Writer &HttpHandlerResponse::GetJson()
    {
        if(this->mJson == nullptr)
        {
            this->mJson = std::make_shared<Json::Writer>();
        }
        return *this->mJson;
    }

    void HttpHandlerResponse::WriteString(const char *content, size_t size)
    {
        this->mContentSize += size;
        this->mContent.append(content, size);
    }

	void HttpHandlerResponse::WriteFile(std::fstream * ofstream)
	{
		this->mFstream = ofstream;
		this->mFstream->seekg(0, this->mFstream->end);
		this->mContentSize = ofstream->tellg();
		this->mFstream->seekp(0, this->mFstream->beg);
	}
}