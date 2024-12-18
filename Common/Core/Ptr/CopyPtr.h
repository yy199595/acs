#pragma once
#include <iostream>
#include <memory>
#include <utility>

namespace custom {

	template<typename T>
	class shared_ptr {
	private:
		struct ControlBlock {
			T* mPtr;  // 指向对象的指针
			int mRefCount;  // 引用计数

			explicit ControlBlock(T* ptr)
					: mPtr(ptr), mRefCount(1) {}
		};

	public:
		// 默认构造函数
		shared_ptr() : mControlBlock(nullptr) {}

		// 构造函数，接收原始指针
		explicit shared_ptr(T* ptr) {
			mControlBlock = new ControlBlock(ptr);
		}

		// 拷贝构造函数
		shared_ptr(const shared_ptr& other) {
			mControlBlock = other.mControlBlock;
			if (mControlBlock) {
				mControlBlock->mRefCount++;
			}
		}

		// 移动构造函数
		shared_ptr(shared_ptr&& other) noexcept {
			mControlBlock = other.mControlBlock;
			other.mControlBlock = nullptr;
		}

		// 析构函数
		~shared_ptr() {
			reset();
		}

		// 拷贝赋值运算符
		shared_ptr& operator=(const shared_ptr& other) {
			if (this != &other) {
				reset();
				mControlBlock = other.mControlBlock;
				if (mControlBlock) {
					mControlBlock->mRefCount++;
				}
			}
			return *this;
		}

		// 移动赋值运算符
		shared_ptr& operator=(shared_ptr&& other) noexcept {
			if (this != &other) {
				reset();
				mControlBlock = other.mControlBlock;
				other.mControlBlock = nullptr;
			}
			return *this;
		}

		// 解引用运算符
		T& operator*() const {
			return *(mControlBlock->mPtr);
		}

		// 指针访问运算符
		T* operator->() const {
			return mControlBlock->mPtr;
		}

		// 重置 shared_ptr，减少引用计数
		void reset() {
			if (mControlBlock) {
				mControlBlock->mRefCount--;
				if (mControlBlock->mRefCount == 0) {
					delete mControlBlock->mPtr;
					delete mControlBlock;
				}
				mControlBlock = nullptr;
			}
		}

		// 获取原始指针
		T* get() const {
			return mControlBlock ? mControlBlock->mPtr : nullptr;
		}

		// 判断指针是否有效
		bool is_null() const {
			return mControlBlock == nullptr;
		}

		// 获取引用计数
		int use_count() const {
			return mControlBlock ? mControlBlock->mRefCount : 0;
		}

	private:
		ControlBlock* mControlBlock;  // 控制块，包含指针和引用计数
	};

	// make_shared 工厂函数
	template<typename T, typename... Args>
	shared_ptr<T> make_shared(Args&&... args) {
		shared_ptr<T> p(new T(std::forward<Args>(args)...));
		return p;
	}

} // namespace custom
