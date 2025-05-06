//
// Created by 64658 on 2025/3/28.
//

#ifndef APP_RECORDINFO_H
#define APP_RECORDINFO_H

namespace record
{
	struct Info
	{
		unsigned int timeout = 0; //超时数量
		unsigned int sum = 0; //总处理数量
		unsigned int wait = 0; //等待处理数量
		unsigned int conn = 0; //连接数量
		unsigned int success = 0; //成功数量
		unsigned int failure = 0; //失败数量
	};
}

#endif //APP_RECORDINFO_H
