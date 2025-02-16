//
// Created by yy on 2025/2/10.
//

#include "Client.h"

#include <utility>
#include "Util/Tools/String.h"
#include "Util/Crypt/sha1.h"
#include "Core/Thread/ThreadSync.h"

namespace mysql
{
	Client::Client(int id, tcp::Socket* socket, mysql::Client::Component* component, mysql::Config config, Asio::Context& io)
		: mClientID(id), tcp::Client(socket, 1024 * 1024), mConfig(std::move(config)), mComponent(component), mMain(io)
	{
		this->mIndex = 0;
	}

	bool Client::Connect()
	{
		custom::ThreadSync<bool> threadSync;
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(context, [this, &threadSync, self]
		{
			mysql::Response response;
			if(this->Auth(response))
			{
				threadSync.SetResult(true);
				return;
			}
			threadSync.SetResult(false);
			CONSOLE_LOG_ERROR("{}", response.GetBuffer());
		});
		return threadSync.Wait();
	}

	void Client::Send(std::unique_ptr<Request> request)
	{
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(context, [message = request.release(), this, self]()
		{
			unsigned char index = ++this->mIndex;
			if(index == 255)
			{
				this->mIndex = 0;
			}
			message->SetIndex(0);
			this->mMessages.emplace(std::move(std::unique_ptr<Request>(message)));
			if(this->mMessages.size() == 1)
			{
				this->Write(*this->mMessages.front());
			}
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
		stream.readsome((char*)response.data(), size);
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

	bool Client::Auth(mysql::Response & response)
	{
		Asio::Code code;
		if(!this->ConnectSync(code))
		{
			return false;
		}
		std::string response1;
		if(!this->SyncReadResponse(response1))
		{
			return false;
		}
		if(!this->mHandshake.OnDecode(response1))
		{
			return false;
		}
		std::string message;
		mysql::LoginRequest loginRequest;
		{
			loginRequest.mUser = this->mConfig.user;
			//loginRequest.mDatabase = this->mConfig.db;
			loginRequest.mSalt = this->mHandshake.salt;
			loginRequest.mPassword = this->mConfig.passwd;
			loginRequest.mCharset = mysql::config::charset;
			loginRequest.mAuthPlugin = this->mHandshake.auth_plugin_name;
		}
		if(!this->SendSync(loginRequest))
		{
			return false;
		}
		if(!this->SyncReadResponse(response))
		{
			return false;
		}
		return response.IsOk() && this->SwitchDataBase();
	}

	bool Client::SyncRun(const std::string & sql, mysql::Response& response)
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

	bool Client::SwitchDataBase()
	{
		if (this->mConfig.db.empty())
		{
			return true;
		}
		// 创建数据库 并切换
		mysql::Response response;
		std::string sql1 = fmt::format("USE {};", this->mConfig.db);
		std::string sql2 = fmt::format("CREATE DATABASE IF NOT EXISTS {};", this->mConfig.db);
		if(!this->SyncRun(sql1, response))
		{
			return this->SyncRun(sql2, response);
		}
		return true;
	}

	void Client::OnSendMessage(size_t size)
	{
		this->ReadLength(4);
	}

	void Client::OnSendMessage(const Asio::Code& code)
	{
		mysql::Response response;
		if(!this->Auth(response))
		{

		}
	}

	void Client::OnReadError(const Asio::Code& code)
	{
		CONSOLE_LOG_ERROR("{}", code.message());
	}

	void Client::OnReceiveLine(std::istream& readStream, size_t size)
	{
		asio::error_code code;
		this->OnReceiveMessage(readStream, size, code);
	}

	void Client::OnReceiveMessage(std::istream& readStream, size_t size, const asio::error_code& code)
	{
		std::unique_ptr<mysql::Response> response = std::make_unique<mysql::Response>();
		{
			this->SyncReadResponse(*response);
			int code = response->GetPackageCode();
			if(code != mysql::PACKAGE_OK && code != mysql::PACKAGE_ERR)
			{
				this->mFields.clear();
				unsigned int pos = 0;
				unsigned int count = response->DecodeColumnCount(pos);
				mysql::Result & mysqlResult = response->GetResult();
				for(int index = 0; index < count; index++)
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
				if(this->mResponse.GetPackageCode() != mysql::PACKAGE_EOF)
				{
					std::string json;
					this->SyncReadResponse(this->mResponse);
					while(this->mResponse.GetPackageCode() != mysql::PACKAGE_EOF)
					{
						pos = 0;
						json::w::Document document;
						for(const mysql::FieldInfo & fieldInfo : this->mFields)
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
									long long number = std::stoll(value);
									document.Add(fieldInfo.name.c_str(), number);
									break;
								}
								case mysql::field::MYSQL_TYPE_FLOAT:
								case mysql::field::MYSQL_TYPE_DOUBLE:
								{
									double number = std::stod(value);
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
						this->SyncReadResponse(this->mResponse);
					}
				}
			}
		}
		mysql::Request* req = nullptr;
		if(!this->mMessages.empty())
		{
			req = this->mMessages.front().release();
			this->mMessages.pop();
			if(!this->mMessages.empty())
			{
				this->Write(*this->mMessages.front());
			}
		}
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(this->mMain, [this, self, req, resp = response.release()]
		{
			this->mComponent->OnMessage(this->mClientID, req, resp);
			delete req;
		});
	}
}