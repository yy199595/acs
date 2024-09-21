//
// Created by leyi on 2024/3/8.
//

#ifndef APP_RECORDCOMPONENT_H
#define APP_RECORDCOMPONENT_H
#include "Entity/Component/Component.h"

namespace http
{
	class Request;
	class Response;
}

namespace acs
{
	class HttpMethodConfig;
	class RecordComponent : public Component, public IComplete, public IRequest<HttpMethodConfig, http::Request, http::Response>
	{
	public:
		RecordComponent();
		~RecordComponent() final = default;
	private:
		bool LateAwake() final;
		void Complete() final;
	public:
		void OnRequestDone(const acs::HttpMethodConfig &c, const http::Request &t1, const http::Response &t2) final;
	private:
		class MongoComponent * mMongo;
	};
}


#endif //APP_RECORDCOMPONENT_H
