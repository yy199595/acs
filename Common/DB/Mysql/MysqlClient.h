//
// Created by mac on 2022/4/14.
//

#ifndef SERVER_MYSQLCLIENT_H
#define SERVER_MYSQLCLIENT_H

#include"MysqlDefine.h"
#include"Network/TcpContext.h"
namespace Sentry
{
    class MysqlRpcComponent;
    class MysqlClient : public Tcp::TcpContext
	{
	 public:
		MysqlClient(std::shared_ptr<SocketProxy> socket,
                    MysqlConfig& config, MysqlRpcComponent * component);
    private:
        bool StartAuth();
        std::string ComputeToken(const std::string & password, const std::string & scramble);
	 private:
		const MysqlConfig& mConfig;
        MysqlRpcComponent * mComponent;
	};
}
#endif //SERVER_MYSQLCLIENT_H
