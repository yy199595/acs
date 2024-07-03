
#include"Base64Helper.h"
#include"Proto/Bson/base64.h"
namespace help
{
	std::string Base64::Encode(const std::string& str)
	{
		return _bson::base64::encode(str);
	}

	std::string Base64::Decode(const std::string& str)
	{
		return _bson::base64::decode(str);
	}
}// namespace Base64Helper