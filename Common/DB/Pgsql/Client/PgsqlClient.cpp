//
// Created by 64658 on 2025/2/18.
//

#include <iomanip>
#include "PgsqlClient.h"
#include "Proto/Bson/base64.h"
#include "Util/Tools/String.h"
#include "Util/Crypt/MD5Helper.h"
#include "Core/Thread/ThreadSync.h"
#include "Pgsql/Common/PgsqlCommon.h"
#include "XCode/XCode.h"

#ifdef __ENABLE_OPEN_SSL__
#include <openssl/sha.h>
namespace sha256
{
	inline std::string compute_client_proof(const std::string& password, const std::string& client_first_bare,
			const std::string& server_first, const std::string& client_final_no_proof,
			const std::string& salt_b64, int iterations) {
		// Base64 解码盐
		std::string salt = _bson::base64::decode(salt_b64);

		// 计算 SaltedPassword
		unsigned char salted_password[SHA256_DIGEST_LENGTH];
		PKCS5_PBKDF2_HMAC(password.c_str(), password.size(),
				reinterpret_cast<const unsigned char*>(salt.c_str()), salt.size(),
				iterations, EVP_sha256(), SHA256_DIGEST_LENGTH, salted_password);

		// 计算 ClientKey
		unsigned char client_key[SHA256_DIGEST_LENGTH];
		HMAC(EVP_sha256(), salted_password, SHA256_DIGEST_LENGTH,
				reinterpret_cast<const unsigned char*>("Client Key"), 10, client_key, nullptr);


		// 计算 StoredKey
		unsigned char stored_key[SHA256_DIGEST_LENGTH];
		SHA256(client_key, SHA256_DIGEST_LENGTH, stored_key);

		// 计算 AuthMessage
		std::string auth_message = client_first_bare + "," + server_first + "," + client_final_no_proof;

		// 计算 ClientSignature
		unsigned char client_signature[SHA256_DIGEST_LENGTH];
		HMAC(EVP_sha256(), stored_key, SHA256_DIGEST_LENGTH,
				reinterpret_cast<const unsigned char*>(auth_message.c_str()), auth_message.size(),
				client_signature, nullptr);

		// 计算 ClientProof (XOR)
		unsigned char client_proof[SHA256_DIGEST_LENGTH] = { 0};
		for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
			client_proof[i] = client_key[i] ^ client_signature[i];
		}

		// Base64 编码
		return _bson::base64::encode((char *)client_proof, SHA256_DIGEST_LENGTH);
	}
}

#endif

namespace pgsql
{
	Client::Client(int id, pgsql::Component* component, pgsql::Config& config, Asio::Context & main)
		: mClientId(id), tcp::Client(1024 * 1024), mConfig(config), mComponent(component), mMain(main)
	{

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
		if(socket == nullptr)
		{
			Asio::Context & context = this->mSocket->GetContext();
			std::shared_ptr<tcp::Client> self = this->shared_from_this();
			asio::post(context, [self, this]() {
				this->mConnectCount = 0;
				this->Connect(5);
			});
			return XCode::Ok;
		}
		this->SetSocket(socket);
		custom::ThreadSync<int> threadSync;
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [self = this->shared_from_this(), &threadSync, this]()
		{
			int code = this->Auth(true);
			threadSync.SetResult(code);
		});
		return threadSync.Wait();
#endif

	}

	void Client::Send(std::unique_ptr<pgsql::Request> request)
	{
#ifdef ONLY_MAIN_THREAD
		assert(this->mRequest == nullptr);
		this->mRequest = std::move(request);
		this->Write(*this->mRequest);
#else
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(context, [self, this, req = request.release()]()
		{
			this->mRequest.reset(req);
			this->Write(*this->mRequest);
		});
#endif

	}

	bool Client::CreateDatabase()
	{
		if (this->mConfig.db.empty())
		{
			return true;
		}
		const std::string & db = this->mConfig.db;
		const std::string & user = this->mConfig.user;
		std::string sql2 = fmt::format("CREATE DATABASE {} OWNER {};", db, user);
		return this->ReadResponse(sql2, this->mResponse);
	}

	int Client::Auth(bool connect)
	{
		if(connect)
		{
			Asio::Code code;
			if(!this->ConnectSync(code))
			{
				return XCode::ConnectDatabaseFail;
			}
		}
		pgsql::StartRequest request;
		{
			request.db = this->mConfig.db;
			request.user = this->mConfig.user;
		}
		if(!this->SendSync(request))
		{
			return XCode::NetSendFailure;
		}
		char type = 0;
		std::string response;
		if(!this->ReadResponse(type, response))
		{
			return XCode::NetReadFailure;
		}
		if(type != pgsql::type::auth)
		{
			return XCode::PasswordAuthFail;
		}
		unsigned int authType = pgsql::ReadLength(response.c_str());
		int code = this->Auth(authType, response.substr(4));
		if(code == XCode::Ok && this->mRequest == nullptr)
		{
			std::shared_ptr<tcp::Client> self = this->shared_from_this();
			asio::post(this->mMain, [self, this, id = this->mClientId]()
			{
				this->mComponent->OnConnectOK(id);
			});
		}
		return code;
	}

	int Client::Auth(unsigned int type, const std::string & message)
	{
		std::string packet;
		switch(type)
		{
			case pgsql::auth::OK:
				return XCode::Ok;
			case pgsql::auth::PASSWORD:
			{
				packet.append(this->mConfig.password);
				break;
			}
			case pgsql::auth::MD5_PASSWORD:
			{
				std::string input = this->mConfig.password + this->mConfig.user;
				std::string step1 = help::md5::GetHex(input);
				std::string md5 = help::md5::GetHex(step1 + message);
				packet.append("md5");
				packet.append(md5);
				break;
			}
#ifdef __ENABLE_OPEN_SSL__
			case pgsql::auth::SCRAM_SHA_256:
			{
				return this->AuthBySha256();
			}
#endif
			default:
			LOG_ERROR("[{}] {}", type, message);
				return XCode::AuthPluginNonsupport;
		}

		pgsql::Request request(pgsql::type::password, packet);
		if(!this->SendSync(request))
		{
			return XCode::NetSendFailure;
		}
		pgsql::Result response;
		if(!this->ReadResponse(response))
		{
			return XCode::NetReadFailure;
		}
		if(!response.mError.empty())
		{
			CONSOLE_LOG_ERROR("{}", response.mError);
			return XCode::PasswordAuthFail;
		}
		return XCode::Ok;
	}
