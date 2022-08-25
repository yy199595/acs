//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"Component/Mysql/MysqlRpcComponent.h"
#include"MysqlMessage.h"
#include"Util/sha1.h"
namespace Sentry
{
	MysqlClient::MysqlClient(std::shared_ptr<SocketProxy> socket,
                             MysqlConfig &config, MysqlRpcComponent *component)
		: Tcp::TcpContext(socket), mConfig(config)
	{
        this->mComponent = component;
        this->StartAuth();
	}

    std::string MysqlClient::ComputeToken(const std::string &password, const std::string &scramble)
    {
        if(password.empty())
        {
            return std::string();
        }
        std::string stage1 = Helper::Sha1::GetHash(password);
        std::string stage2 = Helper::Sha1::GetHash(stage1);
        std::string stage3 = Helper::Sha1::GetHash(scramble + stage2);
        std::cout << stage3 << std::endl << stage1 << std::endl;

        size_t pos = stage3.find('.');
        if(pos != std::string::npos)
        {
            for(size_t index =0 ;index<stage3.size(); index++)
            {
                unsigned char v1 = stage3[index];
                unsigned char v2 = stage1[index];
            }
        }
        return stage3;

    }

    bool MysqlClient::StartAuth()
    {
        if (!this->ConnectSync())
        {
            return false;
        }
        if (this->RecvSync(3) <= 0)
        {
            return false;
        }
        int len = 0;
        std::istream readStream(&this->mRecvBuffer);
        if (readStream.readsome((char *) &len, 3) <= 0)
        {
            return false;
        }
        if (this->RecvSync(len) <= 0)
        {
            return false;
        }
        int index = readStream.get();
        int protocol = readStream.get();
        Tcp::ReadStreamHelper helper(readStream);
        const std::string version = helper.ReadString();
        const int threadId = helper.ReadByType<int>();
        int size = 1 + version.size() + 1 + sizeof(int);
        std::string scramble1 = helper.ReadString(8);
        readStream.get();

        unsigned int _server_capabilities = helper.ReadByType<unsigned short>();
        unsigned int _server_lang = readStream.get();

        unsigned int _server_status = helper.ReadByType<unsigned short>();
        unsigned int more_capabilities = helper.ReadByType<unsigned short>();
        _server_capabilities = _server_capabilities | more_capabilities << 16;

        std::string scramble_part2 = helper.ReadString(12);

        std::string scramble = scramble1 + scramble_part2;

        std::string token = this->ComputeToken(this->mConfig.mPassword, scramble);

        const unsigned int client_flags = 260047;
        const unsigned int maxPackCount = 1024 * 1024;

        std::shared_ptr<Tcp::MysqlMessage> message(new Tcp::MysqlMessage(1));

        char rep[23] = { 0};
        memset(rep, '\0', 23);
        message->Add(client_flags);
        message->Add(maxPackCount);
        message->Add('_');
        message->Add(rep, 23);
        message->Add(this->mConfig.mUser.c_str());
        message->Add(token.c_str());
        message->Add("");
        message->Add('\0');
        size_t s = token.size();
        if(this->SendSync(message) <= 0)
        {
            return false;
        }
        this->RecvSync(3);
        if (readStream.readsome((char *) &len, 3) <= 0)
        {
            return false;
        }
        if (this->RecvSync(len) <= 0)
        {
            return false;
        }

        return true;
    }
}
