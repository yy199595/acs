//
// Created by leyi on 2023/11/6.
//

#include"LogMgr.h"
#include"Message/com/com.pb.h"
#include"Entity/Actor/App.h"
#include"Util/Time/TimeHelper.h"
#include"Util/File/FileHelper.h"
#include"Util/File/DirectoryHelper.h"

namespace joke
{
	bool LogMgr::Awake()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		const ServerConfig& config = this->mApp->Config();
		LOG_CHECK_RET_FALSE(config.Get("log", jsonObject));
		LOG_CHECK_RET_FALSE(jsonObject->Get("path", this->mLogDir));
		return true;
	}

	bool LogMgr::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(LogMgr::List);
		BIND_COMMON_HTTP_METHOD(LogMgr::Look);
		BIND_COMMON_HTTP_METHOD(LogMgr::Delete);
		return true;
	}

	int LogMgr::Look(const http::FromData& request, http::Response& response)
	{
		std::string file;
		if(!request.Get("file", file) || file.empty())
		{
			response.SetCode(HttpStatus::BAD_REQUEST);
			return XCode::CallArgsError;
		}
		std::string path = fmt::format("{}/{}",this->mLogDir, file);
		std::unique_ptr<http::FileData> fileData = std::make_unique<http::FileData>();
		{
			fileData->OpenFile(path, http::Header::TEXT);
		}
		response.SetContent(std::move(fileData));
		return XCode::Ok;
	}

	int LogMgr::List(const http::FromData& request, http::Response & response)
	{
		std::string dir;
		request.Get("dir", dir);
		dir = dir.empty() ? this->mLogDir : fmt::format("{}/{}", this->mLogDir, dir);
		std::unique_ptr<http::TextData> customData(new http::TextData(http::Header::HTML));
		{
			customData->Append("<!DOCTYPE html>");
			customData->Append("<meta http-equiv='Content-Type' content='text/html;charset=utf-8'/>");
			customData->Append("<html>");
			customData->Append("<head>");
			customData->Append(fmt::format("<title>{}</title>", "日志列表"));
			customData->Append("/<head>");
			customData->Append("<body>");
			customData->Append("<li>");
			size_t index = 1;
			std::vector<std::string> directors, files;
			if (dir != this->mLogDir)
			{
				index = 0;
				directors.emplace_back(dir);
				while (directors.front().back() != '/')
				{
					directors.front().pop_back();
				}
				directors.front().pop_back();
				customData->Append(fmt::format("<a href='/log'>{}</a>", "/log", "root"));
				customData->Append("</li>");
			}
			help::dir::GetDirAndFiles(dir, directors, files);

			std::sort(directors.begin(), directors.end(),
					[](const std::string & p1, const std::string & p2) ->bool
					{
						long long t1 = help::fs::GetLastWriteTime(p1);
						long long t2 = help::fs::GetLastWriteTime(p2);
						return t1 > t2;
					});


			for (const std::string& fullDir : directors)
			{
				std::string key, director;
				customData->Append("<li>");
				if (index == 0)
				{
					key = "返回";
					index++;
				}
				else
				{
					key = fullDir.substr(dir.size() + 1);
				}
				if (fullDir.size() > this->mLogDir.size())
				{
					director = fullDir.substr(this->mLogDir.size() + 1);
				}
				//director = fullDir.substr(this->mDownload.size() + 1);
				std::string url = fmt::format("/log?dir={}", director);
				customData->Append(fmt::format("<a href='{}'>{}</a>", url, key));
				customData->Append("</li>");
			}
			std::sort(files.begin(), files.end(),
					[](const std::string & p1, const std::string & p2) ->bool
					{
						long long t1 = help::fs::GetLastWriteTime(p1);
						long long t2 = help::fs::GetLastWriteTime(p2);
						return t1 > t2;
					});
			for (const std::string& fullPath : files)
			{
				std::string suffix;
				help::fs::GetFileType(fullPath, suffix);
				//std::string key = fullPath.substr(dir.size() + 1);
				std::string path = fullPath.substr(this->mLogDir.size());
				{
					std::string fileSize;
					customData->Append("<li>");
					help::fs::GetFileSize(fullPath, fileSize);
					long long time = help::fs::GetLastWriteTime(fullPath);
					const std::string timeStr = help::Time::GetDateString(time);
					customData->Append(fmt::format("<span>[{}][{}] &nbsp</span>",fileSize, timeStr));
					customData->Append(fmt::format("<a href='/log/look?file={}'>&nbsp{}</a>", path, path));
					customData->Append("</li>");
					//html.emplace_back(fmt::format("<li><a href='{}'>{}</a></li>", url, path.substr(1)));
				}
			}
			customData->Append("</li>");
			customData->Append("</body>");
			customData->Append("</html>");
		}
		response.SetContent(std::move(customData));
		return XCode::Ok;
	}

	int LogMgr::Delete(const http::FromData& request, json::w::Value& response)
	{
		std::string file;
		if(!request.Get("file", file))
		{
			return XCode::CallArgsError;
		}
		std::string path = fmt::format("{}/{}", this->mLogDir, file);
		if(std::remove(path.c_str()) != 0)
		{
			return XCode::Failure;
		}
		return XCode::Ok;
	}

	int LogMgr::SetLevel(const http::FromData& request, json::w::Value& response)
	{
		int id = 0;
		int level = 0;
		LOG_ERROR_CHECK_ARGS(request.Get("id", id));
		LOG_ERROR_CHECK_ARGS(request.Get("level", level));
		Server * targetServer = this->mApp->ActorMgr()->GetServer(id);
		if(targetServer == nullptr)
		{
			return XCode::NotFoundActor;
		}
		com::type::int32 message;
		message.set_value(level);
		return targetServer->Call("Node.SetLogLevel", message);
	}
}