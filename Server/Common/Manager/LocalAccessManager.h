#pragma once
#include"Manager.h"
namespace SoEasy
{
	// 本地访问管理  此组件可以访问本地action
	class CoroutineManager;
	class ActionManager;
	
	class LocalAccessManager : public Manager
	{
	public:
		LocalAccessManager() { }
		~LocalAccessManager() { }
	protected:
		bool OnInit() final;						//初始化管理器
		void OnSystemUpdate() final;					//处理系统事件
	public:
		bool CallService(const std::string & service, shared_ptr<NetWorkPacket> returnPackage);
	private:
		void HandleLocationRetAction(shared_ptr<NetWorkPacket> returnPackage);	//处理本地调用
		void HandleLocationCallAction(shared_ptr<NetWorkPacket> returnPackage);	//处理本地调用
	private:
		CoroutineManager * mCoroutineManager;
		ActionManager * mLocalActionManager;
		std::queue<shared_ptr<NetWorkPacket>> mLocalRetActionMessage; //本地返回消息缓存
		std::queue<shared_ptr<NetWorkPacket>> mLocalCallActionMessage; //本地调用消息缓存
	};
}