#include "SessionBase.h"
#include <Core/App.h>
#include <Method/MethodProxy.h>
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

	void SessionBase::SetSocket(SharedTcpSocket socket)
	{		
		this->mSocket = socket;
		this->mIsConnected = false;
	}

	void SessionBase::StartReceive()
	{
		if (!this->IsActive())
		{
			return;
		}
		this->mHandler->GetNetThread()->AddTask(NewMethodProxy(&SessionBase::OnStartReceive, this));
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

	bool SessionBase::IsActive()
	{
		if (this->mSocket == nullptr)
		{
			return false;
		}
		return this->mSocket->is_open();
	}

	bool SessionBase::SendNetMessage(SharedMessage message)
	{
		if (message == nullptr || message->empty())
		{
			return false;
		}
		this->mHandler->GetNetThread()->AddTask(NewMethodProxy(&SessionBase::OnSendMessage, this, message));
	}

	void SessionBase::OnSendMessage(SharedMessage message)
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
				return;
			}
		});
	}

	void SessionBase::OnClose()
	{	
		if (this->IsActive())
		{
			asio::error_code err;
			this->mSocket->close(err);
		}
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
		SharedMessage message = make_shared<std::string>(msg, size);
		this->mTaskScheduler.AddMainTask(NewMethodProxy(&ISocketHandler::OnReceiveNewMessage, this->mHandler, this, message));
	}
}
