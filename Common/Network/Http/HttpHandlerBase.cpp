#include "HttpHandlerBase.h"
#include<Util/StringHelper.h>
namespace GameKeeper
{
#ifdef __DEBUG__
	std::string HttpHandlerBase::PrintHeard()
	{
		std::stringstream ss;
		auto iter = this->mHeardMap.begin();
		for (; iter != this->mHeardMap.end(); iter++)
		{
			const std::string & key = iter->first;
			const std::string & val = iter->second;
			ss << key << "=" << val << "\n";
		}
		return ss.str();
	}
#endif // __DEBUG__

    void HttpHandlerBase::Clear()
    {
        this->mHeardMap.clear();
        this->mContentLength = 0;
        memset(this->mHandlerBuffer, 0, 1024);
    }

    bool HttpHandlerBase::GetContentType(std::string &content)
    {
        auto iter = this->mHeardMap.find("Content-Type");
        if(iter != this->mHeardMap.end())
        {
            content = iter->second;
            return true;
        }
        return false;
    }

	void HttpHandlerBase::ParseHeard(asio::streambuf & buf)
    {
		std::string line;
		std::istream is(&buf);
		std::getline(is, line);

		while (std::getline(is, line))
		{
			if (line == "\r")
			{
				break;
			}
			size_t pos = line.find(':');
			if (pos != std::string::npos)
			{
				size_t length = line.size() - pos - 2;
				std::string key = line.substr(0, pos);
				std::string val = line.substr(pos + 1, length);
				this->mHeardMap.insert(std::make_pair(key, val));
			}		
		}
		std::string data;
		this->mContentLength = 0;
		if (this->GetHeardData("Content-Length", data))
		{
			this->mContentLength = std::stoul(data);
		}
    }
	bool HttpHandlerBase::GetHeardData(const std::string & key, std::string & value)
	{
		auto iter = this->mHeardMap.find(key);
		if (iter != this->mHeardMap.end())
		{
			value = iter->second;
			return true;
		}
		return false;
	}
}
