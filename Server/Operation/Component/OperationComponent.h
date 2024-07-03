//
// Created by yy on 2024/4/27.
//

#ifndef APP_OPERATIONCOMPONENT_H
#define APP_OPERATIONCOMPONENT_H
#include "Entity/Component/Component.h"
namespace joke
{
	class OperationComponent : public Component
	{
	public:
		OperationComponent();
		~OperationComponent() = default;
	public:
		int Add(int city, int type, const char * field, int value);
	private:
		bool LateAwake() final;
	private:
		class RedisComponent * mRedis;
	};
}


#endif //APP_OPERATIONCOMPONENT_H
