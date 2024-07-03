#pragma once
#include<memory>
namespace custom
{
	template<typename T>
	class shared_ptr
	{
	public:
		shared_ptr() : mPtr(nullptr), mRefCount(nullptr) {}

		explicit shared_ptr(T* ptr) {
			mPtr = ptr;
			mRefCount = new int(1);
		}

		shared_ptr(const shared_ptr& other) {
			mPtr = other.mPtr;
			mRefCount = other.mRefCount;
			(*mRefCount)++;
		}

		shared_ptr(shared_ptr&& other) noexcept {
			mPtr = other.mPtr;
			mRefCount = other.mRefCount;
			other.mPtr = nullptr;
			other.mRefCount = nullptr;
		}

		~shared_ptr() { reset(); }

		shared_ptr& operator=(const shared_ptr& other) {
			if (this != &other) {
				reset();
				mPtr = other.mPtr;
				mRefCount = other.mRefCount;
				(*mRefCount)++;
			}
			return *this;
		}

		shared_ptr& operator=(shared_ptr&& other) noexcept {
			if (this != &other) {
				reset();
				mPtr = other.mPtr;
				mRefCount = other.mRefCount;
				other.mPtr = nullptr;
				other.mRefCount = nullptr;
			}
			return *this;
		}

		T* operator->() { return mPtr; }

		T& operator*() { return *mPtr; }

		void reset() {
			if (mRefCount != nullptr) {
				(*mRefCount)--;
				if (*mRefCount == 0) {
					delete mPtr;
					delete mRefCount;
				}
				mPtr = nullptr;
				mRefCount = nullptr;
			}
		}

	private:
		T* mPtr;
		int* mRefCount;
	};

	template<typename T, typename ... Args>
	inline shared_ptr<T> make_shared(Args && ... args)
	{
		shared_ptr<T> p(new T(std::forward<Args>(args)...));
		return std::move(p);
	}
}
