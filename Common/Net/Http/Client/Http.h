//
// Created by zmhy0073 on 2021/10/27.
//

#ifndef APP_HTTP_H
#define APP_HTTP_H

#include<string>
#include<unordered_map>

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \
/**
 * @brief HTTP状态枚举
 */
enum class HttpStatus
{
#define XX(code, name, desc) name = (code),
	HTTP_STATUS_MAP(XX)
#undef XX
};

inline const char* HttpStatusToString(const HttpStatus& s)
{
	switch (s)
	{
#define XX(code, name, msg) \
        case HttpStatus::name: \
            return #msg;
		HTTP_STATUS_MAP(XX);
#undef XX
		default:
			return "<unknown>";
	}
}

namespace http
{
	constexpr int PermissUser = 1;    //用户权限
	constexpr int PermissCreator = 8; //主创
	constexpr int PermissCharge = 9;	//商户
	constexpr int PermissPersonCharge = 10;	//主理人
	constexpr int PermissOperate = 20; //运营者
	constexpr int PermissAdmin = 100; //管理员权限

	constexpr const char* CRLF = "\r\n";
	constexpr const char* CRLF2 = "\r\n\r\n";
	constexpr const char* Version = "HTTP/1.1";
	constexpr unsigned int BodyMaxCount = 1024 * 10;

	enum ContentType
	{
		NONE = 0, FROM = 1, JSON = 2, TEXT = 3, FILE = 4, CHUNKED = 5, MULTIPAR = 6, XML = 7, PB = 8
	};

	namespace query
	{
		constexpr const char * UserId = "USER_ID";
		constexpr const char * ClubId = "CLUB_ID";
		constexpr const char * Permission = "PERMISSION";
	}

	namespace Header
	{
		constexpr const char * RealIp = "x-real-ip";
//		constexpr const char * Permission = "X-Permission";
//		constexpr const char * OpenId = "X-Open-ID";
		constexpr const char* Auth = "Authorization";
		constexpr const char* Token = "Access-Token";
		constexpr const char* SetCookie = "Set-Cookie";
		constexpr const char* Connection = "Connection";
		constexpr const char* ContentType = "Content-Type";
		constexpr const char* ContentLength = "Content-Length";
		constexpr const char* ContentEncoding = "Content-Encoding";
		constexpr const char* TransferEncoding = "Transfer-Encoding";
		constexpr const char* AccessControlAllowOrigin = "Access-Control-Allow-Origin";
		constexpr const char * ContentDisposition = "Content-Disposition";

		constexpr const char* Close = "close";
		constexpr const char* KeepAlive = "keep-alive";

		constexpr const char* CSS = "text/css";
		constexpr const char* LUA = "text/lua";
		constexpr const char* HTML = "text/html";
		constexpr const char* JS = "application/javascript";

		constexpr const char* MPEG = "audio/mpeg";
		constexpr const char* WAV = "audio/wav";

		constexpr const char* JPG = "image/jpg";
		constexpr const char* JPEG = "image/jpeg";
		constexpr const char* PNG = "image/png";
		constexpr const char* GIF = "image/gif";
		constexpr const char* ICO = "image/x-icon";

		constexpr const char* MP4 = "video/mp4";
		constexpr const char* AVI = "video/avi";

		constexpr const char * ZIP = "application/zip";

		constexpr const char* TEXT = "text/plain";
		constexpr const char* JSON = "application/json";
		constexpr const char* PB = "application/x-protobuf";
		constexpr const char* XML = "application/xml";
		constexpr const char* XHTML = "application/xhtml+xml";
		constexpr const char* FORM = "application/x-www-form-urlencoded";
		constexpr const char* PDF = "application/pdf";
		constexpr const char* WORD = "application/msword";
		constexpr const char* Bin = "application/octet-stream";
		constexpr const char* MulFromData = "multipart/form-data";
	}
};


#define HTTP_CONTENT_TYPE_MAP(XX)       		\
    XX("html", http::Header::HTML)             	\
    XX("css", http::Header::CSS)               	\
    XX("js", http::Header::JS)  				\
                                        		\
    XX("jpg", http::Header::JPG)             	\
    XX("ico", http::Header::ICO)             	\
    XX("png", http::Header::PNG)              	\
    XX("gif", http::Header::GIF)              	\
    XX("jpeg", http::Header::JPEG)            	\
                                        		\
    XX("mp4", http::Header::MP4)              	\
    XX("avi", http::Header::AVI)              	\
    XX("wav", http::Header::WAV)              	\
                                        		\
    XX("log", http::Header::TEXT)             	\
    XX("text", http::Header::TEXT)            	\
    XX("h", http::Header::TEXT)               	\
    XX("cpp", http::Header::TEXT)             	\
    XX("hpp", http::Header::TEXT)             	\
    XX("proto", http::Header::TEXT)           	\
    XX("cc", http::Header::TEXT)              	\
    XX("json", http::Header::JSON)      		\
	XX("xml", http::Header::XML)         		\
	XX("zip", http::Header::ZIP)         		\

namespace http
{
// 获取后缀名对应的Content-Type
	inline const char* GetContentType(const std::string& suffix)
	{
		static const std::unordered_map<std::string, const char*> contentTypeMap = {
#define XX(suffix, contentType) {suffix, contentType},
				HTTP_CONTENT_TYPE_MAP(XX)
#undef XX
		};
		auto it = contentTypeMap.find(suffix);
		return (it != contentTypeMap.end()) ? it->second : "text/plain"; // 默认的Content-Type
	}
}

#endif
