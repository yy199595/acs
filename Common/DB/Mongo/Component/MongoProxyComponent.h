//
// Created by 64658 on 2024/9/21.
//

#ifndef APP_MONGOPROXYCOMPONENT_H
#define APP_MONGOPROXYCOMPONENT_H
#include "XCode/XCode.h"
#include "MongoComponent.h"
namespace acs
{
	class MongoProxyComponent final : public Component
	{
	public:
		MongoProxyComponent() : mMongo(nullptr) { }
	public:
		template<typename T>
		inline std::unique_ptr<T> QueryOne(json::w::Document & filter);
	private:
		bool LateAwake() final;
	private:
		MongoComponent * mMongo;
	};
	template<typename T>
	std::unique_ptr<T> MongoProxyComponent::QueryOne(json::w::Document& filter)
	{
		std::unique_ptr<T> result = std::make_unique<T>();
		std::unique_ptr<json::r::Document> document = std::make_unique<json::r::Document>();
		{
			const std::string & tab = result->GetDBName();
			if(this->mMongo->FindOne(tab.c_str(), filter, document.get()) != XCode::Ok)
			{
				return nullptr;
			}
			if(!result->Decode(*document))
			{
				return false;
			}
		}
		return result;
	}
}



#endif //APP_MONGOPROXYCOMPONENT_H
