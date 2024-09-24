//
// Created by leyi on 2023/11/6.
//

#include<ostream>
#include"MongoMgr.h"
#include"Core/System/System.h"
#include"Util/File/DirectoryHelper.h"
#include"Mongo/Component/MongoComponent.h"

namespace acs
{
	MongoMgr::MongoMgr()
	{
		this->mMongo = nullptr;
	}

	bool MongoMgr::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(MongoMgr::Export);
		this->mMongo = this->GetComponent<MongoComponent>();
		return true;
	}

	int MongoMgr::Export(const http::FromContent& request, http::Response& response)
	{
		std::string tab;
		std::string work = os::System::WorkPath();
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab));
		std::string dir = fmt::format("{}/export", work);
		if (!help::dir::DirectorIsExist(dir))
		{
			help::dir::MakeDir(dir);
		}
		int page = 0;
		std::string path = fmt::format("{}/{}.json", dir, tab);
		std::ofstream ofs(path.c_str(), std::ios::out);
		if (!ofs.is_open())
		{
			return XCode::Failure;
		}
		ofs << "[\n";
		int count = 0;
		json::w::Document filter;
		db::mongo::find_page::request request1;
		int sumCount = this->mMongo->Count(tab.c_str(), filter);
		std::unique_ptr<db::mongo::find_page::response> response1 = std::make_unique<db::mongo::find_page::response>();
		do
		{
			page++;
			response1->Clear();
			request1.set_tab(tab);
			request1.set_page(page);
			request1.set_count(200);
			if (this->mMongo->FindPage(request1, response1.get()) != XCode::Ok)
			{
				break;
			}
			for (int index = 0; index < response1->json_size(); index++)
			{
				count++;
				const std::string& json = response1->json(index);
				ofs.write(json.c_str(), json.size());
				if (count < sumCount)
				{
					ofs << ",\n";
				}
			}
			LOG_WARN("export {}.json {:.2f}%", tab, (count / (float)sumCount) * 100.0f);
			ofs.flush();
		} 
		while (response1->json_size() > 0);
		{
			ofs << "\n]\n";
			ofs.flush();
			ofs.close();
		}
		response.File(http::Header::JSON, path);
		response.Header().Add("Content-Disposition", fmt::format("attachment; filename=\"{}.json\"", tab));
		return XCode::Ok;
	}
}