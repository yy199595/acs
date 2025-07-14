//
// Created by 64658 on 2024/10/24.
//

#include "XCode/XCode.h"
#include "Util/Tools/String.h"
#include "Message/c2s/c2s.pb.h"
#include "UdpComponent.h"
#include "Rpc/Common/Message.h"
#include "Entity/Actor/App.h"
#include "Server/Config/CodeConfig.h"
#include "Server/Component/ThreadComponent.h"
#include "Rpc/Component/DispatchComponent.h"
#include "Util/Tools/TimeHelper.h"

namespace acs
{
	UdpComponent::UdpComponent()
		: mSendStream(&mSendBuffer), mReadStream(&mReceiveBuffer)
	{
		this->mActor = nullptr;
		this->mDispatch = nullptr;
		this->mMsg = rpc::msg::bin;
		this->mAddrName = rpc::Header::from_addr;
	}

	bool UdpComponent::LateAwake()
	{
		this->mActor = this->GetComponent<NodeComponent>();
		this->mDispatch = this->GetComponent<DispatchComponent>();
		return true;
	}

	bool UdpComponent::StopListen()
	{
		Asio::Code code;
		code = this->mSocket->close(code);
		return code.value() == Asio::OK;
	}

	bool UdpComponent::StartListen(const acs::ListenConfig& listen)
	{
		try
		{
			Asio::Context& context = this->mApp->GetContext();
			{
				unsigned short port = listen.port;
				udp::EndPoint endpoint(asio::ip::udp::v4(), port);
				this->mSocket = std::make_unique<udp::Socket>(context, endpoint);
			}
			asio::post(context, [this]() { this->StartReceive(); });
			return true;
		}
		catch (std::exception& e)
		{
			return false;
		}
	}

	void UdpComponent::StartReceive()
	{
		auto callback = [this](const asio::error_code& code, size_t size)
		{
			if (code.value() == Asio::OK)
			{
				this->OnReceiveMessage(size);
			}
			if (code != asio::error::operation_aborted)
			{
				this->StartReceive();
			}
		};
		auto buffer = this->mReceiveBuffer.prepare(udp::BUFFER_COUNT);
		this->mSocket->async_receive_from(buffer, this->mRemotePoint, callback);
	}

	void UdpComponent::OnReceiveMessage(size_t size)
	{
		this->mReceiveBuffer.commit(size);
		std::unique_ptr<rpc::Message> rpcPacket = std::make_unique<rpc::Message>();
		{
			rpc::ProtoHead & protoHead = rpcPacket->GetProtoHead();
			{
				rpcPacket->SetMsg(this->mMsg);
				tcp::Data::ReadHead(this->mReadStream, protoHead, true);
			}
			rpcPacket->Init(protoHead);
			size_t count = size - rpc::RPC_PACK_HEAD_LEN;
			if (rpcPacket->OnRecvMessage(this->mReadStream, count) != tcp::read::done)
			{
				return;
			}
			unsigned short port = this->mRemotePoint.port();
			std::string ip = this->mRemotePoint.address().to_string();
			const std::string address = fmt::format("{}:{}", ip, port);
#ifdef __DEBUG__
			rpcPacket->TempHead().Add(rpc::Header::from_addr, address);
#endif
			rpcPacket->SetNet(rpc::net::udp);
			Asio::Context& context = this->mApp->GetContext();
			rpcPacket->TempHead().Add(this->mAddrName, address);
			asio::post(context, [this, req = rpcPacket.release()]{ this->OnMessage(req, nullptr); });
		}
		this->mReceiveBuffer.consume(size);
	}

	int UdpComponent::Send(int id, std::unique_ptr<rpc::Message>& message) noexcept
	{
		std::string address;
		switch(message->GetType())
		{
			case rpc::type::request:
			{
				if(!this->mActor->GetListen(id, "udp", address))
				{
					return XCode::NotFoundActor;
				}
				break;
			}
			case rpc::type::response:
			{
				const rpc::Head & head = message->TempHead();
				if(!head.Get(this->mAddrName, address))
				{
					LOG_ERROR("not find udp address => {}", message->ToString())
					return XCode::SendMessageFail;
				}
				break;
			}
		}
		return this->Send(address, message);
	}

	int UdpComponent::Send(const std::string& address, std::unique_ptr<rpc::Message>& message)
	{
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(address, ip, port))
		{
			return XCode::SendMessageFail;
		}
		Asio::Code sendCode;
		message->SetMsg(this->mMsg);
		asio::socket_base::message_flags flags = 0;
		int length = message->OnSendMessage(this->mSendStream);
		asio::ip::udp::endpoint endpoint(asio::ip::make_address(ip), port);
		size_t count = this->mSocket->send_to(this->mSendBuffer.data(), endpoint, flags, sendCode);
		if(sendCode.value() != XCode::Ok)
		{
			return XCode::SendMessageFail;
		}
		this->mSendBuffer.consume(count);
		return XCode::Ok;
	}

	void UdpComponent::OnMessage(rpc::Message* request, rpc::Message* ) noexcept
	{
		std::unique_ptr<rpc::Message> message(request);
		switch(request->GetType())
		{
			case rpc::type::request:
				this->OnRequest(message);
				break;
			case rpc::type::response:
				this->mDispatch->OnMessage(message);
				break;
			default:
				break;
		}
	}

	int UdpComponent::OnRequest(std::unique_ptr<rpc::Message>& message) noexcept
	{
		int code = this->mDispatch->OnMessage(message);
		if (code != XCode::Ok)
		{
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			LOG_ERROR("call {} code = {}", message->GetHead().GetStr(rpc::Header::func), desc);

			if (message->GetRpcId() == 0)
			{
				return XCode::Failure;
			}
			message->Body()->clear();
			message->SetType(rpc::type::response);
			message->GetHead().Add(rpc::Header::code, code);
			return this->Send(message->SockId(), message);
		}
		return XCode::Ok;
	}
}
