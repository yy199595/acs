
#ifdef __ENABLE_OPEN_SSL__
#include "Jwt.h"
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace jwt
{
	std::string base64_encode(const std::string& input)
	{
		BIO* bio, * b64;
		BUF_MEM* bufferPtr;
		bio = BIO_new(BIO_s_mem());
		b64 = BIO_new(BIO_f_base64());
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
		BIO_push(b64, bio);
		BIO_write(b64, input.c_str(), static_cast<int>(input.length()));
		BIO_flush(b64);
		BIO_get_mem_ptr(b64, &bufferPtr);
		std::string result(bufferPtr->data, bufferPtr->length);
		BIO_free_all(b64);
		return result;
	}

	std::string base64_decode(const std::string& input)
	{
		BIO* bio, * b64;
		char buffer[512];
		memset(buffer, 0, sizeof(buffer));
		b64 = BIO_new(BIO_f_base64());
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
		bio = BIO_new_mem_buf(input.c_str(), static_cast<int>(input.length()));
		bio = BIO_push(b64, bio);
		int length = BIO_read(bio, buffer, sizeof(buffer));
		BIO_free_all(b64);
		return std::string(buffer, length);
	}

	std::string hmac_sha256(const std::string& key, const std::string& data)
	{
		unsigned char result[EVP_MAX_MD_SIZE];
		unsigned int result_len;
		HMAC(EVP_sha256(), key.c_str(), static_cast<int>(key.length()),
				reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), result, &result_len);
		return std::string(reinterpret_cast<char*>(result), result_len);
	}
}

std::string jwt::Create(const std::string& payload, const std::string& secret)
{
	std::string header = "{}";
	std::string encoded_header = base64_encode(header);
	std::string encoded_payload = base64_encode(payload);
	std::string signature_input = encoded_header + "." + encoded_payload;
	std::string signature = hmac_sha256(secret, signature_input);
	return encoded_header + "." + encoded_payload + "." + base64_encode(signature);
}

bool jwt::Verify(const std::string& jwt, const std::string& secret, std::string& payload)
{
	size_t first_dot = jwt.find('.');
	size_t second_dot = jwt.find('.', first_dot + 1);
	if (first_dot == std::string::npos || second_dot == std::string::npos)
	{
		return false; // Invalid JWT format
	}

	std::string encoded_header = jwt.substr(0, first_dot);
	payload = jwt.substr(first_dot + 1, second_dot - first_dot - 1);
	std::string encoded_signature = jwt.substr(second_dot + 1);

	std::string signature_input = encoded_header + "." + payload;
	std::string calculated_signature = hmac_sha256(secret, signature_input);
	std::string decoded_signature = base64_decode(encoded_signature);

	payload = base64_decode(payload);
	return calculated_signature == decoded_signature;
}
#endif