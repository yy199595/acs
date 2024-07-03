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

namespace joke
{
	class RecordComponent : public Component
	{
	public:
		RecordComponent();
		~RecordComponent() = default;
	private:
		bool LateAwake() final;
	public:
		void OnRecord(const class HttpMethodConfig * config, http::Request * request, http::Response * response);
	private:
		class MongoComponent * mMongo;
	};
}


#endif //APP_RECORDCOMPONENT_H
