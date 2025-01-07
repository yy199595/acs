//
// Created by yy on 2024/7/5.
//

#include "WatchComponent.h"

namespace acs
{
	WatchComponent::WatchComponent()
	= default;

	void WatchComponent::OnSecondUpdate(int tick)
	{
		for (auto & process : this->mProcess)
		{
			if (!process.IsRunning() && !process.Run())
			{
				LOG_ERROR("restart process error");
			}
		}
		//LOG_WARN("cur process : {}", this->mProcess.size());
	}

	bool WatchComponent::CloseProcess(long long pid)
	{
		for(size_t index = 0; index < this->mProcess.size(); index++)
		{
			os::Process & process = this->mProcess[index];
			if(process.GetPID() == pid)
			{
				process.Close();
				this->mProcess.erase(this->mProcess.begin() + index);
				return true;
			}
		}
		return false;
	}

	bool WatchComponent::RestartProcess(long long pid)
	{
		for (size_t index = 0; index < this->mProcess.size(); index++)
		{
			os::Process& process = this->mProcess[index];
			if (process.GetPID() == pid)
			{
				int count = 0;
				bool result = false;
				do
				{
					count++;
					result = process.Close();
				}
				while (!result && count < 3);		
				return result;
			}
		}
	}

	void WatchComponent::GetAllProcess(json::w::Document& response)
	{
		auto jsonArray = response.AddArray("list");
		for (size_t index = 0; index < this->mProcess.size(); index++)
		{
			auto jsonObejct = jsonArray->AddObject();
			os::Process& process = this->mProcess[index];
			{
				jsonObejct->Add("pid", process.GetPID());
				jsonObejct->Add("cmd", process.CmdLine());
				jsonObejct->Add("name", process.Name());
			}
		}
	}

	bool WatchComponent::StartProcess(const std::string& name, const std::string& exe, const std::string& args)
	{
		os::Process process(exe, name);
		if(!process.Start(args))
		{
			return false;
		}
		this->mProcess.emplace_back(process);
		return true;
	}

	bool WatchComponent::StartProcess(const std::string& name, const std::string& exe, const std::vector<std::string>& args)
	{
		os::Process process(exe, name);
		if(!process.Start(args))
		{
			return false;
		}
		this->mProcess.emplace_back(process);
		return true;
	}

}