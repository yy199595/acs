
#pragma once

#include <string>
#include <stdint.h>

namespace WxBizDataSecure {

static const unsigned int kAesKeySize = 16;
static const unsigned int kAesIVSize = 16;
static const unsigned int kEncodingKeySize = 24;
static const unsigned int kMaxBase64Size = 1000000000;
enum  WxBizDataCryptErrorCode
{
    WXBizDataCrypt_OK = 0,
    WXBizDataCrypt_IllegalAesKey = -41001,
    WXBizDataCrypt_IllegalIv = -41002,
    WXBizDataCrypt_IllegalBuffer = -41003,
    WXBizDataCrypt_DecodeBase64_Error = -41004,
};

class WXBizDataCrypt
{
public:
    //���캯��
    // @param sSessionKey: 
    // @param sAppid: 

    WXBizDataCrypt(const std::string &sSessionkey )
					 :m_sSessionkey(sSessionkey)
					{   }
    
    
    // �������ݵ���ʵ�ԣ����һ�ȡ���ܺ������
    // @param sEncryptedData: 
    // @param sIv: 
    // @param sData: 
    // @return: �ɹ�0��ʧ�ܷ��ض�Ӧ�Ĵ�����
    int DecryptData(
			const std::string &sEncryptedData,
			const std::string &sIv,
			std::string &sData);

    private:
    std::string m_sSessionkey;

private:
    int AES_CBCDecrypt( const char * sSource, const uint32_t iSize,
            const char * sKey, uint32_t iKeySize, 
			const char * sIv, uint32_t iIvSize,
			std::string * poResult );
    
    int AES_CBCDecrypt( const std::string & objSource,
            const std::string & objKey, const std::string & sIv, 
			std::string * poResult );
    
    //base64
    int DecodeBase64(const std::string sSrc, std::string & sTarget);
    
};

}