#ifdef __ENABLE_OPEN_SSL__
	int Client::AuthBySha256()
	{
		std::string mechanism = "SCRAM-SHA-256";
		std::string nonce = _bson::base64::encode(help::Str::RandomString(16));
		std::string client_first = "n,,n=" + this->mConfig.user + ",r=" + nonce;

		unsigned int content_length = mechanism.size() + 1    // 机制名称 + '\0'
									  + 4                       // client-first长度字段
									  + client_first.size();    // client-first内容

		unsigned int total_length = content_length + 4;
		unsigned int total_size = 1 + total_length; // 包含消息类型'p'

		std::unique_ptr<char[]> buffer = std::make_unique<char[]>(total_size);
		unsigned int offset = 0;

		buffer[offset++] = 'p';

//		unsigned int net_total_length = htonl(total_length);
//		std::memcpy(buffer.get() + offset, &net_total_length, sizeof(net_total_length));
//		offset += sizeof(net_total_length);

		tcp::Data::Write(buffer.get() + offset, total_length);
		offset += sizeof(total_length);

		std::memcpy(buffer.get() + offset, mechanism.c_str(), mechanism.size());
		offset += mechanism.size();
		buffer[offset++] = '\0';


		unsigned int net_client_first_len = client_first.size();

		tcp::Data::Write(buffer.get() + offset, net_client_first_len);
		offset += sizeof(net_client_first_len);

		std::memcpy(buffer.get() + offset, client_first.c_str(), client_first.size());
		offset += client_first.size();

		if (!this->SendSync(buffer.get(), total_size))
		{
			return XCode::NetSendFailure;
		}

		char type = 0;
		std::string responseMessage;
		if (!this->ReadResponse(type, responseMessage))
		{
			return XCode::NetReadFailure;
		}
		if (type != pgsql::type::auth || responseMessage.size() < 8)
		{
			return XCode::PasswordAuthFail;
		}

		unsigned int auth_type = ntohl(*reinterpret_cast<const unsigned int*>(responseMessage.data()));
		if (auth_type != 11)
		{
			return XCode::PasswordAuthFail;
		}

		std::string server_first = responseMessage.substr(4);

		// 解析 r=, s=, i=
		size_t r_pos = server_first.find("r=");
		size_t s_pos = server_first.find("s=");
		size_t i_pos = server_first.find("i=");
		if (r_pos == std::string::npos || s_pos == std::string::npos || i_pos == std::string::npos)
		{
			return XCode::PasswordAuthFail;
		}

		// 提取 nonce
		size_t r_end = server_first.find(',', r_pos);
		std::string server_nonce = server_first.substr(r_pos + 2, r_end - (r_pos + 2));

		// 提取 salt
		size_t s_end = server_first.find(',', s_pos);
		std::string server_salt = server_first.substr(s_pos + 2, s_end - (s_pos + 2));

		// 提取 iterations
		 int iterations = std::stoi(server_first.substr(i_pos + 2));

		 const std::string & password = this->mConfig.password;
		std::string client_final_no_proof = "c=biws,r=" + server_nonce; // biws = base64(n,,)
		std::string client_first_bare = "n=" + this->mConfig.user + ",r=" + nonce;
		std::string client_proof = sha256::compute_client_proof(password, client_first_bare, server_first,
				client_final_no_proof, server_salt, iterations);
		std::string client_final = client_final_no_proof + ",p=" + client_proof;

		unsigned int length = 4 + client_final.size();
		total_size = 1 + length;

		offset = 0;
		buffer = std::make_unique<char[]>(total_size);

		buffer[offset++] = 'p';
		tcp::Data::Write(buffer.get() + offset, length);
		offset += sizeof(unsigned int);

		std::memcpy(buffer.get() + offset, client_final.c_str(), client_final.size());
		offset += client_final.size();

		if(!this->SendSync(buffer.get(), total_size))
		{
			return XCode::NetSendFailure;
		}

		if(!this->ReadResponse(type, responseMessage))
		{
			return XCode::NetReadFailure;
		}
		if(type != pgsql::type::auth)
		{
			return XCode::PasswordAuthFail;
		}
		auth_type = ntohl(*reinterpret_cast<const unsigned int*>(responseMessage.data()));
		if(auth_type != 12)
		{
			return XCode::PasswordAuthFail;
		}
		std::string server_final = responseMessage.substr(4);
		size_t v_pos = server_final.find("v=");
		if(v_pos == std::string::npos)
		{
			return XCode::PasswordAuthFail;
		}
		this->ReadResponse(this->mResponse);
		return XCode::Ok;
	}
