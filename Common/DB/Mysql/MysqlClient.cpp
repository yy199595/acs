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
        size_t pos = 0;
        std::unique_ptr<char[]> buffer(new char[len]);
        size_t size = readStream.readsome(buffer.get(), 100);
        while (size > 0)
        {
            pos += size;
            size = readStream.readsome(buffer.get() + pos, 100);
        }
        pos = 2;
        int protocol = buffer[0];
        std::string version(buffer.get() + pos);
        pos += version.size();
        int threadId = 0;
        memcpy(&threadId, buffer.get() + pos, sizeof(int));
        pos += 4;
        std::string scramble1(buffer.get() + pos, pos + 8 -1);
        pos += 9;

        return true;
    }
}
