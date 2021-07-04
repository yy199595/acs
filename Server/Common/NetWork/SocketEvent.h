#pragma once
#include<string>
#include<Protocol/com.pb.h>
namespace SoEasy
{
	enum Net2MainEventType
	{
		SocketNewConnect,  //新连接  //网络->逻辑
		SocketConnectSuc,	//连接成功 //网络->逻辑
		SocketConnectFail,	//连接失败 //网络->逻辑
		SocketReceiveData,	//接收到新消息
		SocketReceiveFail,	//接收消息失败 //网络->逻辑
		SocketSendMsgFail,	//发送消息失败 //网络->逻辑
	};
	enum Main2NetEventType
	{
		SocketConnectEvent,	//连接某个地址事件
		SocketDectoryEvent,	//销毁连接事件
		SocketSendMsgEvent	//发送消息事件
	};

	enum SessionType
	{
		SessionNoneType,
		SessionClientType,
		SessionNodeType
	};

	// 网络->逻辑事件
	template<typename T>
	class SocketEvent
	{
	public:
		SocketEvent(T type, const std::string & address, const std::string name = "", PB::NetWorkPacket * msg = nullptr)
			:mEventType(type),mName(name), mAddress(address), mMessageData(msg) {}
	public:
		T GetEventType() { return this->mEventType; }
		const std::string & GetName() { return this->mName; }
		const std::string & GetAddress() { return this->mAddress; }		
		PB::NetWorkPacket * GetMsgData() { return this->mMessageData; }
	private:
		T mEventType;
		std::string mName;
		const std::string mAddress;
		PB::NetWorkPacket * mMessageData;
	};

	typedef SocketEvent<Net2MainEventType> Net2MainEvent; //网络到逻辑事件
	typedef SocketEvent<Main2NetEventType> Main2NetEvent; //逻辑到网络事件
}