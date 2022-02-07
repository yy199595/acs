
#include"TelnetClient.h"
#include"Util/StringHelper.h"
#include <Define/CommonLogDef.h>
namespace Sentry
{
	TelnetClient::TelnetClient(std::shared_ptr<SocketProxy> socketProxy)
	{
        this->mSocket = socketProxy;
	}

    std::shared_ptr<TelnetContent> TelnetClient::ReadCommand()
    {
        std::shared_ptr<TelnetContent> telnetContent(new TelnetContent());
        std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());
#ifdef ONLY_MAIN_THREAD
        this->ReadData(taskSource, telnetContent);
#else
        IAsioThread & netWorkThread = this->mSocket->GetThread();
        netWorkThread.Invoke(&TelnetClient::ReadData, this, taskSource, telnetContent);
#endif

        telnetContent->IsOk = taskSource->Await();
        return telnetContent;
    }

    bool TelnetClient::Response(const std::string &content)
    {
        std::ostream os(&this->mSendBuffer);
        os << content << "\r\n";
        std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());
#ifdef ONLY_MAIN_THREAD
        this->ResponseData(taskSource);
#else
        IAsioThread &netWorkThread = this->mSocket->GetThread();
        netWorkThread.Invoke(&TelnetClient::ResponseData, this, taskSource);
#endif
        return taskSource->Await();
    }

    void TelnetClient::ResponseData(std::shared_ptr<TaskSource<bool>> taskSource)
    {
        AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
        asio::async_write(tcpSocket, this->mSendBuffer,[taskSource, this]
                          (const asio::error_code & code, size_t size)
        {
            if(code)
            {
                this->mSocket->Close();
                STD_ERROR_LOG(code.message());
                taskSource->SetResult(false);
                return;
            }
            taskSource->SetResult(true);
        });
    }

    void TelnetClient::ReadData(std::shared_ptr<TaskSource<bool>> taskSource, std::shared_ptr<TelnetContent> content)
    {
        AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
        asio::async_read(tcpSocket, this->mReceiveBuffer,asio::transfer_at_least(1),
                         [this, taskSource, content] (const asio::error_code &code, size_t size)
                         {
                             if (code)
                             {
                                 this->mSocket->Close();
                                 STD_ERROR_LOG(code.message());
                                 taskSource->SetResult(false);
                                 return;
                             }
                             std::string lineData;
                             std::iostream iostream1(&this->mReceiveBuffer);
                             if(std::getline(iostream1, lineData))
                             {
                                 lineData.pop_back();
                                 size_t pos = lineData.find(' ');
                                 content->Command = lineData;
                                 if(pos != std::string::npos)
                                 {
                                     content->Command = lineData.substr(0, pos);
                                     content->Paramater = lineData.substr(pos +1 );
                                 }
                                 taskSource->SetResult(true);
                                 return;
                             }
                             this->ReadData(taskSource, content);
                         });
    }
}

