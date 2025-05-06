//
// Created by yy on 2025/2/10.
//

#include "MysqlClient.h"

#include "Util/Tools/String.h"
#include "Util/Crypt/sha1.h"
#include "Util/File/FileHelper.h"
#include "Core/Thread/ThreadSync.h"
#include "XCode/XCode.h"

namespace mysql
{
	Client::Client(int id, mysql::Client::Component* component, mysql::Config config, Asio::Context& io)
			: mClientID(id), tcp::Client(1024 * 1024), mConfig(std::move(config)), mComponent(component),
			  mMain(io)
	{
		this->mIndex = 0;
	}

	int Client::Start(tcp::Socket* socket)
	{
#ifdef ONLY_MAIN_THREAD
		if(socket == nullptr)
		{
			this->Connect(5);
			return XCode::Ok;
		}
		this->SetSocket(socket);
		return this->Auth(true);
#else
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		if (socket == nullptr)
		{
			Asio::Context& context = this->mSocket->GetContext();
			asio::post(context, [self, this]()
			{
				this->mConnectCount = 0;
				this->Connect(5);
			});
			return XCode::Ok;
		}
		this->SetSocket(socket);
		custom::ThreadSync<int> threadSync;
		Asio::Context& context = this->mSocket->GetContext();
		asio::post(context, [this, &threadSync, self]
		{
			int code = this->Auth(true);
			if(code == XCode::Ok && !this->mConfig.script.empty())
			{
				std::string sql;
				if(help::fs::ReadTxtFile(this->mConfig.script, sql))
				{
					this->SyncRun(sql, this->mResponse);
				}
			}
			threadSync.SetResult(code);
		});
		return threadSync.Wait();
#endif

	}

	void Client::StartReceive()
	{
		Asio::Context& context = this->mSocket->GetContext();
		asio::post(context, [self = this->shared_from_this(), this]
		{
			this->ReadLength(4);
		});
	}


	void Client::Send(std::unique_ptr<Request> request)
	{
		Asio::Context& context = this->mSocket->GetContext();
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(context, [req = request.release(), this, self]()
		{
			unsigned char index = ++this->mIndex;
			if (index == 255)
			{
				this->mIndex = 0;
			}
			req->SetIndex(0);
			this->mMessage.reset(req);
			this->Write(*this->mMessage);
		});
	}

