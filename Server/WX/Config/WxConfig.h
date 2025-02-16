//
// Created by leyi on 2024/2/19.
//

#ifndef APP_WXCONFIG_H
#define APP_WXCONFIG_H
#include<string>
#include "Yyjson/Object/JsonObject.h"
namespace wx
{
	struct LoginConfig
	{
		//微信登录
		std::string appId;
		std::string secret; //密钥
	};

	struct PayConfig
	{
		std::string mchId; //商户号
		std::string apiKey; //商户私钥
		std::string apiCert;	//商户证书
		std::string apiV3key;
		std::string notifyUrl; //回调url
		std::string complaintUrl; //投诉回调url
		std::string tradeType; //支付类型
		std::string mchNumber;	//商户证书序列号
		std::string apiKeyPath;
		std::string apiCertPath;
		std::string publicKeyPath;
		std::string certNumber;	//证书序列号
	};

	struct Config
	{
		PayConfig pay;
		LoginConfig login;
	};
}

#endif //APP_WXCONFIG_H
