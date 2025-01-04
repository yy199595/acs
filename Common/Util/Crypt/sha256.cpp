#include "sha256.h"

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <iomanip>
#include <sstream>
#include <string>


#define BLOCKSIZE 64
#define SHA256_DIGEST_SIZE 32  // SHA-256 哈希值的字节大小

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

namespace help
{
	namespace Sha256
	{
		std::string BytesToHexString(const unsigned char* bytes, size_t length) {
			std::ostringstream oss;
			for (size_t i = 0; i < length; ++i) {
				oss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
			}
			return oss.str();
		}
	}
}

namespace help
{
	std::string Sha256::GetHash(const std::string& str)
	{
		return GetHash(str.c_str(), str.size());
	}

	std::string Sha256::GetHash(const char* data, size_t size)
	{
		unsigned char hash[SHA256_DIGEST_LENGTH];  // 使用 SHA-256 的长度
		SHA256_CTX sha256;  // 使用 SHA-256 的上下文
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, data, size);
		SHA256_Final(hash, &sha256);
		return Sha256::BytesToHexString(hash, SHA256_DIGEST_LENGTH);
	}

	std::string Sha256::GetHMacHash(const std::string& key, const std::string& text)
	{
		unsigned char* hmacResult;
		unsigned int resultLen;
		hmacResult = HMAC(EVP_sha256(), key.c_str(), key.size(), (unsigned char*)text.c_str(), text.size(), nullptr, &resultLen);
		return Sha256::BytesToHexString(hmacResult, resultLen);
	}

	std::string Sha256::XorString(const std::string& s1, const std::string& s2)
	{
		std::string output;
		for (int index = 0; index < s1.size(); index++)
		{
			char cc = s1[index] ^ s2[index % s2.size()];
			output += cc;
		}
		return output;
	}
}
