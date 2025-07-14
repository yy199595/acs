//
// Created by yy on 2025/7/6.
//

#include "MysqlProxyComponent.h"
#include "Mysql/Service/MysqlReadProxy.h"
#include "Mysql/Service/MysqlWriteProxy.h"
#include "Node/Component/NodeComponent.h"
#include "Entity/Component/ComponentFactory.h"
namespace acs
{
    MysqlProxyComponent::MysqlProxyComponent()
    {
        this->mNode = nullptr;
        this->mReadName = ComponentFactory::GetName<MysqlReadProxy>();
        this->mWriteName = ComponentFactory::GetName<MysqlWriteProxy>();
    }

    bool MysqlProxyComponent::LateAwake()
    {
        this->mNode = this->GetComponent<NodeComponent>();
        return true;
    }

    int MysqlProxyComponent::InsertOne(const char* tab, const pb::Message& message)
    {
        std::string document;
        if(!pb_json::MessageToJsonString(message, &document).ok())
        {
            return XCode::ProtoCastJsonFailure;
        }
        json::w::Document request;
        request.Add("tab", tab);
        request.AddObject("document", document);
        const std::string func = this->mWriteName + ".InsertOne";
        return this->CallWriteProxy(func, request);
    }

    int MysqlProxyComponent::InsertOne(const char* tab, const json::w::Document& document)
    {
        json::w::Document request;
        request.Add("tab", tab);
        request.Add("document", document);
        const std::string func = this->mWriteName + ".InsertOne";
        return this->CallWriteProxy(func, request);
    }

    int MysqlProxyComponent::ReplaceOne(const char* tab, const json::w::Document& document)
    {
        json::w::Document request;
        request.Add("tab", tab);
        request.Add("document", document);
        const std::string func = this->mWriteName + ".Replace";
        return this->CallWriteProxy(func, request);
    }

    int MysqlProxyComponent::DeleteOne(const char* tab, const json::w::Document& filter)
    {
        json::w::Document request;
        {
            request.Add("tab", tab);
            request.Add("limit", 1);
            request.Add("filter", filter);
        }
        const std::string func = this->mWriteName + ".Delete";
        return this->CallWriteProxy(func, request);
    }

    int MysqlProxyComponent::UpdateOne(const char* tab, const json::w::Document& filter, const json::w::Document& document)
    {
        json::w::Document request;
        {
            request.Add("tab", tab);
            request.Add("limit", 1);
            request.Add("filter", filter);
            request.Add("document", document);
        }
        const std::string func = this->mWriteName + ".Update";
        return this->CallWriteProxy(func, request);
    }

    int MysqlProxyComponent::CallWriteProxy(const std::string& func, const json::w::Document& request)
    {
        Node * mysqlProxy = this->mNode->Next(this->mWriteName);
        if(mysqlProxy == nullptr)
        {
            return XCode::NotFoundActor;
        }
        std::unique_ptr<json::r::Document> response = std::make_unique<json::r::Document>();
        if( mysqlProxy->Call(func, request, response) != XCode::Ok)
        {
            return XCode::Failure;
        }
        int count = 0;
        return response->Get("count", count) && count >= 1 ? XCode::Ok : XCode::Failure;
    }

    int MysqlProxyComponent::RunInRead(const std::string& sql, std::unique_ptr<json::r::Document>& response)
    {
        Node * mysqlProxy = this->mNode->Next(this->mReadName);
        if(mysqlProxy == nullptr)
        {
            return XCode::NotFoundActor;
        }
        const std::string func = this->mReadName + ".Run";
        return mysqlProxy->Call(func, sql, response);
    }


    int MysqlProxyComponent::RunInWrite(const std::string& sql, std::unique_ptr<json::r::Document>& response)
    {
        Node * mysqlProxy = this->mNode->Next(this->mWriteName);
        if(mysqlProxy == nullptr)
        {
            return XCode::NotFoundActor;
        }
        const std::string func = this->mWriteName + ".Run";
        return mysqlProxy->Call(func, sql, response);
    }

