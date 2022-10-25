//
// Created by yjz on 2022/10/24.
//

#include"SubPublish.h"
#include"Helper/Helper.h"
#include"Component/ForwardComponent.h"
namespace Sentry
{
	bool SubPublish::OnStart()
	{
		BIND_COMMON_RPC_METHOD(SubPublish::Sub);
		BIND_COMMON_RPC_METHOD(SubPublish::UnSub);
		BIND_COMMON_RPC_METHOD(SubPublish::Publish);
        this->mFrowardComponent = this->GetComponent<ForwardComponent>();
		return true;
	}

	bool SubPublish::OnClose()
	{
		return false;
	}

	XCode SubPublish::Sub(const Rpc::Head& head, const s2s::forward::sub& request)
    {
        std::string address;
        LOG_RPC_CHECK_ARGS(head.Get("resp", address));
        const ServiceNodeInfo * nodeInfo = this->mFrowardComponent->GetServerInfo(address);
        LOG_RPC_CHECK_ARGS(nodeInfo != nullptr && nodeInfo->LocationRpc.size() > 0);
        for (int index = 0; index < request.channels_size(); index++)
        {
            const std::string & channel = request.channels(index);
            auto iter = this->mChannels.find(address);
            if(iter == this->mChannels.end())
            {
                std::set<std::string> locations;
                this->mChannels.emplace(channel, locations);
            }
            this->mChannels[channel].insert(nodeInfo->LocationRpc);
        }
        return XCode::Successful;
    }

	XCode SubPublish::UnSub(const Rpc::Head& head, const s2s::forward::unsub& request)
	{
        std::string address;
        LOG_RPC_CHECK_ARGS(head.Get("resp", address));
        const ServiceNodeInfo * nodeInfo = this->mFrowardComponent->GetServerInfo(address);
        LOG_RPC_CHECK_ARGS(nodeInfo != nullptr && nodeInfo->LocationRpc.size() > 0);
        for (int index = 0; index < request.channels_size(); index++)
        {
            const std::string &channel = request.channels(index);
            auto iter = this->mChannels.find(channel);
            if(iter != this->mChannels.end())
            {
                auto iter1 = iter->second.find(nodeInfo->LocationRpc);
                if(iter1 != iter->second.end())
                {
                    iter->second.erase(iter1);
                    return XCode::Successful;
                }
            }
        }
		return XCode::Failure;
	}

	XCode SubPublish::Publish(const Rpc::Head& head, const s2s::forward::publish& request)
	{
        const std::string & channel = request.channel();
        const std::string & content = request.message();
        auto iter = this->mChannels.find(channel);
        if(iter == this->mChannels.end())
        {
            return XCode::Failure;
        }
        for(const std::string & address : iter->second)
        {
            std::shared_ptr<Rpc::Packet> message(new Rpc::Packet());
            {
                message->SetContent(content);
                message->SetType(Tcp::Type::Broadcast);
                message->GetHead().Add("channel", channel);
            }
            this->mFrowardComponent->Send(address, message);
        }
		return XCode::Successful;
	}
}