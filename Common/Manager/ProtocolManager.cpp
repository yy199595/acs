#include "ProtocolManager.h"
#include <Util/FileHelper.h>
#include <Util/StringHelper.h>
#include <google/protobuf/util/json_util.h>

namespace Sentry
{
				bool ProtocolManager::OnInit()
				{
								std::string path;
								std::vector<std::string> tempArray;
								std::vector<std::string> fileContents;
								SayNoAssertRetFalse_F(this->GetConfig().GetValue("rpc", path));
								SayNoAssertRetFalse_F(FileHelper::ReadTxtFile(path, fileContents));

								for (size_t index = 1; index < fileContents.size(); index++)
								{
												tempArray.clear();
												const std::string &content = fileContents[index];
												StringHelper::SplitString(content, "\t", tempArray);

												unsigned short id = (unsigned short) std::stoi(tempArray[0]);
												const std::string service = tempArray[1];
												const std::string method = tempArray[2];
												const std::string request = tempArray.size() >= 4 ? tempArray[3] : "";
												const std::string response = tempArray.size() >= 5 ? tempArray[4] : "";

												ProtocolConfig *protocol = new ProtocolConfig(id, service, method, request, response);
												if (protocol != nullptr)
												{
																std::string name = service + "." + method;
																this->mProtocolMap.insert(std::make_pair(id, protocol));
																this->mProtocolNameMap.insert(std::make_pair(name, protocol));
												}
								}
								return true;
				}


				const ProtocolConfig *ProtocolManager::GetProtocolConfig(unsigned short id) const
				{
								auto iter = this->mProtocolMap.find(id);
								return iter != this->mProtocolMap.end() ? iter->second : nullptr;
				}

				const ProtocolConfig *
				ProtocolManager::GetProtocolConfig(const std::string &service, const std::string &method) const
				{
								std::string name = service + "." + method;
								auto iter = this->mProtocolNameMap.find(name);
								return iter != this->mProtocolNameMap.end() ? iter->second : nullptr;
				}

				Message *ProtocolManager::CreateMessage(const std::string &name)
				{
								auto iter = this->mProtocolPoolMap.find(name);
								if (iter != this->mProtocolPoolMap.end())
								{
												if (iter->second.size() > 0)
												{
																Message *message = iter->second.front();
																iter->second.pop();
																return message;
												}
								}
								const DescriptorPool *pDescriptorPool = DescriptorPool::generated_pool();
								const Descriptor *pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
								if (pDescriptor == nullptr)
								{
												SayNoDebugError("create pb fail " << name);
												return nullptr;
								}
								MessageFactory *factory = MessageFactory::generated_factory();
								const Message *pMessage = factory->GetPrototype(pDescriptor);

								return pMessage->New();
				}

				Message *ProtocolManager::CreateMessageByJson(const std::string &name, const char *msg, const size_t size)
				{
								Message *message = this->CreateMessage(name);
								if (message != nullptr)
								{
												if (util::JsonStringToMessage(std::string(msg, size), message).ok())
												{
																return message;
												}
												return message;
								}
								return nullptr;
				}

				bool ProtocolManager::DestoryMessage(Message *message)
				{
								if (message == nullptr)
								{
												return true;
								}
								const std::string name = message->GetTypeName();
								std::queue<Message *> &pool = this->mProtocolPoolMap[name];

								if (pool.size() >= 100)
								{
												delete message;
												return true;
								}
								message->Clear();
								pool.push(message);
								return false;
				}

}
