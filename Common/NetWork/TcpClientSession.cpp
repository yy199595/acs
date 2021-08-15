#include"TcpClientSession.h"
#include<Core/Applocation.h>
#include<Util/StringHelper.h>
#include<Manager/NetSessionManager.h>
#include<NetWork/SocketEvent.h>

namespace Sentry
{
				TcpClientSession::TcpClientSession(AsioContext &io, NetSessionManager *manager, SharedTcpSocket socket)
												: mAsioContext(io)
				{
								this->mBinTcpSocket = socket;
								this->mSessionType = SessionClient;
								if (this->mBinTcpSocket != nullptr)
								{
												this->mDispatchManager = manager;
												this->mSocketEndPoint = socket->remote_endpoint();
												this->InitMember(this->mSocketEndPoint.address().to_string(), this->mSocketEndPoint.port());
								}
				}

				TcpClientSession::TcpClientSession(AsioContext &io, NetSessionManager *manager, std::string name, std::string ip,
				                                   unsigned short port)
												: mAsioContext(io)
				{
								asio::error_code ec;
								this->mSessionName = name;
								this->InitMember(ip, port);
								this->mDispatchManager = manager;
								this->mSessionType = SessionNode;
								this->mSocketEndPoint = asio::ip::tcp::endpoint(asio::ip::make_address(mIp, ec), mPort);
								if (ec)
								{
												SayNoDebugError(ec.message());
								}
				}

				void TcpClientSession::InitMember(const std::string &ip, unsigned short port)
				{
								this->mIp = ip;
								this->mPort = port;
								this->mConnectCount = 0;
								this->mRecvBufferSize = 1024;
								this->mRecvMsgBuffer = new char[this->mRecvBufferSize];
								this->mAdress = this->mIp + ":" + std::to_string(this->mPort);
				}

				TcpClientSession::~TcpClientSession()
				{
								delete[]this->mRecvMsgBuffer;
				}

				bool TcpClientSession::IsActive()
				{
								return this->mBinTcpSocket != nullptr && this->mBinTcpSocket->is_open();
				}

				bool TcpClientSession::SendPackage(const shared_ptr<std::string> message)
				{
								if (message == nullptr || message->size() == 0)
								{
												return false;
								}
								if (!this->mBinTcpSocket || !this->mBinTcpSocket->is_open())
								{
												return false;
								}

								mBinTcpSocket->async_send(asio::buffer(message->c_str(), message->size()),
								                          [this, message](const asio::error_code &error_code, std::size_t size) {
												                          if (error_code)
												                          {
																                          this->StartClose();
																                          const char *msg = message->c_str();
																                          this->mDispatchManager->OnSendMessageError(this, msg, size);
												                          }
								                          });
								return true;
				}

				bool TcpClientSession::StartConnect()
				{
								if (this->IsActive() || this->mSessionType != SessionNode)
								{
												return false;
								}
								this->mConnectCount++;
								if (this->mBinTcpSocket == nullptr)
								{
												this->mBinTcpSocket = std::make_shared<AsioTcpSocket>(this->mAsioContext);
								}
								this->mBinTcpSocket->async_connect(this->mSocketEndPoint, [this](const asio::error_code &error_code) {
												if (error_code)
												{
																this->StartClose();
																SayNoDebugWarning("Connect " << this->GetSessionName()
																                             << " fail count = " << this->mConnectCount << " error : "
																                             << error_code.message());
																this->mDispatchManager->OnSessionError(this, Net2MainEventType::SocketConnectFail);
												} else
												{
																this->mConnectCount = 0;
																this->mDispatchManager->OnConnectSuccess(this);
												}
								});
								SayNoDebugLog(this->GetSessionName() << " start connect " << this->mAdress);
								return true;
				}

				void TcpClientSession::StartClose()
				{
								if (this->mBinTcpSocket != nullptr && this->mBinTcpSocket->is_open())
								{
												asio::error_code closeCode;
												this->mBinTcpSocket->shutdown(asio::socket_base::shutdown_send, closeCode);
												this->mBinTcpSocket->shutdown(asio::socket_base::shutdown_receive, closeCode);
												this->mBinTcpSocket->close(closeCode);
								}
								this->mBinTcpSocket = nullptr;
				}

				bool TcpClientSession::StartReceiveMsg()
				{
								if (this->IsActive() == false)
								{
												return false;
								}
								memset(this->mRecvMsgBuffer, 0, this->mRecvBufferSize);
								this->mBinTcpSocket->async_read_some(asio::buffer(this->mRecvMsgBuffer, sizeof(unsigned int)),
								                                     [this](const asio::error_code &error_code, const std::size_t t) {
												                                     if (error_code)
												                                     {
																                                     this->StartClose();
																                                     SayNoDebugError(error_code.message());
																                                     this->mDispatchManager->OnSessionError(this, SocketReceiveFail);
												                                     } else
												                                     {
																                                     size_t packageSize = 0;
																                                     memcpy(&packageSize, this->mRecvMsgBuffer, t);
																                                     this->ReadMessageBody(packageSize);
												                                     }
								                                     });
								return true;
				}

				void TcpClientSession::ReadMessageBody(const size_t allSize)
				{
								char *nMessageBuffer = this->mRecvMsgBuffer;
								if (allSize > this->mRecvBufferSize)
								{
												nMessageBuffer = new char[allSize];
												if (nMessageBuffer == nullptr)
												{
																this->StartClose();
																this->mDispatchManager->OnSessionError(this, SocketReceiveFail);
																return;
												}
								}

								this->mBinTcpSocket->async_read_some(asio::buffer(nMessageBuffer, allSize),
								                                     [this, nMessageBuffer](const asio::error_code &error_code,
								                                                            const std::size_t messageSize) {
												                                     if (error_code)
												                                     {
																                                     this->StartClose();
																                                     SayNoDebugError(error_code.message());
																                                     this->mDispatchManager->OnSessionError(this, SocketReceiveFail);
												                                     } else
												                                     {
																                                     if (!this->mDispatchManager->OnRecvMessage(this, nMessageBuffer,
																                                                                                messageSize))
																                                     {
																				                                     this->StartClose();
																				                                     SayNoDebugError("parse message fail close socket " << mAdress);
																                                     }
												                                     }
												                                     if (nMessageBuffer != this->mRecvMsgBuffer)
												                                     {
																                                     delete[]nMessageBuffer;
												                                     }
								                                     });
				}
}