#endif
	bool Client::ReadResponse(char& type, std::string& response)
	{
		type = 0;
		response.clear();
		size_t count = 0;
		if (!this->RecvSync(5, count))
		{
			return false;
		}
		std::istream stream(&this->mRecvBuffer);
		{
			char buffer[4] = { 0 };
			type = (char)stream.get();
			stream.readsome(buffer, sizeof(buffer));
			unsigned int len = pgsql::ReadLength(buffer);
			if(len > 0)
			{
				if (!this->RecvSync(len - sizeof(buffer), count))
				{
					return false;
				}
				response.resize(count);
				stream.readsome((char *)response.c_str(), count);
			}
		}
		return true;
	}

	bool Client::ReadResponse(const std::string& sql, pgsql::Result& response)
	{
		pgsql::Request request(sql);
		if(!this->SendSync(request))
		{
			return false;
		}
		return this->ReadResponse(response);
	}

	bool Client::ReadResponse(pgsql::Result & response)
	{
		size_t count = 0;
		response.Clear();

		do
		{
			if (!this->RecvSync(5, count))
			{
				return false;
			}
			std::istream stream(&this->mRecvBuffer);
			int len = response.OnRecvMessage(stream, count);
			if(len > 0)
			{
				if (!this->RecvSync(len, count))
				{
					return false;
				}
				response.OnRecvMessage(stream, count);
			}
		}
		while(response.mType != pgsql::type::ready_for_query);
		return true;
	}

	void Client::OnReadError(const Asio::Code& code)
	{
		if(code != asio::error::eof)
		{
			this->Connect(5);
		}
	}

	void Client::OnConnect(const Asio::Code& code, int count)
	{
		if(code.value() != Asio::OK)
		{
			if(count <= this->mConfig.conn_count)
			{
				this->Connect(5);
				return;
			}
		}
		else if(this->Auth(false) == XCode::Ok)
		{
			if(this->mRequest != nullptr)
			{
				this->Write(*this->mRequest);
			}
			return;
		}
#ifdef ONLY_MAIN_THREAD
		int id = this->mClientId;
		if(this->mRequest != nullptr)
		{
			this->mComponent->OnSendFailure(id, this->mRequest.release());
		}
		this->mComponent->OnClientError(id, XCode::ConnectDatabaseFail);
#else
		int id = this->mClientId;
		asio::post(this->mMain, [id, this, self = this->shared_from_this()]()
		{
			if(this->mRequest != nullptr)
			{
				this->mComponent->OnSendFailure(id, this->mRequest.release());
			}
			this->mComponent->OnClientError(id, XCode::ConnectDatabaseFail);
		});
#endif
	}

	void Client::OnSendMessage(size_t)
	{
		this->ReadLength(5);
	}

	void Client::OnReceiveMessage(std::istream& , size_t size, const asio::error_code& code)
	{
		this->ReadResponse(this->mResponse);
		std::unique_ptr<pgsql::Response> response = std::make_unique<pgsql::Response>();
		{
			this->mResponse.DecodeData(*response);
#ifdef ONLY_MAIN_THREAD
			int id = this->mClientId;
			std::unique_ptr<pgsql::Request> request = std::move(this->mRequest);
			this->mComponent->OnMessage(id, request.get(), response.release());
#else
			pgsql::Request * request = this->mRequest.release();
			std::shared_ptr<tcp::Client> self = this->shared_from_this();
			asio::post(this->mMain, [self, this, request, res = response.release()]
			{
				int id = this->mClientId;
				this->mComponent->OnMessage(id, request, res);
				delete request;
			});
#endif
		}
	}
}