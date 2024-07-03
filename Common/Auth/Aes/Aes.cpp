//
// Created by leyi on 2024/2/28.
//

#ifdef __ENABLE_OPEN_SSL__
#include "Aes.h"

#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <cstring>
#include "Proto/Bson/base64.h"
// 加密密钥长度
#define KEY_LENGTH 32

// 使用的加密算法
#define CIPHER EVP_aes_256_gcm()

// 初始化 OpenSSL

void aes::Init()
{
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	ERR_load_ERR_strings();
	OPENSSL_config(NULL);
}

// 加密数据
std::string aes::Create(const std::string& data, const std::string& key)
{
	EVP_CIPHER_CTX *ctx;
	unsigned char iv[EVP_MAX_IV_LENGTH];
	unsigned char *outbuf;
	int outlen;
	int len;
	std::string encrypted_data;


	// 初始化加密上下文
	ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		return "";
	}

	// 生成随机初始化向量
	if (!RAND_bytes(iv, EVP_MAX_IV_LENGTH)) {
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	// 初始化加密
	if (!EVP_EncryptInit_ex(ctx, CIPHER, NULL, reinterpret_cast<const unsigned char*>(key.c_str()), iv)) {
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	// 分配输出缓冲区
	outbuf = new unsigned char[data.size() + EVP_CIPHER_block_size(CIPHER)];

	// 加密数据
	if (!EVP_EncryptUpdate(ctx, outbuf, &outlen, reinterpret_cast<const unsigned char*>(data.c_str()), data.size())) {
		delete[] outbuf;
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	len = outlen;

	// 结束加密
	if (!EVP_EncryptFinal_ex(ctx, outbuf + len, &outlen)) {
		delete[] outbuf;
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	len += outlen;

	// 构造加密后的数据
	encrypted_data.assign(reinterpret_cast<const char*>(iv), EVP_MAX_IV_LENGTH);
	encrypted_data.append(reinterpret_cast<const char*>(outbuf), len);

	// 释放资源
	delete[] outbuf;
	EVP_CIPHER_CTX_free(ctx);
	return _bson::base64::encode(encrypted_data);
}

bool aes::Verify(const std::string& text, const std::string& key, std::string& payload)
{
	EVP_CIPHER_CTX* ctx;
	unsigned char iv[EVP_MAX_IV_LENGTH];
	unsigned char* outbuf;
	int outlen;
	int len;
	std::string decrypted_data;
	std::string ciphertext = _bson::base64::decode(text);

	// 提取初始化向量
	memcpy(iv, ciphertext.c_str(), EVP_MAX_IV_LENGTH);

	// 初始化解密上下文
	ctx = EVP_CIPHER_CTX_new();
	if (!ctx)
	{
		return false;
	}

	// 初始化解密
	if (!EVP_DecryptInit_ex(ctx, CIPHER, NULL, reinterpret_cast<const unsigned char*>(key.c_str()), iv))
	{
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}

	// 分配输出缓冲区
	outbuf = new unsigned char[ciphertext.size() - EVP_MAX_IV_LENGTH];
	// 解密数据
	if (!EVP_DecryptUpdate(ctx, outbuf, &outlen,
			reinterpret_cast<const unsigned char*>(ciphertext.c_str()) + EVP_MAX_IV_LENGTH,
			ciphertext.size() - EVP_MAX_IV_LENGTH))
	{
		delete[] outbuf;
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}

	len = outlen;

	// 结束解密
	if (!EVP_DecryptFinal_ex(ctx, outbuf + len, &outlen))
	{
		delete[] outbuf;
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}

	len += outlen;

	// 构造解密后的数据
	payload.assign(reinterpret_cast<const char*>(outbuf), len);

	// 释放资源
	delete[] outbuf;
	EVP_CIPHER_CTX_free(ctx);
	return true;
}

std::string aes::Aes256GcmDecode(const std::string &ciphertext,
		const std::string &key,
		const std::string &iv,
		const std::string &aad)
{
	EVP_CIPHER_CTX *ctx;
	int len, plaintext_len;

	std::string plaintext;
	plaintext.resize(ciphertext.size() - EVP_GCM_TLS_TAG_LEN);

	// 创建并初始化解密上下文
	if (!(ctx = EVP_CIPHER_CTX_new())) {
		return "";
	}

	// 初始化解密器并设置密钥、IV和操作模式
	if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1
		|| EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL) != 1
		|| EVP_DecryptInit_ex(ctx, NULL, NULL, reinterpret_cast<const unsigned char *>(key.c_str()), reinterpret_cast<const unsigned char *>(iv.c_str())) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	// 添加附加的认证数据（AAD）
	if (!aad.empty() && EVP_DecryptUpdate(ctx, NULL, &len, reinterpret_cast<const unsigned char *>(aad.c_str()), aad.size()) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	// 解密数据
	if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(&plaintext[0]), &len, reinterpret_cast<const unsigned char *>(ciphertext.c_str()), ciphertext.size() - EVP_GCM_TLS_TAG_LEN) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}
	plaintext_len = len;

	// 设置认证标签
	if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, EVP_GCM_TLS_TAG_LEN, reinterpret_cast<unsigned char *>(const_cast<char *>(ciphertext.c_str() + ciphertext.size() - EVP_GCM_TLS_TAG_LEN))) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}

	// 完成解密操作
	if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(&plaintext[0]) + len, &len) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		return "";
	}
	plaintext_len += len;

	// 清理解密上下文
	EVP_CIPHER_CTX_free(ctx);

	// 返回明文
	plaintext.resize(plaintext_len);
	return plaintext;
}
#endif