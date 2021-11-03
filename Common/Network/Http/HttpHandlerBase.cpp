#include "HttpHandlerBase.h"
#include<Util/StringHelper.h>
#include<Define/CommonDef.h>
namespace GameKeeper
{
	void HttpHandlerBase::ParseHeard(asio::streambuf & buf, size_t size)
    {
        std::string heard = "";
        std::istream is(&buf);
        char *buffer = this->mHandlerBuffer;
        if (size > 1024)
        {
            buffer = new char[size];
        }

        is.readsome(buffer, size);
        heard.append(buffer, size);
        if (buffer != this->mHandlerBuffer)
        {
            delete[]buffer;
        }

        std::vector<std::string> tempArray1;
        std::vector<std::string> tempArray2;
        StringHelper::SplitString(heard, "\r\n", tempArray1);
        for (const std::string &line: tempArray1)
        {
            StringHelper::SplitString(line, ":", tempArray2);
            if (tempArray2.size() == 2)
            {
                const std::string &key = tempArray2[0];
                const std::string &val = tempArray2[1];
                this->mHeardMap.insert(std::make_pair(key, val));
#ifdef __DEBUG__
                GKDebugWarning(key << "  =  " << val);
#endif
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