    long long MysqlProxyComponent::Inc(const char* tab, const char* field, const json::w::Document& filter, int value)
    {
        long long result = 0;
        do
        {
            Node * mysqlProxy = this->mNode->Next(this->mWriteName);
            if(mysqlProxy == nullptr)
            {
                result = -1;
                break;
            }
            json::w::Document request;
            {
                request.Add("tab", tab);
                request.Add("field", field);
                request.Add("value", value);
                request.Add("filter", filter);
            }
            const std::string func = this->mWriteName + ".Inc";
            std::unique_ptr<json::r::Document> response = std::make_unique<json::r::Document>();
            if(mysqlProxy->Call(func, request, response) != XCode::Ok)
            {
                result = 0;
            }
            response->Get("value", result);
        }
        while(false);
        return result;
    }

    int MysqlProxyComponent::FindOne(const char* tab, const json::w::Document& filter, std::unique_ptr<json::r::Document>& document)
    {
        Node * mysqlProxy = this->mNode->Next(this->mWriteName);
        if(mysqlProxy == nullptr)
        {
            return  XCode::NotFoundActor;
        }
        json::w::Document request;
        {
            request.Add("tab", tab);
            request.Add("filter", filter);
        }
        const std::string func = this->mWriteName + ".FindOne";
        return  mysqlProxy->Call(func, request, document);
    }

    int MysqlProxyComponent::Find(const char* tab, const json::w::Document& filter, std::unique_ptr<json::r::Document>& document)
    {
        Node * mysqlProxy = this->mNode->Next(this->mWriteName);
        if(mysqlProxy == nullptr)
        {
            return  XCode::NotFoundActor;
        }
        json::w::Document request;
        {
            request.Add("tab", tab);
            request.Add("filter", filter);
        }
        const std::string func = this->mWriteName + ".Find";
        return  mysqlProxy->Call(func, request, document);
    }

    int MysqlProxyComponent::Find(const char* tab, const std::list<std::string>& fields, const json::w::Document& filter, std::unique_ptr<json::r::Document>& document)
    {
        Node * mysqlProxy = this->mNode->Next(this->mWriteName);
        if(mysqlProxy == nullptr)
        {
            return  XCode::NotFoundActor;
        }
        json::w::Document request;
        {
            request.Add("tab", tab);
            request.Add("filter", filter);
            request.Add("fields", fields);
        }
        const std::string func = this->mWriteName + ".Find";
        return  mysqlProxy->Call(func, request, document);
    }

    int MysqlProxyComponent::FindOne(const char* tab, const std::list<std::string>& fields, const json::w::Document& filter, std::unique_ptr<json::r::Document>& document)
    {
        Node * mysqlProxy = this->mNode->Next(this->mReadName);
        if(mysqlProxy == nullptr)
        {
            return  XCode::NotFoundActor;
        }
        json::w::Document request;
        {
            request.Add("tab", tab);
            request.Add("filter", filter);
            request.Add("fields", fields);
        }
        const std::string func = this->mReadName + ".FindOne";
        return  mysqlProxy->Call(func, request, document);
    }

    long long MysqlProxyComponent::Count(const char* tab, const json::w::Document& filter)
    {
        long long result = 0;
        do
        {
            Node * mysqlProxy = this->mNode->Next(this->mReadName);
            if(mysqlProxy == nullptr)
            {
                result = -1;
                break;
            }
            json::w::Document request;
            {
                request.Add("tab", tab);
                request.Add("filter", filter);
            }
            const std::string func = this->mReadName + ".Count";
            std::unique_ptr<json::r::Document> response = std::make_unique<json::r::Document>();
            if(mysqlProxy->Call(func, request, response) != XCode::Ok)
            {
                result = 0;
                break;
            }
            response->Get("count", result);
        }
        while(false);
        return result;
    }
}