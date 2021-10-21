#include "SessionBase.h"
#include <Core/App.h>
#include <Method/MethodProxy.h>
#include <regex>
#include <Pool/StringPool.h>
namespace Sentry
{

	SessionBase::SessionBase(ISocketHandler * handler)
		: mHandler(handler),
		mTaskScheduler(App::Get().GetTaskScheduler()),
		mContext(handler->GetNetThread()->GetContext())
	{
		this->mIsOpen = false;
		this->mIsConnected = false;
        this->mSocket = make_shared<AsioTcpSocket>(this->mContext);
	}

	void SessionBase::Close()
	{
		if (!App::Get().IsMainThread())
		{
			this->OnClose();
			return;
		}
		this->mTaskScheduler.AddMainTask(NewMethodProxy(&SessionBase::OnClose, this));
	}

    void SessionBase::OnListenDone()
    {
        this->OnStartReceive();
        this->mIsConnected = false;
        this->mIsOpen = this->mSocket->is_open();
        this->mLocalAddress = this->mSocket->local_endpoint().address().to_string()
                              + ":" + std::to_string(this->mSocket->local_endpoint().port());

        this->mRemoteAddress = this->mSocket->remote_endpoint().address().to_string()
                               + ":" + std::to_string(this->mSocket->remote_endpoint().port());
    }

    void SessionBase::StartConnect(std::string name, std::string ip, unsigned short port)
	{
		if (ip.empty())
		{
			return;
		}
		this->mIsConnected = true;
		this->mHandler->GetNetThread()->AddTask(NewMethodProxy(&SessionBase::OnStartConnect, this, name, ip, port));
	}

    void SessionBase::OnStartConnect(std::string name, std::string host, unsigned short port)
    {
        if (this->mSocket->is_open())
        {
            return;
        }
        this->mName = name;
        this->mConnectCount++;

        std::regex regExpress(
                R"((?=(\b|\D))(((\d{1,2})|(1\d{1,2})|(2[0-4]\d)|(25[0-5]))\.){3}((\d{1,2})|(1\d{1,2})|(2[0-4]\d)|(25[0-5]))(?=(\b|\D)))");
        if (std::regex_match(host, regExpress))
        {
            auto address = asio::ip::make_address_v4(host);
            asio::ip::tcp::endpoint endPoint(address, port);
            this->mSocket->async_connect(endPoint, std::bind(&SessionBase::ConnectHandler, this, args1));
            SayNoDebugLog(this->mName << " start connect " << host << port);
        }
        else
        {
            asio::ip::tcp::resolver resolver(this->GetContext());
            SayNoDebugLog("start connect host " << host << ":" << port);
            asio::ip::tcp::resolver::query query(host, std::to_string(port));
            resolver.async_resolve(query, [this](const asio::error_code &err, tcp::resolver::iterator iterator)
            {
                if (err)
                {
                    this->OnClose();
                    this->OnConnect(err);
                    SayNoDebugError(err.message());
                    return;
                }
                else
                {
                    asio::async_connect(this->GetSocket(), iterator,
                                        std::bind(&SessionBase::ConnectHandler, this, args1));
                }
            });
        }
    }

    void SessionBase::ConnectHandler(const asio::error_code & err)
    {
        if(err)
        {
            this->OnClose();
            return;
        }
        this->OnConnect(err);
        this->OnStartReceive();
        this->mConnectCount = 0;
        this->mIsOpen = this->mSocket->is_open();
        this->mLocalAddress = this->mSocket->local_endpoint().address().to_string()
                              + ":" + std::to_string(this->mSocket->local_endpoint().port());

        this->mRemoteAddress = this->mSocket->remote_endpoint().address().to_string()
                               + ":" + std::to_string(this->mSocket->remote_endpoint().port());
    }

	bool SessionBase::SendNetMessage(std::string * message)
	{
		if (message == nullptr || message->empty())
		{
			return false;
		}
		this->mHandler->GetNetThread()->AddTask(NewMethodProxy(&SessionBase::OnSendMessage, this, message));
	}

	void SessionBase::OnSendMessage(std::string * message)
    {
        if (!this->IsActive())
        {
            return;
        }
        this->mSocket->async_send(asio::buffer(message->c_str(), message->size()),
                                  [message, this](const asio::error_code &error_code, std::size_t size)
                                  {
                                      if (error_code)
                                      {
                                          this->OnError(error_code);
                                      }
                                      this->mTaskScheduler.AddMainTask(
                                              NewMethodProxy(&ISocketHandler::OnSendMessage, this->mHandler,
                                                             this, message, error_code));
                                  });
    }

	void SessionBase::OnClose()
	{	
		if (this->IsActive())
		{
			asio::error_code err;
            this->mIsOpen = false;
            this->mSocket->close(err);
		}
		SayNoDebugError("remove socket " << this->GetRemoteAddress());
		this->mTaskScheduler.AddMainTask(NewMethodProxy(&ISocketHandler::OnClose, this->mHandler, this));
	}

	void SessionBase::OnError(const asio::error_code & err)
	{
		this->mTaskScheduler.AddMainTask(NewMethodProxy(&ISocketHandler::OnSessionErr, this->mHandler, this, err));
	}

	void SessionBase::OnConnect(const asio::error_code & err)
	{
		this->mTaskScheduler.AddMainTask(NewMethodProxy(&ISocketHandler::OnConnectRemote, this->mHandler, this, err));
	}

	void SessionBase::OnReceiveMessage(const char * msg, size_t size)
    {
        if (msg == nullptr || size == 0)
        {
            return;
        }
        std::string *message = this->mHandler->GetStringPool().New(msg, size);
        this->mTaskScheduler.AddMainTask(NewMethodProxy(&ISocketHandler::OnReceiveNewMessage, this->mHandler, this, message));
    }
}
