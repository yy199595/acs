//
// Created by mac on 2022/5/18.
//

#include"MongoClient.h"
#include"XCode/XCode.h"
#include <utility>
#include"Util/Crypt/md5.h"
#ifdef __MONGO_DB_AUTH_SHA256__
#include"Util/Crypt/sha256.h"
#else
#include "Util/Crypt/sha1.h"
#endif

#include"Proto/Bson/base64.h"
#include"Util/Tools/String.h"
#include"Mongo/Config/MongoConfig.h"
#include"Core/Thread/ThreadSync.h"
#include"Mongo/Client/MongoFactory.h"

namespace mongo
{
    std::string SaltPassword(std::string & pwd, std::string salt, int iter)
    {
        salt = salt + '\0' + '\0' + '\0' + '\1';
#ifdef __MONGO_DB_AUTH_SHA256__
        std::string output = help::Sha256::GetHMacHash(pwd, salt);
        std::string inter(output);
        for(int index = 2; index <= iter; index++)
        {
            inter = help::Sha256::GetHMacHash(pwd, inter);
            output = help::Sha256::XorString(output, inter);
        }
#else
		std::string output = help::Sha1::GetHMacHash(pwd, salt);
		std::string inter(output);
		for(int index = 2; index <= iter; index++)
		{
			inter = help::Sha1::GetHMacHash(pwd, inter);
			output = help::Sha1::XorString(output, inter);
		}
#endif
        return output;
    }

    void Client::Stop()
    {
        //TODO
    }

	bool Client::Start(bool async)
	{
#ifdef ONLY_MAIN_THREAD
		return this->StartAuthBySha1();
#else
		if (!async)
		{
			return this->StartAuthBySha1();
		}
		custom::ThreadSync<bool> threadSync;
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, &threadSync]
			{
				if (this->StartAuthBySha1())
				{
					threadSync.SetResult(true);
					this->StartUpdate(this->mConfig.Ping);
					return;
				}
				threadSync.SetResult(false);
			});
		return threadSync.Wait();
