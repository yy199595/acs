//
// Created by leyi on 2024/2/28.
//
#ifdef __ENABLE_OPEN_SSL__
#ifndef APP_AES_H
#define APP_AES_H
#include <string>
namespace aes
{
	extern void Init();
	extern std::string Create(const std::string& payload, const std::string& secret);
	extern bool Verify(const std::string& jwt, const std::string& secret, std::string & payload);
	extern std::string Aes256GcmDecode(const std::string &ciphertext,
			const std::string &key,
			const std::string &iv,
			const std::string &aad);
}

#endif //APP_AES_H

#endif
