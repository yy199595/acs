#include "ContentType.h"
#include "Http/Common/Data.h"
namespace http
{
	bool ContentFactory::New(const std::string& content, std::unique_ptr<Data>& data)
	{
		auto iter = this->mContentFactoryMap.find(content);
		if(iter == this->mContentFactoryMap.end())
		{
			return false;
		}
		data = iter->second->New();
		return true;
	}
}