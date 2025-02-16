//
// Created by leyi on 2024/3/28.
//

#ifndef APP_FILEUPLOAD_H
#define APP_FILEUPLOAD_H
#include "Http/Service/HttpService.h"

namespace upload
{
	const int FILE_UPLOAD_ICON = 1; //用户头像
	const int FILE_UPLOAD_CLUB_ICON = 2; //俱乐部头像
	const int FILE_UPLOAD_WX_CODE = 3; //微信二维码
	const int FILE_UPLOAD_ACTIVITY_RES = 4; //活动图片
	const int FILE_SHARE_ICON = 5; //分享的图片
	const int FILE_VIDEO_RES = 6; //视频
}

namespace acs
{
	class FileUpload final : public HttpService
	{
	public:
		FileUpload();
		~FileUpload() final = default;
	private:
		bool Awake() final;
		bool OnInit() final;
	private:
		int File(const http::Request & request, http::Response & response);
#ifdef __ENABLE_OPEN_SSL__
		int Oss(const http::FromContent & request, json::w::Document & response);
#endif
	private:
		std::string mDoMain;
#ifdef __ENABLE_OPEN_SSL__
		class OssComponent * mOss;
#endif
	};
}


#endif //APP_FILEUPLOAD_H
