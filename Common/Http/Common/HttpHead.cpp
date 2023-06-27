//
// Created by yjz on 2022/10/27.
//
#include"httpHead.h"
#include"Util/String/StringHelper.h"
namespace Http
{
    bool Head::Add(const std::string &k, int v)
    {
        auto iter = this->mHeads.find(k);
        if(iter != this->mHeads.end())
        {
            return false;
        }
        this->mHeads.emplace(k, std::to_string(v));
        return true;
    }

    bool Head::Add(const std::string &k, const std::string &v)
    {
        auto iter = this->mHeads.find(k);
        if(iter != this->mHeads.end())
        {
            return false;
        }
        this->mHeads.emplace(k, v);
        return true;
    }

	bool Head::Get(const std::string& k, int& v) const
	{
		auto iter = this->mHeads.find(k);
		if (iter == this->mHeads.end())
		{
			return false;
		}
		v = std::stoi(iter->second);
		return true;
	}

    bool Head::Get(const std::string& k, long long& v) const
    {
        auto iter = this->mHeads.find(k);
        if (iter == this->mHeads.end())
        {
            return false;
        }
        v = std::stoll(iter->second);
        return true;
    }

    bool Head::Get(const std::string &k, std::string &v) const
    {
        auto iter = this->mHeads.find(k);
        if(iter == this->mHeads.end())
        {
            return false;
        }
        v = iter->second;
        return true;
    }

    int Head::OnRead(std::istream &buffer)
    {
        std::string lineData;
        while(std::getline(buffer, lineData))
        {
            if(lineData == "\r")
            {
				this->Get(Http::HeadName::ContentType, this->mContentType);
				this->Get(Http::HeadName::ContentLength, this->mContentLength);
                return HTTP_READ_COMPLETE;
            }
            size_t pos = lineData.find(':');
            if (pos != std::string::npos)
            {
                size_t length = lineData.size() - pos - 2;
                std::string key = lineData.substr(0, pos);
				std::string val = lineData.substr(pos + 1, length);

				Helper::Str::Tolower(key);
				Helper::Str::ClearBlank(val);
				this->mHeads.emplace(key, val);
            }
        }
        return HTTP_READ_LINE;
    }

    int Head::OnWrite(std::ostream &buffer)
    {
		auto iter = this->mHeads.begin();
        for(; iter != this->mHeads.end(); iter++)
        {
            const std::string & key = iter->first;
            const std::string & value = iter->second;
            buffer << key << ": " << value << "\r\n";
        }
		//buffer << "Access-Control-Allow-Origin: *" << "\r\n";
		//buffer << "Access-Control-Allow-Credentials: true" << "\r\n";
        return 0;
    }

	void Head::Keys(std::vector<std::string>& keys) const
	{
		keys.reserve(this->mHeads.size());
		auto iter = this->mHeads.begin();
		for(; iter != this->mHeads.end(); iter++)
		{
			keys.emplace_back(iter->first);
		}
	}

	int Head::WriteToLua(lua_State* lua) const
	{
		size_t size = this->mHeads.size();
		lua_createtable(lua, 0, (int)size);
		for(const auto & item : this->mHeads)
		{
			const std::string & key = item.first;
			const std::string & val = item.second;
			lua_pushlstring(lua, val.c_str(), val.size());
			lua_setfield(lua, -2, key.c_str());
		}
		return 0;
	}
}