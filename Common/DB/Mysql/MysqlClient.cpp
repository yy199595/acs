//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"Component/Mysql/MysqlRpcComponent.h"
namespace Sentry
{
	MysqlClient::MysqlClient(std::shared_ptr<SocketProxy> socket,
                             MysqlConfig &config, MysqlRpcComponent *component)
		: Tcp::TcpContext(socket), mConfig(config)
	{
        this->mComponent = component;
        this->StartAuth();
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
        int protocol = readStream.get();
        Tcp::ReadStreamHelper helper(readStream);
        const std::string version = helper.ReadString();
        const int threadId = helper.ReadByType<int>();
        std::string scramble1 = helper.ReadString(7);

        return true;
    }
}
