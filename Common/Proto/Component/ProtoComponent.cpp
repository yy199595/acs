//
// Created by zmhy0073 on 2022/6/2.
//

#include"ProtoComponent.h"
#include"Entity/App/App.h"
#include<fstream>
#include"Util/Md5/MD5.h"
#include"Util/File/DirectoryHelper.h"
#include"google/protobuf/util/json_util.h"
#include"google/protobuf/dynamic_message.h"
#include"Proto/Lua/Message.h"
#include"Script/Lua/ClassProxyHelper.h"
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
	ProtoComponent::ProtoComponent()
	{
		this->mImporter = nullptr;
		this->mSourceTree = nullptr;
	}

    bool ProtoComponent::LateAwake()
    {
        std::string path;
        const ServerConfig * config = ServerConfig::Inst();
		if(config->GetPath("proto", path))
        {
            return this->Load(path.c_str());
        }
        return true;
    }

    bool ProtoComponent::Load(const char * path)
    {
        if(!Helper::Directory::DirectorIsExist(path))
        {
            LOG_ERROR("director [" << path << "] not exist");
            return false;
        }
		this->mDynamicMessageMap.clear();
		this->mDynamicMessageFactorys.clear();
		this->mDynamicMessageFactory = nullptr;
		this->mImportError = std::make_unique<ImportError>();
		this->mSourceTree = std::make_unique<compiler::DiskSourceTree>();
        this->mSourceTree->MapPath("", path);
        this->mImporter = std::make_unique<compiler::Importer>(
                this->mSourceTree.get(), this->mImportError.get());
        if (this->mImportError->HasError())
        {
            LOG_ERROR("load proto message [" << path << "] error");
            return false;
        }
        LOG_INFO("load proto path : " << path);
        return true;
    }

	bool ProtoComponent::Import(const char * fileName)
	{
        io::ZeroCopyInputStream * inputStream = this->mSourceTree->Open(fileName);
        if(inputStream == nullptr)
        {
            LOG_ERROR("not proto file [" << fileName << "]");
            return false;
        }
        std::string path;
        ServerConfig::Inst()->GetPath("proto", path);
        const std::string fullPath(fmt::format("{0}/{1}", path, fileName));


        std::ifstream fs(fullPath);
		auto iter = this->mFiles.find(fileName);
		const std::string md5 = Helper::Md5::GetMd5(fs);
        if(iter != this->mFiles.end() && iter->second == md5)
		{
			return true;
		}
        this->mFiles[fileName] = md5;
        const FileDescriptor * fileDescriptor = this->mImporter->Import(fileName);
		if (fileDescriptor == nullptr)
		{
			LOG_ERROR("import [" << fileName << "] error");
			return false;
		}
		LOG_INFO("import [" << fileName << "] successful");
		this->mDynamicMessageFactory = std::make_unique<DynamicMessageFactory>(fileDescriptor->pool());
		for(int x = 0; x < fileDescriptor->message_type_count(); x++)
		{
			const Descriptor * descriptor = fileDescriptor->message_type(x);
			this->LoopMessage(descriptor);
			if(descriptor->field_count() > 0)
			{
				const Message * message = this->mDynamicMessageFactory->GetPrototype(descriptor);
                LOG_DEBUG("add new dynamic message " <<  message->GetTypeName());
                this->mDynamicMessageMap.emplace(message->GetTypeName(), message);
			}
		}
		this->mDynamicMessageFactorys.emplace_back(std::move(mDynamicMessageFactory));
		return true;
	}

	void ProtoComponent::LoopMessage(const Descriptor* descriptor)
	{
		for(int y = 0; y < descriptor->nested_type_count(); y++)
		{
			const Descriptor * descriptor1 = descriptor->nested_type(y);
			this->LoopMessage(descriptor1);
			if(descriptor1->field_count() > 0)
			{
				const Message * message = this->mDynamicMessageFactory->GetPrototype(descriptor1);
				LOG_DEBUG("add new dynamic message " <<  message->GetTypeName());
				this->mDynamicMessageMap.emplace(message->GetTypeName(), message);
			}
		}
	}

    bool ProtoComponent::New(const Any &any, std::shared_ptr<Message> & message)
    {
        std::string fullName;
        if(!Any::ParseAnyTypeUrl(any.type_url(), &fullName))
        {
            return false;
        }
        if(!this->New(fullName, message))
		{
			return false;
		}
		return any.UnpackTo(message.get());
    }

    bool ProtoComponent::New(const std::string & name, std::shared_ptr<Message> & message)
    {
        const Message * tmpMessage = this->FindMessage(name);
        if(tmpMessage == nullptr)
        {
            LOG_ERROR("find protobuf message [" << name << "] error");
            return false;
        }
        message = std::shared_ptr<Message>(tmpMessage->New());
		return true;
    }

    bool ProtoComponent::New(const std::string &name, const std::string &json, std::shared_ptr<Message> & message)
    {
		if(!this->New(name, message))
		{
			return false;
		}
		return util::JsonStringToMessage(json, message.get()).ok();
    }

	bool ProtoComponent::New(const string& name, const char* json, size_t size, std::shared_ptr<Message> & message)
	{
		if(!this->New(name, message))
		{
			return false;
		}
		return util::JsonStringToMessage(StringPiece(json, size), message.get()).ok();
	}

    const Message * ProtoComponent::FindMessage(const std::string &name)
	{
		auto iter1 = this->mDynamicMessageMap.find(name);
		if (iter1 != this->mDynamicMessageMap.end())
		{
			return iter1->second;
		}

		//静态查找
		const DescriptorPool* pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor* pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor == nullptr)
		{
			return nullptr;
		}
		MessageFactory* factory = MessageFactory::generated_factory();
		const Message* pMessage = factory->GetPrototype(pDescriptor);
		if (pMessage == nullptr)
		{
			return nullptr;
		}
		this->mStaticMessageMap.emplace(name, pMessage);
		return pMessage;
	}

    bool ProtoComponent::HasMessage(const std::string &name)
    {
        return this->FindMessage(name) != nullptr;
    }

    void ProtoComponent::OnLuaRegister(Lua::ClassProxyHelper &luaRegister)
    {
        luaRegister.BeginNewTable("Proto");
		luaRegister.PushExtensionFunction("New", Lua::MessageEx::New);
		luaRegister.PushExtensionFunction("Import", Lua::MessageEx::Import);
	}

	bool ProtoComponent::Write(lua_State * lua, const Message& message)
	{
		MessageDecoder messageDecoder(lua, this);
		return messageDecoder.Decode(message);
	}
	std::shared_ptr<Message> ProtoComponent::Read(lua_State * lua, const std::string& name, int index)
	{
        std::shared_ptr<Message> message;
		if(!this->New(name, message))
		{
			return nullptr;
		}
		MessageEncoder messageEncoder(lua);
        return messageEncoder.Encode(message, index) ? message : nullptr;
	}

}