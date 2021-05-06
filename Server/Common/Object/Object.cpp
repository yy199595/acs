#include"Object.h"
#include<Core/Applocation.h>
#include<Util/NumberHelper.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	Object::Object()
	{
		this->SetActive(false);
		this->mCoroutineManager = nullptr;
	}

	Object::~Object()
	{
		
	}

	bool Object::Init(Applocation * app, const std::string & name)
	{
		if (this->mIsActive == false)
		{
			this->SetActive(true);
			this->mClassName = name;
			this->mAppLocation = app;
			this->mAppLocation = Applocation::Get();
			this->mIntanceID = NumberHelper::Create();
			return true;
		}
		return false;
	}

	CoroutineManager * Object::GetScheduler()
	{
		
		return this->mCoroutineManager;
	}
}