

#include "sha1.h"

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <iomanip>
#include <sstream>
#include <string>


#define BLOCKSIZE 64
#define SHA1_DIGEST_SIZE 20

#define    rol(value, bits) (((value) << (bits)) |    ((value) >>    (32    - (bits))))

namespace help
{
	namespace Sha1
	{
		std::string BytesToHexString(const unsigned char* bytes, size_t length)
		{
			std::ostringstream oss;
			for (size_t i = 0; i < length; ++i)
			{
				oss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
			}
			return oss.str();
		}
	}
}

namespace help
{
	std::string Sha1::GetHash(const std::string& str)
	{
		return GetHash(str.c_str(), str.size());
	}

	std::string Sha1::GetHash(const char* data, size_t size)
	{
		unsigned char hash[SHA_DIGEST_LENGTH];
		SHA_CTX sha1;
		SHA1_Init(&sha1);
		SHA1_Update(&sha1, data, size);
		SHA1_Final(hash, &sha1);
		return Sha1::BytesToHexString(hash, SHA_DIGEST_LENGTH);
	}

	void xor_key(unsigned char key[BLOCKSIZE], uint32_t xor1)
	{
		for (int i = 0; i < BLOCKSIZE; i += sizeof(uint32_t))
		{
			uint32_t* k = (uint32_t*)&key[i];
			*k ^= xor1;
		}
	}

	std::string Sha1::GetHMacHash(const std::string& key, const std::string& text)
	{
		unsigned char* hmacResult;
		unsigned int resultLen;
		hmacResult = HMAC(EVP_sha1(), key.c_str(), key.size(), (unsigned char*)text.c_str(), text.size(), nullptr, &resultLen);
		return Sha1::BytesToHexString(hmacResult, resultLen);
	}

	std::string Sha1::XorString(const std::string& s1, const std::string& s2)
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


