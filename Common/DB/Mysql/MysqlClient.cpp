//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"Component/Mysql/MysqlComponent.h"
namespace Sentry
{
	MysqlClient::MysqlClient(std::shared_ptr<SocketProxy> socket,
                             MysqlConfig &config, MysqlComponent *component)
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
        std::istream &readStream = this->GetReadStream();
        if (readStream.readsome((char *) &len, 3) <= 0)
        {
            return false;
        }
        if(this->RecvSync(len) <= 0)
        {
            return false;
        }
        int offset = 0;
        int threadId = 0;
        std::string version;
        long long randNum = 0;
        int index = readStream.get(); //序号
        int proto = readStream.get();
        char cc = readStream.get();
        while(cc != '\0')
        {
            version += cc;
            cc = readStream.get();
        }
        readStream.readsome((char *)&threadId, sizeof(int));
        readStream.readsome((char *)&randNum, sizeof(long long));
        std::string str;
        char buffer[100] = {0};
        size_t size = readStream.readsome(buffer, 100);
        while(size > 0)
        {
            str.append(buffer, size);
            size = readStream.readsome(buffer, 100);
        }
        return true;
    }
}
