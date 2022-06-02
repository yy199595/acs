//
// Created by zmhy0073 on 2022/6/2.
//

#include"MessageComponent.h"
#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include<google/protobuf/util/json_util.h>
#include<google/protobuf/dynamic_message.h>
#include"Script/Extension/Message/Message.h"
namespace Sentry
{
    ImportError::ImportError()
        : mHasError(false), mHasWarning(false)
    {

    }
    void ImportError::AddError(const std::string &filename, int line, int column, const std::string &message)
    {
        this->mHasError = true;
        LOG_ERROR(filename << ":" << line << "  " << message);
    }

    void ImportError::AddWarning(const std::string &filename, int line, int column, const std::string &message)
    {
        this->mHasWarning = true;
        LOG_WARN(filename << ":" << line << "  " << message);
    }
}

namespace Sentry
{
    bool MessageComponent::Load(const std::string path, std::vector<std::string> protos)
    {
        ImportError importError;
        compiler::DiskSourceTree sourceTree;
        sourceTree.MapPath("", path);
        compiler::Importer importer(&sourceTree, &importError);
        if (importError.HasError())
        {
            LOG_ERROR("load proto message [" << path << "] error");
            return false;
        }

        for (const std::string &fileName: protos)
        {
            const FileDescriptor *fileDescriptor = importer.Import(fileName);
            if (fileDescriptor == nullptr)
            {
                LOG_ERROR("import [" << fileName << "] error");
                return false;
            }
            LOG_INFO("import [" << fileName << "] successful");
            this->mFileDescripors.emplace(fileName, fileDescriptor);
        }
        return true;
    }

    std::shared_ptr<Message> MessageComponent::New(const Any &any)
    {
        std::string fullName;
        if(!Any::ParseAnyTypeUrl(any.type_url(), &fullName))
        {
            return nullptr;
        }
        std::shared_ptr<Message> message = this->New(fullName);
        if(message != nullptr)
        {
            message->CopyFrom(any);
            return message;
        }
        return nullptr;
    }

    std::shared_ptr<Message> MessageComponent::New(const std::string & name)
    {
        const Message * message = this->FindMessage(name);
        if(message == nullptr)
        {
            LOG_ERROR("find protobuf message [" << name << "] error");
            return nullptr;
        }
        return std::shared_ptr<Message>(message->New());
    }

    std::shared_ptr<Message> MessageComponent::New(const std::string &name, const std::string &json)
    {
        std::shared_ptr<Message> message = this->New(name);
        if(message != nullptr && util::JsonStringToMessage(json, message.get()).ok())
        {
            return message;
        }
        return nullptr;
    }

    const Message * MessageComponent::FindMessage(const std::string &name)
    {
        auto iter1 = this->mMessageMap.find(name);
        if(iter1 != this->mMessageMap.end())
        {
            return iter1->second;
        }

        //静态查找
        const DescriptorPool* pDescriptorPool = DescriptorPool::generated_pool();
        const Descriptor* pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
        if(pDescriptor != nullptr)
        {
            MessageFactory *factory = MessageFactory::generated_factory();
            const Message *pMessage = factory->GetPrototype(pDescriptor);
            if(pMessage != nullptr)
            {
                this->mMessageMap.emplace(name, pMessage);
                return pMessage;
            }
            return nullptr;
        }

        //动态查找
        auto iter = this->mFileDescripors.begin();
        for(; iter != this->mFileDescripors.end(); iter++)
        {
            const FileDescriptor * fileDescriptor = iter->second;
            const Descriptor * descriptor = fileDescriptor->FindMessageTypeByName(name);
            if(descriptor != nullptr)
            {
                DynamicMessageFactory factory(fileDescriptor->pool());
                const Message * message = factory.GetPrototype(descriptor);
                if(message != nullptr)
                {
                    this->mMessageMap.emplace(name, message);
                    return message;
                }
            }
        }
        return nullptr;
    }

    void MessageComponent::OnLuaRegister(Lua::ClassProxyHelper &luaRegister)
    {
        luaRegister.BeginRegister<MessageComponent>();
        luaRegister.PushMemberFunction("Load", &MessageComponent::Load);
        luaRegister.PushExtensionFunction("New", Lua::MessageEx::New);
    }
}