#include "HttpHandlerBase.h"
#include<Util/StringHelper.h>
#include<Define/CommonDef.h>
namespace Sentry
{
	void HttpHandlerBase::ParseHeard(asio::streambuf & buf)
	{
		std::istream is(&buf);
		std::istreambuf_iterator<char> eos;
		std::string heard(std::istreambuf_iterator<char>(is), eos);

		std::vector<std::string> tempArray1;
		std::vector<std::string> tempArray2;
		StringHelper::SplitString(heard, "\n", tempArray1);
		for (const std::string & line : tempArray1)
		{
			StringHelper::SplitString(line, ":", tempArray2);
			if (tempArray2.size() == 2)
			{
				const std::string & key = tempArray2[0];
				const std::string & val = tempArray2[1];
				this->mHeardMap.insert(std::make_pair(key, val));
#ifdef __DEBUG__
				SayNoDebugWarning(key << "  =  " << val);
#endif
			}
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
