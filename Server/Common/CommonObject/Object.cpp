#include"Object.h"
#include<CommonCore/Applocation.h>
#include<CommonCoroutine/CoroutineManager.h>
namespace SoEasy
{
	Object::Object()
	{
		this->mIsActive = false;
		this->mAppLocation = nullptr;
		this->mCoroutineManager = nullptr;
	}

	Object::~Object()
	{

	}

	void Object::Init(Applocation * app, std::string & typeName)
	{
		this->SetActive(true);
		this->mAppLocation = app;
		this->mCoroutineManager = app->GetManager<CoroutineManager>();
	}
}