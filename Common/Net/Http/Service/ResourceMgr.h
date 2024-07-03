//
// Created by leyi on 2023/11/20.
//

#ifndef APP_RESOURCEMGR_H
#define APP_RESOURCEMGR_H
#include "HttpService.h"
#include "Core/Map/HashMap.h"

namespace joke
{
	class ResourceMgr : public HttpService
	{
	public:
		ResourceMgr() = default;
		~ResourceMgr() override = default;
	private:
		bool Awake() final;
		bool OnInit() final;
	private:
		int Look(const http::FromData& request, http::Response& response);
		int List(const http::FromData& request, http::Response& response);
		int Upload(const http::Request & request, http::Response & response);
		int Download(const http::FromData & request, http::Response & response);
	private:
		std::string mUpload;
		std::string mDoMain;
	};
}


#endif //APP_RESOURCEMGR_H