	bool Client::SyncReadResponse(std::string& response)
	{
		size_t size = 0;
		if (!this->RecvSync(4, size))
		{
			return false;
		}
		char buffer[4] = { 0 };
		std::istream stream(&this->mRecvBuffer);
		stream.readsome(buffer, sizeof(buffer));
		this->mIndex = (unsigned char)buffer[3];
		int length = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16);
		if (!this->RecvSync(length, size))
		{
			return false;
		}
		response.resize(length);
		stream.readsome((char*)response.data(), length);
		return true;
	}

	bool Client::SyncReadResponse(mysql::Response& response)
	{
		size_t size = 0;
		response.Clear();
		if (!this->RecvSync(4, size))
		{
			return false;
		}
		std::istream stream(&this->mRecvBuffer);
		int len = response.OnRecvMessage(stream, size);
		if (len <= 0)
		{
			return false;
		}
		if (!this->RecvSync(len, size))
		{
			return false;
		}
		response.OnRecvMessage(stream, size);
		this->mIndex = response.GetIndex();
		return true;
	}

	int Client::Auth(bool connect)
	{
		if (connect)
		{
			Asio::Code error;
			if (!this->ConnectSync(error))
			{
				return XCode::ConnectDatabaseFail;
			}
		}

		std::string response1;
		if (!this->SyncReadResponse(response1))
		{
			return XCode::NetReadFailure;
		}
		if (!this->mHandshake.OnDecode(response1))
		{
			return XCode::PasswordAuthFail;
		}
		std::string message;
		uint32_t capability_flags = this->mHandshake.capability_flags;
		mysql::LoginRequest loginRequest;
		{
			loginRequest.user = this->mConfig.user;
			//loginRequest.database = this->mConfig.db;
			loginRequest.salt = this->mHandshake.salt;
			loginRequest.charset = mysql::config::charset;
			loginRequest.password = this->mConfig.password;
			loginRequest.authPlugin = this->mHandshake.auth_plugin_name;
			loginRequest.enable_compression = (capability_flags & mysql::client_flag::CLIENT_COMPRESS) != 0;
		}
		int code = this->Login(loginRequest);
		if (code != XCode::Ok)
		{
			return code;
		}
		if (!this->InitDataBase())
		{
			return XCode::CreateDatabaseFail;
		}
		if (this->mMessage == nullptr)
		{
			std::shared_ptr<tcp::Client> self = this->shared_from_this();
			asio::post(this->mMain, [self, this, id = this->mClientID]()
			{
				this->mComponent->OnConnectOK(id);
			});
		}
		return XCode::Ok;
	}

	int Client::Login(const mysql::LoginRequest& request)
	{
		std::vector<uint8_t> packet;
		if (request.Encode(packet) != XCode::Ok)
		{
			return XCode::AuthPluginNonsupport;
		}
		uint8_t sequence_id = 1; // 登录包的 sequence_id 必须是 1
		uint32_t length = packet.size();

		std::vector<uint8_t> final_packet = mysql::encode(length);
		final_packet.emplace_back(sequence_id);
		//final_packet.emplace_back(mysql::cmd::CONNECT);

		final_packet.insert(final_packet.end(), packet.begin(), packet.end());

		if (!this->SendSync((char*)final_packet.data(), final_packet.size()))
		{
			return XCode::NetSendFailure;
		}
		do
		{
			if (!this->SyncReadResponse(this->mResponse))
			{
				return XCode::NetReadFailure;
			}
		} while (this->mResponse.GetPackageCode() == mysql::PACKAGE_MORE);
		return this->mResponse.GetPackageCode() == mysql::PACKAGE_OK ? XCode::Ok : XCode::Failure;
	}

	bool Client::SyncRun(const std::string& sql, mysql::Response& response)
	{
		mysql::Request request(sql);
		if (!this->SendSync(request))
		{
			return false;
		}
		if (!this->SyncReadResponse(response))
		{
			return false;
		}
		return response.IsOk();
	}

	bool Client::InitDataBase()
	{
		if (this->mConfig.db.empty())
		{
			return true;
		}
		// 创建数据库 并切换
		std::string sql1 = fmt::format("USE {};", this->mConfig.db);
		std::string sql2 = fmt::format("CREATE DATABASE IF NOT EXISTS {};", this->mConfig.db);
		if (this->SyncRun(sql2, this->mResponse))
		{
			if (!this->SyncRun(sql1, this->mResponse))
			{
				return false;
			}
		}
		return true;
	}

	void Client::OnSendMessage(size_t size)
	{
		this->ReadLength(4);
	}

	void Client::OnConnect(const Asio::Code& code, int count)
	{
		if (code.value() != Asio::OK)
		{
			if (count <= this->mConfig.conn_count)
			{
				this->Connect(10);
				return;
			}
		}
		else if (this->Auth(false) == XCode::Ok)
		{
			if (this->mMessage != nullptr)
			{
				this->Write(*this->mMessage);
			}
			return;
		}
#ifdef ONLY_MAIN_THREAD
		int id = this->mClientID;
		this->mComponent->OnClientError(id, XCode::NetConnectFailure);
		if (this->mMessage != nullptr)
		{
			this->mComponent->OnSendFailure(id, this->mMessage.release());
		}
#else
		int id = this->mClientID;
		asio::post(this->mMain, [this, self = this->shared_from_this(), id]
		{
			this->mComponent->OnClientError(id, XCode::NetConnectFailure);
			if (this->mMessage != nullptr)
			{
				this->mComponent->OnSendFailure(id, this->mMessage.release());
			}
		});
#endif

	}

	void Client::OnSendMessage(const Asio::Code& code)
	{
		this->Connect(10);
	}

	void Client::OnReadError(const Asio::Code& code)
	{
		if (code != asio::error::eof)
		{
			this->Connect(10);
		}
	}

	void Client::OnReceiveLine(std::istream& readStream, size_t size)
	{
		asio::error_code code;
		this->OnReceiveMessage(readStream, size, code);
	}

	std::unique_ptr<mysql::Response> Client::ReadResponse()
	{
		std::unique_ptr<mysql::Response> response = std::make_unique<mysql::Response>();
		{
			this->SyncReadResponse(*response);
			int code = response->GetPackageCode();
			if (code == mysql::PACKAGE_ERR || code == mysql::PACKAGE_OK)
			{
				return response;
			}

			this->mFields.clear();
			unsigned int pos = 0;
			unsigned int count = response->DecodeColumnCount(pos);
			mysql::Result& mysqlResult = response->GetResult();
			for (int index = 0; index < count; index++)
			{
				pos = 0;
				this->SyncReadResponse(this->mResponse);
				{
					mysql::FieldInfo fieldInfo;
					std::string catalog = this->mResponse.ReadString(pos);
					std::string schema = this->mResponse.ReadString(pos);
					std::string table = this->mResponse.ReadString(pos);
					std::string org_table = this->mResponse.ReadString(pos);
					fieldInfo.name = this->mResponse.ReadString(pos);
					std::string org_name = this->mResponse.ReadString(pos);
					pos += 7;
					fieldInfo.type = this->mResponse.ReadChar(pos);
					unsigned short flags = this->mResponse.ReadShort(pos);
					fieldInfo.decimals = this->mResponse.ReadChar(pos);
					this->mFields.emplace_back(fieldInfo);
				}
			}

			std::string json;
			this->SyncReadResponse(this->mResponse);
			if (this->mResponse.GetPackageCode() == mysql::PACKAGE_EOF)
			{
				return response;
			}
			while (this->mResponse.GetPackageCode() != mysql::PACKAGE_EOF)
			{
				pos = 0;
				if(this->mResponse.GetPackageCode() != mysql::PACKAGE_OK)
				{
					json::w::Document document;
					for (const mysql::FieldInfo& fieldInfo: this->mFields)
					{
						std::string value = this->mResponse.ReadString(pos);
						switch (fieldInfo.type)
						{
							case mysql::field::MYSQL_TYPE_TINY:
							case mysql::field::MYSQL_TYPE_SHORT:
							case mysql::field::MYSQL_TYPE_INT24:
							case mysql::field::MYSQL_TYPE_YEAR:
							case mysql::field::MYSQL_TYPE_LONG:
							case mysql::field::MYSQL_TYPE_LONGLONG:
							case mysql::field::MYSQL_TYPE_NEWDECIMAL:
							{
								long long number = 0;
								if (!value.empty())
								{
									number = std::stoll(value);
								}
								document.Add(fieldInfo.name.c_str(), number);
								break;
							}
							case mysql::field::MYSQL_TYPE_FLOAT:
							case mysql::field::MYSQL_TYPE_DOUBLE:
							{
								double number = 0;
								if (!value.empty())
								{
									number = std::stod(value);
								}
								document.Add(fieldInfo.name.c_str(), number);
								break;
							}
							case mysql::field::MYSQL_TYPE_JSON:
							{
								document.AddObject(fieldInfo.name.c_str(), value);
								break;
							}
							case mysql::field::MYSQL_TYPE_NULL:
							{
								document.AddNull(fieldInfo.name.c_str());
								break;
							}
							default:
								document.Add(fieldInfo.name.c_str(), value);
								break;
						}
					}
					document.Encode(&json);
					mysqlResult.contents.emplace_back(json);
				}
				this->SyncReadResponse(this->mResponse);
			}
		}
		return response;
	}

	void Client::OnReceiveMessage(std::istream& readStream, size_t size, const asio::error_code&)
	{
		std::unique_ptr<mysql::Response> response = this->ReadResponse();
		{
			
#ifdef ONLY_MAIN_THREAD
			std::unique_ptr<mysql::Request> request = std::move(this->mMessage);
			this->mComponent->OnMessage(this->mClientID, request.get(), response.release());
#else
			mysql::Request* req = nullptr;
			if (this->mMessage != nullptr)
			{
				req = this->mMessage.release();
			}
			std::shared_ptr<tcp::Client> self = this->shared_from_this();
			asio::post(this->mMain, [this, self, req, resp = response.release()]
				{
					this->mComponent->OnMessage(this->mClientID, req, resp);
					delete req;
				});
#endif
			
		}
	}
}
