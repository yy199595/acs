#include "SessionBase.h"
#include <Core/App.h>
#include <Method/MethodProxy.h>
#include <Pool/StringPool.h>
namespace Sentry
{

	SessionBase::SessionBase(ISocketHandler * handler)
		: mHandler(handler),
		mTaskScheduler(App::Get().GetTaskScheduler()),
		mContext(handler->GetNetThread()->GetContext())
	{
		this->mIsOpen = false;
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

    void SessionBase::InitMember()
    {
        this->mIsOpen = this->mSocket->is_open();
        this->mAddress = this->mSocket->remote_endpoint().address().to_string()
                               + ":" + std::to_string(this->mSocket->remote_endpoint().port());
    }

	void SessionBase::OnListenDone(const asio::error_code & err)
    {
#ifdef __DEBUG__
        if(!this->mHandler->GetNetThread()->IsCurrentThread())
        {
            SayNoDebugFatal("not in net thread call");
            return;
        }
#endif
        if(!err)
        {
            this->InitMember();
            this->OnSessionEnable();
        }	
        this->mTaskScheduler.AddMainTask(NewMethodProxy(&ISocketHandler::OnListenConnect, this->mHandler, this, err));
    }

	bool SessionBase::SendNetMessage(std::string * message)
	{
		if (message == nullptr || message->empty())
		{
			return false;
		}
#ifdef __DEBUG__
        if(!App::Get().IsMainThread())
        {
            SayNoDebugFatal("not in main thread call");
            return false;
        }
#endif
		this->mHandler->GetNetThread()->AddTask(&SessionBase::SendByString, this, message);
	}

	void SessionBase::SendByString(std::string *message)
    {
#ifdef __DEBUG__
        if(!this->mHandler->GetNetThread()->IsCurrentThread())
        {
            SayNoDebugFatal("not in net thread call");
            return;
        }
#endif
        if (!this->IsActive())
        {
            return;
        }
        this->mSocket->async_send(asio::buffer(message->c_str(), message->size()),
                                  [message, this](const asio::error_code &error_code, std::size_t size)
                                  {                                     
                                      this->OnSendByString(message, error_code);
                                  });
    }

	void SessionBase::OnSendByString(std::string * message, const asio::error_code &err)
    {
        this->mTaskScheduler.AddMainTask(
                NewMethodProxy(&ISocketHandler::OnSendMessage, this->mHandler,
                               this, message, err));
    }

	void SessionBase::OnClose()
	{	
		if (this->IsActive())
		{
			asio::error_code err;
            this->mIsOpen = false;
            this->mSocket->close(err);
		}
		SayNoDebugError("remove socket " << this->GetAddress());
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