#endif
	}

    Client::Client(tcp::Socket * socket, Component * component,
			MongoConfig config, Asio::Context & io)
		: tcp::Client(socket, 0), mComponent(component), mConfig(std::move(config)), mMainContext(io)
	{
		this->mRequest = nullptr;
		this->mResponse = nullptr;
	}

	Client::Client(tcp::Socket * socket, MongoConfig  config, Asio::Context & io)
		: tcp::Client(socket, 0), mComponent(nullptr), mConfig(std::move(config)), mMainContext(io)
	{
		this->mRequest = nullptr;
		this->mResponse = nullptr;
	}

	void Client::OnSendMessage(size_t size)
	{
		this->ReadLength(sizeof(Head));
//		if(this->mSendMessages.Size() >= 10)
//		{
//			LOG_WARN("mongo cmd count:{}", this->mSendMessages.Size());
//		}
	}

	void Client::OnTimeout(tcp::TimeoutFlag flag)
	{
		switch(flag)
		{
			case tcp::TimeoutFlag::Write:
				this->OnResponse(XCode::SendDataTimeout);
				break;
			case tcp::TimeoutFlag::Connect:
				this->OnResponse(XCode::ConnectTimeout);
				break;
			case tcp::TimeoutFlag::ReadLine:
			case tcp::TimeoutFlag::ReadSome:
			case tcp::TimeoutFlag::ReadCount:
				this->OnResponse(XCode::ReadDataTimeout);
				break;
		}
	}

	void Client::OnSendMessage(const Asio::Code & code)
	{
		this->Connect();
	}

    bool Client::Auth(const std::string &user, const std::string &db, const std::string &pwd)
    {
        if(pwd.empty())
        {
            return true;
        }
        std::string nonce = _bson::base64::encode(help::Str::RandomString(8));
        std::string firstBare = fmt::format("n={0},r={1}", this->mConfig.User, nonce);
		std::unique_ptr<mongo::Request> request1 = std::make_unique<mongo::Request>();

		std::string payload = _bson::base64::encode(fmt::format("n,,{0}", firstBare));

        request1->header.requestID = 1;
        request1->collectionName = db + ".$cmd";
        request1->document.Add("saslStart", 1);
        request1->document.Add("autoAuthorize", 1);
#ifdef __MONGO_DB_AUTH_SHA256__
        request1->document.Add("mechanism", "SCRAM-SHA-256");
#else
		request1->document.Add("mechanism", "SCRAM-SHA-1");
#endif
        request1->document.Add("payload", payload);

        std::unique_ptr<Response> response1 = this->SyncSendMongoCommand(std::move(request1));
		if(response1 == nullptr || response1->Document() == nullptr)
		{
			return false;
		}

        int conversationId = 0;
        std::string server_first;
        bson::Reader::Document* document1 = response1->Document();
        if(!document1->Get("payload", server_first) || !document1->Get("conversationId", conversationId))
        {
            return false;
        }

        std::string parsedSource = _bson::base64::decode(server_first);

        std::vector<std::string> ret;
        help::Str::Split(parsedSource, ',', ret);

        std::string salt(ret[1].c_str() + 2, ret[1].size() - 2);
        std::string rnonce(ret[0].c_str() + 2, ret[0].size() - 2);
        int iterations = std::stoi(std::string(ret[2].c_str() + 2, ret[2].size() - 2));

        std::string without_proof = "c=biws,r=" + rnonce;
        std::string pbkdf2_key = help::md5::GetMd5(
                fmt::format("{0}:mongo:{1}", user, pwd));
        std::string salted_pass = SaltPassword(pbkdf2_key,
                                               _bson::base64::decode(salt), iterations);

#ifdef __MONGO_DB_AUTH_SHA256__
        std::string client_key = help::Sha256::GetHMacHash(salted_pass, "Client Key");
        std::string stored_key = help::Sha256::GetHash(client_key);

        std::string auth_msg = firstBare + ',' + parsedSource + ',' + without_proof;
        std::string client_sin = help::Sha256::GetHMacHash(stored_key, auth_msg);
        std::string client_key_xor_sig = help::Sha256::XorString(client_key, client_sin);
        std::string client_proof = std::string("p=") + _bson::base64::encode(client_key_xor_sig);
        std::string client_final = _bson::base64::encode(without_proof + "," + client_proof);
        std::string server_key = help::Sha256::GetHMacHash(salted_pass, "Server Key");
        std::string server_sig = _bson::base64::encode(help::Sha256::GetHMacHash(server_key, auth_msg));
#else
		std::string client_key = help::Sha1::GetHMacHash(salted_pass, "Client Key");
		std::string stored_key = help::Sha1::GetHash(client_key);

		std::string auth_msg = firstBare + ',' + parsedSource + ',' + without_proof;
		std::string client_sin = help::Sha1::GetHMacHash(stored_key, auth_msg);
		std::string client_key_xor_sig = help::Sha1::XorString(client_key, client_sin);
		std::string client_proof = std::string("p=") + _bson::base64::encode(client_key_xor_sig);
		std::string client_final = _bson::base64::encode(without_proof + "," + client_proof);
		std::string server_key = help::Sha1::GetHMacHash(salted_pass, "Server Key");
		std::string server_sig = _bson::base64::encode(help::Sha1::GetHMacHash(server_key, auth_msg));
#endif

        std::unique_ptr<mongo::Request> request2 = std::make_unique<mongo::Request>();
		{
			request2->header.requestID = 1;
			request2->collectionName = db + ".$cmd";

			request2->document.Add("saslContinue", 1);
			request2->document.Add("conversationId", conversationId);
			request2->document.Add("payload", client_final);
		}
        std::unique_ptr<Response> response2 = this->SyncSendMongoCommand(std::move(request2));
        if(response2 == nullptr || response2->Document() == nullptr)
		{
			return false;
		}
        parsedSource.clear();
        bson::Reader::Document * document2 = response2->Document();
        if(!document2->Get("payload", parsedSource))
        {
            return false;
        }
        bool done = false;
        if(document2->Get("done", done) && done)
        {
            return true;
        }
        std::unique_ptr<Request> request3 = std::make_unique<Request>();
		{
			request3->header.requestID = 1;
			request3->collectionName = db + ".$cmd";
			request3->document.Add("saslContinue", 1);
			request3->document.Add("conversationId", conversationId);
			request3->document.Add("payload", "");
		}

        std::unique_ptr<Response> response3 = this->SyncSendMongoCommand(std::move(request3));

		if(response3 == nullptr || response3->Document() == nullptr)
		{
			return false;
		}

        bson::Reader::Document * document3 = response3->Document();
        if(document3->IsOk() && document3->Get("done", done) && done)
		{
			return true;
		}
		return false;
    }

	void Client::OnReadError(const Asio::Code& code)
	{
		this->Connect(3);
	}

    void Client::OnReceiveMessage(std::istream & is, size_t size, const Asio::Code &)
	{
		if (this->mResponse == nullptr)
		{
			this->mResponse = std::make_unique<mongo::Response>();
		}

		int length = this->mResponse->OnRecvMessage(is, size);
		if (length > 0)
		{
			this->ReadLength(length);
			return;
		}
		this->OnResponse(XCode::Ok, std::move(this->mRequest), std::move(this->mResponse));
	}

	void Client::OnResponse(int code)
	{
		this->StopTimer();
		this->ClearBuffer();
		if (this->mRequest != nullptr)
		{
			int id = this->mRequest->header.requestID;
			this->mResponse = std::make_unique<mongo::Response>(id);
			this->OnResponse(code, std::move(this->mRequest), std::move(this->mResponse));
		}
	}

	void Client::OnResponse(int code, std::unique_ptr<Request> request, std::unique_ptr<Response> response)
	{
		this->StopTimer();
		this->ClearBuffer();
		if (this->mComponent == nullptr || request->header.requestID == 0)
		{
			this->mRequest.reset();
			this->mResponse.reset();
			return;
		}
		if(response->Document() != nullptr)
		{
			std::string error;
			response->SetCode(code);
			if(response->Document()->Get("errmsg", error))
			{
				response->SetCode(XCode::UnKnowPacket);
			}
		}
		if(code != XCode::Ok && request->GetRpcId() != response->RpcId())
		{
			response->SetRpcId(request->GetRpcId());
		}
		int id = this->mConfig.Index;
#ifdef ONLY_MAIN_THREAD
		this->mComponent->OnMessage(id, request.release(), response.release());
#else
		mongo::Request * req = request.release();
		mongo::Response * resp = response.release();
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(this->mMainContext, [this, self, req, id, resp]
		{
			this->mComponent->OnMessage(id, req, resp);
			delete req;
		});
#endif
	}

	void Client::SendMongoCommand(std::unique_ptr<Request> request)
	{
#ifdef ONLY_MAIN_THREAD
		this->mRequest = std::move(request);
		this->Write(*this->mRequest);
#else
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(context, [this, self, data = request.release()]
		{
			this->mRequest.reset(data);
			this->Write(*data);
		});
#endif
	}

	bool Client::SyncSend(std::unique_ptr<Request> request, mongo::Response& response)
	{
		if(!this->SendSync(*request))
		{
			return false;
		}
		size_t readSize = 0;
		if(!this->RecvSync(sizeof(Head), readSize))
		{
			return false;
		}
		std::istream readStream1(&this->mRecvBuffer);
		{
			int count = response.OnRecvMessage(readStream1, readSize);
			if (!this->RecvSync(count, readSize))
			{
				return false;
			}
			response.OnRecvMessage(readStream1, readSize);
		}
		return true;
	}

	std::unique_ptr<mongo::Response> Client::SyncMongoCommand(std::unique_ptr<Request> request)
	{
		custom::ThreadSync<bool> threadSync;
		std::unique_ptr<mongo::Response> response;
		Asio::Socket & sock = this->mSocket->Get();
		const Asio::Executor & executor = sock.get_executor();
		asio::post(executor, [this, &request, &response, &threadSync]
		{
			response = this->SyncSendMongoCommand(std::move(request));
			threadSync.SetResult(true);
		});
		threadSync.Wait();
		return response;
	}

	std::unique_ptr<Response> Client::SyncSendMongoCommand(std::unique_ptr<Request> request)
	{
		if(!this->SendSync(*request))
		{
			return nullptr;
		}
		return this->ReadResponse();
	}

	std::unique_ptr<Response> Client::ReadResponse()
	{
		size_t readSize = 0;
		if(!this->RecvSync(sizeof(Head), readSize))
		{
			return nullptr;
		}
		std::istream readStream1(&this->mRecvBuffer);
		std::unique_ptr<Response> response = std::make_unique<Response>();
		{
			int count = response->OnRecvMessage(readStream1, readSize);
			if (!this->RecvSync(count, readSize))
			{
				return response;
			}
			response->OnRecvMessage(readStream1, readSize);
		}
		return response;
	}

	void Client::OnConnect(bool result, int count)
	{
		if(!result)
		{
			this->OnResponse(XCode::NetConnectFailure);
			return;
		}
		this->mSocket->SetOption(tcp::OptionType::NoDelay, true);
		this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
		if(!this->Auth(this->mConfig.User, this->mConfig.DB, this->mConfig.Password))
		{
			if(!this->Auth(this->mConfig.User, "admin", this->mConfig.Password))
			{
				this->OnResponse(XCode::NetWorkError);
				return;
			}
		}

		this->ClearSendStream();
		this->ClearRecvStream();
		if(this->mRequest != nullptr)
		{
			this->Write(*this->mRequest);
		}
	}

	bool Client::StartAuthBySha1()
	{
		Asio::Code code;
		if (!this->ConnectSync(code))
		{
			const std::string& addr = this->mConfig.Address;
			CONSOLE_LOG_ERROR("connect mongo server {} fail {}", addr, code.message());
			return false;
		}
		this->mSocket->SetOption(tcp::OptionType::NoDelay, true);
		this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
		if(!this->Auth(this->mConfig.User, this->mConfig.DB, this->mConfig.Password))
		{
			return this->Auth(this->mConfig.User, "admin", this->mConfig.Password);
		}
		return true;
	}

}