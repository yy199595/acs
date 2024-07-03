//
// Created by leyi on 2024/3/28.
//

#include "FileUpload.h"
#include "Entity/Actor/App.h"
#include "Util/File/FileHelper.h"
#include "Util/Time/TimeHelper.h"
#include "Oss/Component/OssComponent.h"
namespace joke
{
	bool FileUpload::Awake()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		if (!this->mApp->Config().Get("http", jsonObject))
		{
			return false;
		}
		return jsonObject->Get("domain", this->mDoMain);
	}

	bool FileUpload::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(FileUpload::Oss);
		BIND_COMMON_HTTP_METHOD(FileUpload::Icon);
		BIND_COMMON_HTTP_METHOD(FileUpload::Code);
		BIND_COMMON_HTTP_METHOD(FileUpload::Club);
		BIND_COMMON_HTTP_METHOD(FileUpload::File);
		this->mOss = this->GetComponent<OssComponent>();
		return true;
	}

	int FileUpload::Oss(const http::FromData& request, json::w::Document & response)
	{
		std::string fileName;
		int userId, uploadType = 0;
		if(!request.Get(http::query::UserId, userId))
		{
			return XCode::CallArgsError;
		}
		if(!request.Get("type", uploadType))
		{
			return XCode::CallArgsError;
		}

		oss::Policy policy;
		if(!request.Get("file", policy.file_type))
		{
			return XCode::CallArgsError;
		}
		long long nowTime = help::Time::NowSec();
		policy.limit_type.emplace_back("image/png");
		policy.limit_type.emplace_back("image/jpg");
		policy.limit_type.emplace_back("image/jpeg");
		policy.expiration = help::Time::NowSec() + 30;
		policy.file_name = fmt::format("{}-{}", userId, nowTime);
		switch(uploadType)
		{
			case upload::FILE_UPLOAD_ICON:
			{
				policy.max_length = 1024 * 500;
				policy.upload_dir = "user-icon/";
				break;
			}
			case upload::FILE_UPLOAD_CLUB_ICON:
				policy.max_length = 1024 * 500;
				policy.upload_dir = "club-icon/";
				break;
			case upload::FILE_UPLOAD_WX_CODE:
				policy.max_length = 1024 * 1024;
				policy.upload_dir = "club-wx_code/";
				break;
			case upload::FILE_SHARE_ICON:
				policy.upload_dir = "share/";
				policy.max_length = 1024 * 1024;
				break;
			case upload::FILE_VIDEO_RES:
			{
				policy.limit_type.clear();
				policy.upload_dir = "video/";
				policy.max_length = 1024 * 1024 * 50;
				policy.limit_type.push_back("video/avi");
				policy.limit_type.push_back("video/mp4");
				break;
			}
			case upload::FILE_UPLOAD_ACTIVITY_RES:
			{
				int permission = 0;
				policy.upload_dir = fmt::format("{}/", userId);
				policy.file_name = std::to_string(this->mApp->MakeGuid());
				if(!request.Get(http::query::Permission, permission))
				{
					return XCode::PermissionDenied;
				}
				if(permission < http::PermissCreator)
				{
					return XCode::PermissionDenied;
				}
				policy.max_length = 1024 * 1024 * 5;
			}
				break;
			default:
				return XCode::CallArgsError;
		}
		std::string json = this->mOss->Sign(policy);
		if(json.empty())
		{
			return XCode::Failure;
		}
		response.AddJson("data", json);
		return XCode::Ok;
	}

	int FileUpload::Icon(const http::Request& request, http::Response& response)
	{
		const std::string name("icon");
		return this->OnUpload(request, name, response);
	}

	int FileUpload::Club(const http::Request& request, http::Response& response)
	{
		const std::string name("club");
		return this->OnUpload(request, name, response);
	}

	int FileUpload::Code(const http::Request& request, http::Response& response)
	{
		const std::string name("code");
		return this->OnUpload(request, name, response);
	}

	int FileUpload::File(const http::Request &request, http::Response &response)
	{
		int userId = 0;
		const http::Data* data = request.GetBody();
		request.GetUrl().GetQuery().Get(http::query::UserId, userId);
		const http::MultipartFromData* multiData = data->To<const http::MultipartFromData>();
		if (multiData == nullptr)
		{
			return XCode::CallArgsError;
		}
		if (!multiData->IsDone())
		{
			return XCode::CallArgsError;
		}
		const std::string & path = multiData->Path();
		const std::string& name = multiData->FileName();
		const std::string newName = fmt::format("{}/{}", userId, name);
		if(!help::fs::ChangeName(path, newName))
		{
			//LOG_ERROR("change file name {}=>{}", path, newName)
			return XCode::Failure;
		}
		const std::string url = fmt::format("{}/{}", this->mDoMain, newName);
		response.SetContent(http::Header::TEXT, url);
		return XCode::Ok;
	}

	int FileUpload::OnUpload(const http::Request& request, const std::string& name, http::Response& response)
	{
		int userId = 0;
		const http::Data* data = request.GetBody();
		request.GetUrl().GetQuery().Get(http::query::UserId, userId);
		const http::MultipartFromData* multiData = data->To<const http::MultipartFromData>();
		if (multiData == nullptr)
		{
			return XCode::CallArgsError;
		}
		if (!multiData->IsDone())
		{
			return XCode::CallArgsError;
		}
		std::string fileType;
		const std::string & path = multiData->Path();
		if(!help::fs::GetFileType(path, fileType))
		{
			return XCode::Failure;
		}
		const std::string newName = fmt::format("{}/{}.{}", userId, name, fileType);
		if(!help::fs::ChangeName(path, newName))
		{
			return XCode::Failure;
		}
		const std::string url = fmt::format("{}/{}", this->mDoMain, newName);
		response.SetContent(http::Header::TEXT, url);
		return XCode::Ok;
	}
}