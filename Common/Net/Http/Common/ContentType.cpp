#include "ContentType.h"
#include "Http/Common/Content.h"
namespace http
{
	bool ContentFactory::New(const std::string& content, std::unique_ptr<Content>& data)
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