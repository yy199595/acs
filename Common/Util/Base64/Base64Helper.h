#pragma once
#include <string>
namespace Helper
{
    namespace Base64
    {
		extern bool IsBase64(const std::string & str);

		extern bool IsBase64(const char * str, size_t size);

		extern std::string Base64Encode(const char *data, const size_t size);

        extern std::string Base64Encode(const std::string &str);

        extern std::string Base64Decode(const char *data, const size_t size);

        extern std::string Base64Decode(const std::string &str);
    }// namespace Base64Helper
}