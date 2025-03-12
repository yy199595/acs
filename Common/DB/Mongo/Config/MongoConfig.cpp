#include "MongoConfig.h"
#include <regex>
#include <fmt.h>
namespace mongo
{
	bool MongoConfig::FromString(const std::string& url)
	{
		std::regex re(R"(^(mongodb(?:\+srv)?://)(?:([^:]+)(?::([^@]+))?@)?([^:/]+)(?::(\d+))?(?:/([^?]+))?)");
		std::smatch matches;
		if (!std::regex_match(url, matches, re))
		{
			return false;
		}
		this->db = matches[6];
		this->user = matches[2];
		this->password = matches[3];
		const std::string ip = matches[4];
		const std::string port = matches[5];
		this->address = fmt::format("{}:{}", ip, port);
		if (this->user.empty())
		{
			this->password.clear();
		}
		return true;
	}
}