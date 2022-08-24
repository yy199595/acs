//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"Component/Mysql/MysqlRpcComponent.h"
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
        return Helper::Sha1::XorString(stage3, stage1);

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

        const int client_flags = 260047;

        return true;
    }
}
