//
// Created by 强哥 on 2023/3/20.
//

#ifndef APP_COMMON_CORE_QUEUE_LOOKFREEQUEUE_H
#define APP_COMMON_CORE_QUEUE_LOOKFREEQUEUE_H
#include <atomic>
#include <queue>
namespace Sentry
{
	template<typename T>
	class LockFreeQueue
	{
	public:
		LockFreeQueue() = default;

		void Push(const T& value)
		{
			Node* node = new Node(value);
			Node* tail = tail_.load(std::memory_order_relaxed);
			do
			{
				node->next = nullptr;
				tail_.compare_exchange_weak(tail, node, std::memory_order_release, std::memory_order_relaxed);
			} while (!tail_.compare_exchange_weak(tail, node, std::memory_order_release, std::memory_order_relaxed));
			if (!tail)
			{
				head_.store(node, std::memory_order_release);
			}
			else
			{
				tail->next = node;
			}
		}

		bool Pop(T& value)
		{
			Node* head = head_.load(std::memory_order_relaxed);
			do
			{
				if (!head)
				{
					return false;
				}
			} while (!head_.compare_exchange_weak(head, head->next, std::memory_order_release, std::memory_order_relaxed));
			value = head->value;
			delete head;
			return true;
		}

	private:
		struct Node
		{
			T value;
			Node* next;
			Node(const T& value) : value(value), next(nullptr)
			{
			}
		};

		std::atomic<Node*> head_{ nullptr };
		std::atomic<Node*> tail_{ nullptr };
	};
}
#endif //APP_COMMON_CORE_QUEUE_LOOKFREEQUEUE_H
