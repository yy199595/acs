#pragma once
#include<CommonDefine/CommonTypeDef.h>
namespace SoEasy
{
	template<typename T>
	class NumberBuilder
	{
	public:
		NumberBuilder(const T num = 100) : mIndex(num) {}
		inline const T GetNumber();
		inline void ReceiveNumber(const T num);
	private:
		T mIndex;
		SayNoQueue<T> mNumberQueue;
	};
	template<typename T>
	inline const T NumberBuilder<T>::GetNumber()
	{
		if (!this->mNumberQueue.empty())
		{
			T number = this->mNumberQueue.front();
			this->mNumberQueue.pop();
			return number;
		}
		return this->mIndex++;
	}
	template<typename T>
	inline void NumberBuilder<T>::ReceiveNumber(const T num)
	{
		this->mNumberQueue.push(num);
	}
}