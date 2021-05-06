#pragma once
#include<string>
#include<XCode/XCode.h>
#include<Object/Object.h>
#include<Util/JsonHelper.h>
namespace SoEasy
{
	class CommandBase
	{
	public:
		virtual XCode Invoke(const std::string & paramate, RapidJsonWriter & returnData) = 0;
	};

	class StopCommand : public CommandBase
	{
	public:
		XCode Invoke(const std::string & paramate, RapidJsonWriter & returnData) override;
	};
	
	class StateCommand : public CommandBase
	{
	public:
		XCode Invoke(const std::string & paramate, RapidJsonWriter & returnData) override;
	};
}