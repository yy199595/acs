
#pragma once

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

#ifdef MCDBGQ_USE_RELACY
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif
#endif

#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif

#ifdef MCDBGQ_USE_RELACY
#include "relacy/relacy_std.hpp"
#include "relacy_shims.h"

#undef new
#undef delete
#undef malloc
#undef free
#else
#include <atomic>		// Requires C++11. Sorry VS2010.
#include <cassert>
#endif
#include <cstddef>              // for max_align_t
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <limits>
#include <climits>		// for CHAR_BIT
#include <array>
#include <thread>		// partly for __WINPTHREADS_VERSION if on MinGW-w64 w/ POSIX threading


namespace MultiThread 
{ 
	namespace details 
	{
		template<typename thread_id_t> struct thread_id_converter 
		{
			typedef thread_id_t thread_id_numeric_size_t;
			typedef thread_id_t thread_id_hash_t;
			static thread_id_hash_t prehash(thread_id_t const& x) { return x; }
		};
	} }

#if defined(MCDBGQ_USE_RELACY)
namespace MultiThread 
{ 
	namespace details 
	{
		typedef std::uint32_t thread_id_t;
		static const thread_id_t invalid_thread_id  = 0xFFFFFFFFU;
		static const thread_id_t invalid_thread_id2 = 0xFFFFFFFEU;
		static inline thread_id_t thread_id() { return rl::thread_index(); }
	} 
}
#elif defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)

extern "C" __declspec(dllimport) unsigned long __stdcall GetCurrentThreadId(void);
namespace MultiThread 
{ 
	namespace details 
	{
		static_assert(sizeof(unsigned long) == sizeof(std::uint32_t), "Expected size of unsigned long to be 32 bits on Windows");
		typedef std::uint32_t thread_id_t;
		static const thread_id_t invalid_thread_id  = 0;			
		static const thread_id_t invalid_thread_id2 = 0xFFFFFFFFU;
		static inline thread_id_t thread_id() { return static_cast<thread_id_t>(::GetCurrentThreadId()); }
	} 
}
#elif defined(__arm__) || defined(_M_ARM) || defined(__aarch64__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
namespace MultiThread { namespace details 
{
	static_assert(sizeof(std::thread::id) == 4 || sizeof(std::thread::id) == 8, "std::thread::id is expected to be either 4 or 8 bytes");

	typedef std::thread::id thread_id_t;
	static const thread_id_t invalid_thread_id;         // Default ctor creates invalid ID

	static inline thread_id_t thread_id() { return std::this_thread::get_id(); }

	template<std::size_t> struct thread_id_size { };
	template<> struct thread_id_size<4> { typedef std::uint32_t numeric_t; };
	template<> struct thread_id_size<8> { typedef std::uint64_t numeric_t; };

	template<> struct thread_id_converter<thread_id_t> {
		typedef thread_id_size<sizeof(thread_id_t)>::numeric_t thread_id_numeric_size_t;
#ifndef __APPLE__
		typedef std::size_t thread_id_hash_t;
#else
		typedef thread_id_numeric_size_t thread_id_hash_t;
#endif

		static thread_id_hash_t prehash(thread_id_t const& x)
		{
#ifndef __APPLE__
			return std::hash<std::thread::id>()(x);
#else
			return *reinterpret_cast<thread_id_hash_t const*>(&x);
#endif
		}
	};
} 
}
#else

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#define MOODYCAMEL_THREADLOCAL __thread
#elif defined(_MSC_VER)
#define MOODYCAMEL_THREADLOCAL __declspec(thread)
#else
// Assume C++11 compliant compiler
#define MOODYCAMEL_THREADLOCAL thread_local
#endif
namespace MultiThread 
{ 
	namespace details
	{
		typedef std::uintptr_t thread_id_t;
		static const thread_id_t invalid_thread_id  = 0;		
		static const thread_id_t invalid_thread_id2 = 1;
		static inline thread_id_t thread_id() { static MOODYCAMEL_THREADLOCAL int x; return reinterpret_cast<thread_id_t>(&x); }
	} 
}
#endif

// Exceptions
#ifndef MOODYCAMEL_EXCEPTIONS_ENABLED
#if (defined(_MSC_VER) && defined(_CPPUNWIND)) || (defined(__GNUC__) && defined(__EXCEPTIONS)) || (!defined(_MSC_VER) && !defined(__GNUC__))
#define MOODYCAMEL_EXCEPTIONS_ENABLED
#endif
#endif
#ifdef MOODYCAMEL_EXCEPTIONS_ENABLED
#define MOODYCAMEL_TRY try
#define MOODYCAMEL_CATCH(...) catch(__VA_ARGS__)
#define MOODYCAMEL_RETHROW throw
#define MOODYCAMEL_THROW(expr) throw (expr)
#else
#define MOODYCAMEL_TRY if (true)
#define MOODYCAMEL_CATCH(...) else if (false)
#define MOODYCAMEL_RETHROW
#define MOODYCAMEL_THROW(expr)
#endif

#ifndef MOODYCAMEL_NOEXCEPT
#if !defined(MOODYCAMEL_EXCEPTIONS_ENABLED)
#define MOODYCAMEL_NOEXCEPT
#define MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr) true
#define MOODYCAMEL_NOEXCEPT_ASSIGN(type, valueType, expr) true
#elif defined(_MSC_VER) && defined(_NOEXCEPT) && _MSC_VER < 1800


#define MOODYCAMEL_NOEXCEPT _NOEXCEPT
#define MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr) (std::is_rvalue_reference<valueType>::value && std::is_move_constructible<type>::value ? std::is_trivially_move_constructible<type>::value : std::is_trivially_copy_constructible<type>::value)
#define MOODYCAMEL_NOEXCEPT_ASSIGN(type, valueType, expr) ((std::is_rvalue_reference<valueType>::value && std::is_move_assignable<type>::value ? std::is_trivially_move_assignable<type>::value || std::is_nothrow_move_assignable<type>::value : std::is_trivially_copy_assignable<type>::value || std::is_nothrow_copy_assignable<type>::value) && MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr))
#elif defined(_MSC_VER) && defined(_NOEXCEPT) && _MSC_VER < 1900
#define MOODYCAMEL_NOEXCEPT _NOEXCEPT
#define MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr) (std::is_rvalue_reference<valueType>::value && std::is_move_constructible<type>::value ? std::is_trivially_move_constructible<type>::value || std::is_nothrow_move_constructible<type>::value : std::is_trivially_copy_constructible<type>::value || std::is_nothrow_copy_constructible<type>::value)
#define MOODYCAMEL_NOEXCEPT_ASSIGN(type, valueType, expr) ((std::is_rvalue_reference<valueType>::value && std::is_move_assignable<type>::value ? std::is_trivially_move_assignable<type>::value || std::is_nothrow_move_assignable<type>::value : std::is_trivially_copy_assignable<type>::value || std::is_nothrow_copy_assignable<type>::value) && MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr))
#else
#define MOODYCAMEL_NOEXCEPT noexcept
#define MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr) noexcept(expr)
#define MOODYCAMEL_NOEXCEPT_ASSIGN(type, valueType, expr) noexcept(expr)
#endif
#endif

#ifndef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
#ifdef MCDBGQ_USE_RELACY
#define MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
#else

#if (!defined(_MSC_VER) || _MSC_VER >= 1900) && (!defined(__MINGW32__) && !defined(__MINGW64__) || !defined(__WINPTHREADS_VERSION)) && (!defined(__GNUC__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)) && (!defined(__APPLE__) || !TARGET_OS_IPHONE) && !defined(__arm__) && !defined(_M_ARM) && !defined(__aarch64__)

#endif
#endif
#endif


#ifndef MOODYCAMEL_DELETE_FUNCTION
#if defined(_MSC_VER) && _MSC_VER < 1800
#define MOODYCAMEL_DELETE_FUNCTION
#else
#define MOODYCAMEL_DELETE_FUNCTION = delete
#endif
#endif

namespace MultiThread { namespace details 
{
#if defined(__GNUC__)
	static inline bool (likely)(bool x) { return __builtin_expect((x), true); }
	static inline bool (unlikely)(bool x) { return __builtin_expect((x), false); }
#else
	static inline bool (likely)(bool x) { return x; }
	static inline bool (unlikely)(bool x) { return x; }
#endif
} }

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
#include "internal/concurrentqueue_internal_debug.h"
#endif

namespace MultiThread 
{
	namespace details 
	{
		template<typename T>
		struct const_numeric_max {
			static_assert(std::is_integral<T>::value, "const_numeric_max can only be used with integers");
			static const T value = std::numeric_limits<T>::is_signed
				? (static_cast<T>(1) << (sizeof(T) * CHAR_BIT - 1)) - static_cast<T>(1)
				: static_cast<T>(-1);
		};

#if defined(__GLIBCXX__)
		typedef ::max_align_t std_max_align_t;
#else
		typedef std::max_align_t std_max_align_t;
#endif

		typedef union {
			std_max_align_t x;
			long long y;
			void* z;
		} max_align_t;
	}


	struct ConcurrentQueueDefaultTraits
	{
		typedef std::size_t size_t;
		typedef std::size_t index_t;
		static const size_t BLOCK_SIZE = 32;
		static const size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = 32;
		static const size_t EXPLICIT_INITIAL_INDEX_SIZE = 32;
		static const size_t IMPLICIT_INITIAL_INDEX_SIZE = 32;
		static const size_t INITIAL_IMPLICIT_PRODUCER_HASH_SIZE = 32;
		static const std::uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = 256;
		static const size_t MAX_SUBQUEUE_SIZE = details::const_numeric_max<size_t>::value;


#ifndef MCDBGQ_USE_RELACY

#if defined(malloc) || defined(free)
		static inline void* WORKAROUND_malloc(size_t size) { return malloc(size); }
		static inline void WORKAROUND_free(void* ptr) { return free(ptr); }
		static inline void* (malloc)(size_t size) { return WORKAROUND_malloc(size); }
		static inline void (free)(void* ptr) { return WORKAROUND_free(ptr); }
#else
		static inline void* malloc(size_t size) { return std::malloc(size); }
		static inline void free(void* ptr) { return std::free(ptr); }
#endif
#else
		static inline void* malloc(size_t size) { return rl::rl_malloc(size, $); }
		static inline void free(void* ptr) { return rl::rl_free(ptr, $); }
#endif
	};

	struct ProducerToken;
	struct ConsumerToken;

	template<typename T, typename Traits> class ConcurrentQueue;
	template<typename T, typename Traits> class BlockingConcurrentQueue;
	class ConcurrentQueueTests;


	namespace details
	{
		struct ConcurrentQueueProducerTypelessBase
		{
			ConcurrentQueueProducerTypelessBase* next;
			std::atomic<bool> inactive;
			ProducerToken* token;

			ConcurrentQueueProducerTypelessBase()
				: next(nullptr), inactive(false), token(nullptr)
			{
			}
		};

		template<bool use32> struct _hash_32_or_64 {
			static inline std::uint32_t hash(std::uint32_t h)
			{
				h ^= h >> 16;
				h *= 0x85ebca6b;
				h ^= h >> 13;
				h *= 0xc2b2ae35;
				return h ^ (h >> 16);
			}
		};
		template<> struct _hash_32_or_64<1> {
			static inline std::uint64_t hash(std::uint64_t h)
			{
				h ^= h >> 33;
				h *= 0xff51afd7ed558ccd;
				h ^= h >> 33;
				h *= 0xc4ceb9fe1a85ec53;
				return h ^ (h >> 33);
			}
		};
		template<std::size_t size> struct hash_32_or_64 : public _hash_32_or_64<(size > 4)> {  };

		static inline size_t hash_thread_id(thread_id_t id)
		{
			static_assert(sizeof(thread_id_t) <= 8, "Expected a platform where thread IDs are at most 64-bit values");
			return static_cast<size_t>(hash_32_or_64<sizeof(thread_id_converter<thread_id_t>::thread_id_hash_t)>::hash(
				thread_id_converter<thread_id_t>::prehash(id)));
		}

		template<typename T>
		static inline bool circular_less_than(T a, T b)
		{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4554)
#endif
			static_assert(std::is_integral<T>::value && !std::numeric_limits<T>::is_signed, "circular_less_than is intended to be used only with unsigned integer types");
			return static_cast<T>(a - b) > static_cast<T>(static_cast<T>(1) << static_cast<T>(sizeof(T) * CHAR_BIT - 1));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
		}

		template<typename U>
		static inline char* align_for(char* ptr)
		{
			const std::size_t alignment = std::alignment_of<U>::value;
			return ptr + (alignment - (reinterpret_cast<std::uintptr_t>(ptr) % alignment)) % alignment;
		}

		template<typename T>
		static inline T ceil_to_pow_2(T x)
		{
			static_assert(std::is_integral<T>::value && !std::numeric_limits<T>::is_signed, "ceil_to_pow_2 is intended to be used only with unsigned integer types");
			--x;
			x |= x >> 1;
			x |= x >> 2;
			x |= x >> 4;
			for (std::size_t i = 1; i < sizeof(T); i <<= 1) {
				x |= x >> (i << 3);
			}
			++x;
			return x;
		}

		template<typename T>
		static inline void swap_relaxed(std::atomic<T>& left, std::atomic<T>& right)
		{
			T temp = std::move(left.load(std::memory_order_relaxed));
			left.store(std::move(right.load(std::memory_order_relaxed)), std::memory_order_relaxed);
			right.store(std::move(temp), std::memory_order_relaxed);
		}

		template<typename T>
		static inline T const& nomove(T const& x)
		{
			return x;
		}

		template<bool Enable>
		struct nomove_if
		{
			template<typename T>
			static inline T const& eval(T const& x)
			{
				return x;
			}
		};

		template<>
		struct nomove_if<false>
		{
			template<typename U>
			static inline auto eval(U&& x)
				-> decltype(std::forward<U>(x))
			{
				return std::forward<U>(x);
			}
		};

		template<typename It>
		static inline auto deref_noexcept(It& it) MOODYCAMEL_NOEXCEPT -> decltype(*it)
		{
			return *it;
		}

#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
		template<typename T> struct is_trivially_destructible : std::is_trivially_destructible<T> { };
#else
		template<typename T> struct is_trivially_destructible : std::has_trivial_destructor<T> { };
#endif

#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
#ifdef MCDBGQ_USE_RELACY
		typedef RelacyThreadExitListener ThreadExitListener;
		typedef RelacyThreadExitNotifier ThreadExitNotifier;
#else
		struct ThreadExitListener
		{
			typedef void (*callback_t)(void*);
			callback_t callback;
			void* userData;

			ThreadExitListener* next;
		};


		class ThreadExitNotifier
		{
		public:
			static void subscribe(ThreadExitListener* listener)
			{
				auto& tlsInst = instance();
				listener->next = tlsInst.tail;
				tlsInst.tail = listener;
			}

			static void unsubscribe(ThreadExitListener* listener)
			{
				auto& tlsInst = instance();
				ThreadExitListener** prev = &tlsInst.tail;
				for (auto ptr = tlsInst.tail; ptr != nullptr; ptr = ptr->next) {
					if (ptr == listener) {
						*prev = ptr->next;
						break;
					}
					prev = &ptr->next;
				}
			}

		private:
			ThreadExitNotifier() : tail(nullptr) { }
			ThreadExitNotifier(ThreadExitNotifier const&) MOODYCAMEL_DELETE_FUNCTION;
			ThreadExitNotifier& operator=(ThreadExitNotifier const&) MOODYCAMEL_DELETE_FUNCTION;

			~ThreadExitNotifier()
			{
				// This thread is about to exit, let everyone know!
				assert(this == &instance() && "If this assert fails, you likely have a buggy compiler! Change the preprocessor conditions such that MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED is no longer defined.");
				for (auto ptr = tail; ptr != nullptr; ptr = ptr->next) {
					ptr->callback(ptr->userData);
				}
			}

			// Thread-local
			static inline ThreadExitNotifier& instance()
			{
				static thread_local ThreadExitNotifier notifier;
				return notifier;
			}

		private:
			ThreadExitListener* tail;
		};
#endif
#endif

		template<typename T> struct static_is_lock_free_num { enum { value = 0 }; };
		template<> struct static_is_lock_free_num<signed char> { enum { value = ATOMIC_CHAR_LOCK_FREE }; };
		template<> struct static_is_lock_free_num<short> { enum { value = ATOMIC_SHORT_LOCK_FREE }; };
		template<> struct static_is_lock_free_num<int> { enum { value = ATOMIC_INT_LOCK_FREE }; };
		template<> struct static_is_lock_free_num<long> { enum { value = ATOMIC_LONG_LOCK_FREE }; };
		template<> struct static_is_lock_free_num<long long> { enum { value = ATOMIC_LLONG_LOCK_FREE }; };
		template<typename T> struct static_is_lock_free : static_is_lock_free_num<typename std::make_signed<T>::type> {  };
		template<> struct static_is_lock_free<bool> { enum { value = ATOMIC_BOOL_LOCK_FREE }; };
		template<typename U> struct static_is_lock_free<U*> { enum { value = ATOMIC_POINTER_LOCK_FREE }; };
	}


	struct ProducerToken
	{
		template<typename T, typename Traits>
		explicit ProducerToken(ConcurrentQueue<T, Traits>& queue);

		template<typename T, typename Traits>
		explicit ProducerToken(BlockingConcurrentQueue<T, Traits>& queue);

		ProducerToken(ProducerToken&& other) MOODYCAMEL_NOEXCEPT
			: producer(other.producer)
		{
			other.producer = nullptr;
			if (producer != nullptr) {
				producer->token = this;
			}
		}

		inline ProducerToken& operator=(ProducerToken&& other) MOODYCAMEL_NOEXCEPT
		{
			swap(other);
			return *this;
		}

		void swap(ProducerToken& other) MOODYCAMEL_NOEXCEPT
		{
			std::swap(producer, other.producer);
			if (producer != nullptr) {
				producer->token = this;
			}
			if (other.producer != nullptr) {
				other.producer->token = &other;
			}
		}

		inline bool valid() const { return producer != nullptr; }

		~ProducerToken()
		{
			if (producer != nullptr) {
				producer->token = nullptr;
				producer->inactive.store(true, std::memory_order_release);
			}
		}

		// Disable copying and assignment
		ProducerToken(ProducerToken const&) MOODYCAMEL_DELETE_FUNCTION;
		ProducerToken& operator=(ProducerToken const&) MOODYCAMEL_DELETE_FUNCTION;

	private:
		template<typename T, typename Traits> friend class ConcurrentQueue;
		friend class ConcurrentQueueTests;

	protected:
		details::ConcurrentQueueProducerTypelessBase* producer;
	};


	struct ConsumerToken
	{
		template<typename T, typename Traits>
		explicit ConsumerToken(ConcurrentQueue<T, Traits>& q);

		template<typename T, typename Traits>
		explicit ConsumerToken(BlockingConcurrentQueue<T, Traits>& q);

		ConsumerToken(ConsumerToken&& other) MOODYCAMEL_NOEXCEPT
			: initialOffset(other.initialOffset), lastKnownGlobalOffset(other.lastKnownGlobalOffset), itemsConsumedFromCurrent(other.itemsConsumedFromCurrent), currentProducer(other.currentProducer), desiredProducer(other.desiredProducer)
		{
		}

		inline ConsumerToken& operator=(ConsumerToken&& other) MOODYCAMEL_NOEXCEPT
		{
			swap(other);
			return *this;
		}

		void swap(ConsumerToken& other) MOODYCAMEL_NOEXCEPT
		{
			std::swap(initialOffset, other.initialOffset);
			std::swap(lastKnownGlobalOffset, other.lastKnownGlobalOffset);
			std::swap(itemsConsumedFromCurrent, other.itemsConsumedFromCurrent);
			std::swap(currentProducer, other.currentProducer);
			std::swap(desiredProducer, other.desiredProducer);
		}

		// Disable copying and assignment
		ConsumerToken(ConsumerToken const&) MOODYCAMEL_DELETE_FUNCTION;
		ConsumerToken& operator=(ConsumerToken const&) MOODYCAMEL_DELETE_FUNCTION;

	private:
		template<typename T, typename Traits> friend class ConcurrentQueue;
		friend class ConcurrentQueueTests;

	private: // but shared with ConcurrentQueue
		std::uint32_t initialOffset;
		std::uint32_t lastKnownGlobalOffset;
		std::uint32_t itemsConsumedFromCurrent;
		details::ConcurrentQueueProducerTypelessBase* currentProducer;
		details::ConcurrentQueueProducerTypelessBase* desiredProducer;
	};


	template<typename T, typename Traits>
	inline void swap(typename ConcurrentQueue<T, Traits>::ImplicitProducerKVP& a, typename ConcurrentQueue<T, Traits>::ImplicitProducerKVP& b) MOODYCAMEL_NOEXCEPT;


	template<typename T, typename Traits = ConcurrentQueueDefaultTraits>
	class ConcurrentQueue
	{
	public:
		typedef ::MultiThread::ProducerToken producer_token_t;
		typedef ::MultiThread::ConsumerToken consumer_token_t;

		typedef typename Traits::index_t index_t;
		typedef typename Traits::size_t size_t;

		static const size_t BLOCK_SIZE = static_cast<size_t>(Traits::BLOCK_SIZE);
		static const size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = static_cast<size_t>(Traits::EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD);
		static const size_t EXPLICIT_INITIAL_INDEX_SIZE = static_cast<size_t>(Traits::EXPLICIT_INITIAL_INDEX_SIZE);
		static const size_t IMPLICIT_INITIAL_INDEX_SIZE = static_cast<size_t>(Traits::IMPLICIT_INITIAL_INDEX_SIZE);
		static const size_t INITIAL_IMPLICIT_PRODUCER_HASH_SIZE = static_cast<size_t>(Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE);
		static const std::uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = static_cast<std::uint32_t>(Traits::EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4307)
#pragma warning(disable: 4309)
#endif
		static const size_t MAX_SUBQUEUE_SIZE = (details::const_numeric_max<size_t>::value - static_cast<size_t>(Traits::MAX_SUBQUEUE_SIZE) < BLOCK_SIZE) ? details::const_numeric_max<size_t>::value : ((static_cast<size_t>(Traits::MAX_SUBQUEUE_SIZE) + (BLOCK_SIZE - 1)) / BLOCK_SIZE * BLOCK_SIZE);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

		static_assert(!std::numeric_limits<size_t>::is_signed && std::is_integral<size_t>::value, "Traits::size_t must be an unsigned integral type");
		static_assert(!std::numeric_limits<index_t>::is_signed && std::is_integral<index_t>::value, "Traits::index_t must be an unsigned integral type");
		static_assert(sizeof(index_t) >= sizeof(size_t), "Traits::index_t must be at least as wide as Traits::size_t");
		static_assert((BLOCK_SIZE > 1) && !(BLOCK_SIZE & (BLOCK_SIZE - 1)), "Traits::BLOCK_SIZE must be a power of 2 (and at least 2)");
		static_assert((EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD > 1) && !(EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD & (EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD - 1)), "Traits::EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD must be a power of 2 (and greater than 1)");
		static_assert((EXPLICIT_INITIAL_INDEX_SIZE > 1) && !(EXPLICIT_INITIAL_INDEX_SIZE & (EXPLICIT_INITIAL_INDEX_SIZE - 1)), "Traits::EXPLICIT_INITIAL_INDEX_SIZE must be a power of 2 (and greater than 1)");
		static_assert((IMPLICIT_INITIAL_INDEX_SIZE > 1) && !(IMPLICIT_INITIAL_INDEX_SIZE & (IMPLICIT_INITIAL_INDEX_SIZE - 1)), "Traits::IMPLICIT_INITIAL_INDEX_SIZE must be a power of 2 (and greater than 1)");
		static_assert((INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) || !(INITIAL_IMPLICIT_PRODUCER_HASH_SIZE & (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE - 1)), "Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE must be a power of 2");
		static_assert(INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 || INITIAL_IMPLICIT_PRODUCER_HASH_SIZE >= 1, "Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE must be at least 1 (or 0 to disable implicit enqueueing)");

	public:

		explicit ConcurrentQueue(size_t capacity = 6 * BLOCK_SIZE)
			: producerListTail(nullptr),
			producerCount(0),
			initialBlockPoolIndex(0),
			nextExplicitConsumerId(0),
			globalExplicitConsumerOffset(0)
		{
			implicitProducerHashResizeInProgress.clear(std::memory_order_relaxed);
			populate_initial_implicit_producer_hash();
			populate_initial_block_list(capacity / BLOCK_SIZE + ((capacity & (BLOCK_SIZE - 1)) == 0 ? 0 : 1));

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
			explicitProducers.store(nullptr, std::memory_order_relaxed);
			implicitProducers.store(nullptr, std::memory_order_relaxed);
#endif
		}

		ConcurrentQueue(size_t minCapacity, size_t maxExplicitProducers, size_t maxImplicitProducers)
			: producerListTail(nullptr),
			producerCount(0),
			initialBlockPoolIndex(0),
			nextExplicitConsumerId(0),
			globalExplicitConsumerOffset(0)
		{
			implicitProducerHashResizeInProgress.clear(std::memory_order_relaxed);
			populate_initial_implicit_producer_hash();
			size_t blocks = (((minCapacity + BLOCK_SIZE - 1) / BLOCK_SIZE) - 1) * (maxExplicitProducers + 1) + 2 * (maxExplicitProducers + maxImplicitProducers);
			populate_initial_block_list(blocks);

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
			explicitProducers.store(nullptr, std::memory_order_relaxed);
			implicitProducers.store(nullptr, std::memory_order_relaxed);
#endif
		}

		~ConcurrentQueue()
		{
			auto ptr = producerListTail.load(std::memory_order_relaxed);
			while (ptr != nullptr) {
				auto next = ptr->next_prod();
				if (ptr->token != nullptr) {
					ptr->token->producer = nullptr;
				}
				destroy(ptr);
				ptr = next;
			}
			if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE != 0) {
				auto hash = implicitProducerHash.load(std::memory_order_relaxed);
				while (hash != nullptr) {
					auto prev = hash->prev;
					if (prev != nullptr) {
						for (size_t i = 0; i != hash->capacity; ++i) {
							hash->entries[i].~ImplicitProducerKVP();
						}
						hash->~ImplicitProducerHash();
						(Traits::free)(hash);
					}
					hash = prev;
				}
			}

			// Destroy global free list
			auto block = freeList.head_unsafe();
			while (block != nullptr) {
				auto next = block->freeListNext.load(std::memory_order_relaxed);
				if (block->dynamicallyAllocated) {
					destroy(block);
				}
				block = next;
			}

			// Destroy initial free list
			destroy_array(initialBlockPool, initialBlockPoolSize);
		}

		// Disable copying and copy assignment
		ConcurrentQueue(ConcurrentQueue const&) MOODYCAMEL_DELETE_FUNCTION;
		ConcurrentQueue& operator=(ConcurrentQueue const&) MOODYCAMEL_DELETE_FUNCTION;

		ConcurrentQueue(ConcurrentQueue&& other) MOODYCAMEL_NOEXCEPT
			: producerListTail(other.producerListTail.load(std::memory_order_relaxed)),
			producerCount(other.producerCount.load(std::memory_order_relaxed)),
			initialBlockPoolIndex(other.initialBlockPoolIndex.load(std::memory_order_relaxed)),
			initialBlockPool(other.initialBlockPool),
			initialBlockPoolSize(other.initialBlockPoolSize),
			freeList(std::move(other.freeList)),
			nextExplicitConsumerId(other.nextExplicitConsumerId.load(std::memory_order_relaxed)),
			globalExplicitConsumerOffset(other.globalExplicitConsumerOffset.load(std::memory_order_relaxed))
		{
			// Move the other one into this, and leave the other one as an empty queue
			implicitProducerHashResizeInProgress.clear(std::memory_order_relaxed);
			populate_initial_implicit_producer_hash();
			swap_implicit_producer_hashes(other);

			other.producerListTail.store(nullptr, std::memory_order_relaxed);
			other.producerCount.store(0, std::memory_order_relaxed);
			other.nextExplicitConsumerId.store(0, std::memory_order_relaxed);
			other.globalExplicitConsumerOffset.store(0, std::memory_order_relaxed);

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
			explicitProducers.store(other.explicitProducers.load(std::memory_order_relaxed), std::memory_order_relaxed);
			other.explicitProducers.store(nullptr, std::memory_order_relaxed);
			implicitProducers.store(other.implicitProducers.load(std::memory_order_relaxed), std::memory_order_relaxed);
			other.implicitProducers.store(nullptr, std::memory_order_relaxed);
#endif

			other.initialBlockPoolIndex.store(0, std::memory_order_relaxed);
			other.initialBlockPoolSize = 0;
			other.initialBlockPool = nullptr;

			reown_producers();
		}

		inline ConcurrentQueue& operator=(ConcurrentQueue&& other) MOODYCAMEL_NOEXCEPT
		{
			return swap_internal(other);
		}

		inline void swap(ConcurrentQueue& other) MOODYCAMEL_NOEXCEPT
		{
			swap_internal(other);
		}

	private:
		ConcurrentQueue& swap_internal(ConcurrentQueue& other)
		{
			if (this == &other) {
				return *this;
			}

			details::swap_relaxed(producerListTail, other.producerListTail);
			details::swap_relaxed(producerCount, other.producerCount);
			details::swap_relaxed(initialBlockPoolIndex, other.initialBlockPoolIndex);
			std::swap(initialBlockPool, other.initialBlockPool);
			std::swap(initialBlockPoolSize, other.initialBlockPoolSize);
			freeList.swap(other.freeList);
			details::swap_relaxed(nextExplicitConsumerId, other.nextExplicitConsumerId);
			details::swap_relaxed(globalExplicitConsumerOffset, other.globalExplicitConsumerOffset);

			swap_implicit_producer_hashes(other);

			reown_producers();
			other.reown_producers();

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
			details::swap_relaxed(explicitProducers, other.explicitProducers);
			details::swap_relaxed(implicitProducers, other.implicitProducers);
#endif

			return *this;
		}

	public:
		inline bool enqueue(T const& item)
		{
			if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
			return inner_enqueue<CanAlloc>(item);
		}

		inline bool enqueue(T&& item)
		{
			if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
			return inner_enqueue<CanAlloc>(std::move(item));
		}

		inline bool enqueue(producer_token_t const& token, T const& item)
		{
			return inner_enqueue<CanAlloc>(token, item);
		}

		inline bool enqueue(producer_token_t const& token, T&& item)
		{
			return inner_enqueue<CanAlloc>(token, std::move(item));
		}
		template<typename It>
		bool enqueue_bulk(It itemFirst, size_t count)
		{
			if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
			return inner_enqueue_bulk<CanAlloc>(itemFirst, count);
		}
		template<typename It>
		bool enqueue_bulk(producer_token_t const& token, It itemFirst, size_t count)
		{
			return inner_enqueue_bulk<CanAlloc>(token, itemFirst, count);
		}

		inline bool try_enqueue(T const& item)
		{
			if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
			return inner_enqueue<CannotAlloc>(item);
		}

		inline bool try_enqueue(T&& item)
		{
			if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
			return inner_enqueue<CannotAlloc>(std::move(item));
		}
		inline bool try_enqueue(producer_token_t const& token, T const& item)
		{
			return inner_enqueue<CannotAlloc>(token, item);
		}
		inline bool try_enqueue(producer_token_t const& token, T&& item)
		{
			return inner_enqueue<CannotAlloc>(token, std::move(item));
		}

		template<typename It>
		bool try_enqueue_bulk(It itemFirst, size_t count)
		{
			if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
			return inner_enqueue_bulk<CannotAlloc>(itemFirst, count);
		}

		template<typename It>
		bool try_enqueue_bulk(producer_token_t const& token, It itemFirst, size_t count)
		{
			return inner_enqueue_bulk<CannotAlloc>(token, itemFirst, count);
		}

		template<typename U>
		bool try_dequeue(U& item)
		{
			size_t nonEmptyCount = 0;
			ProducerBase* best = nullptr;
			size_t bestSize = 0;
			for (auto ptr = producerListTail.load(std::memory_order_acquire); nonEmptyCount < 3 && ptr != nullptr; ptr = ptr->next_prod()) {
				auto size = ptr->size_approx();
				if (size > 0) {
					if (size > bestSize) {
						bestSize = size;
						best = ptr;
					}
					++nonEmptyCount;
				}
			}

			if (nonEmptyCount > 0) {
				if ((details::likely)(best->dequeue(item))) {
					return true;
				}
				for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
					if (ptr != best && ptr->dequeue(item)) {
						return true;
					}
				}
			}
			return false;
		}

		template<typename U>
		bool try_dequeue_non_interleaved(U& item)
		{
			for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
				if (ptr->dequeue(item)) {
					return true;
				}
			}
			return false;
		}

		template<typename U>
		bool try_dequeue(consumer_token_t& token, U& item)
		{		
			if (token.desiredProducer == nullptr || token.lastKnownGlobalOffset != globalExplicitConsumerOffset.load(std::memory_order_relaxed)) {
				if (!update_current_producer_after_rotation(token)) {
					return false;
				}
			}

			if (static_cast<ProducerBase*>(token.currentProducer)->dequeue(item)) {
				if (++token.itemsConsumedFromCurrent == EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE) {
					globalExplicitConsumerOffset.fetch_add(1, std::memory_order_relaxed);
				}
				return true;
			}

			auto tail = producerListTail.load(std::memory_order_acquire);
			auto ptr = static_cast<ProducerBase*>(token.currentProducer)->next_prod();
			if (ptr == nullptr) {
				ptr = tail;
			}
			while (ptr != static_cast<ProducerBase*>(token.currentProducer)) {
				if (ptr->dequeue(item)) {
					token.currentProducer = ptr;
					token.itemsConsumedFromCurrent = 1;
					return true;
				}
				ptr = ptr->next_prod();
				if (ptr == nullptr) {
					ptr = tail;
				}
			}
			return false;
		}

		template<typename It>
		size_t try_dequeue_bulk(It itemFirst, size_t max)
		{
			size_t count = 0;
			for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
				count += ptr->dequeue_bulk(itemFirst, max - count);
				if (count == max) {
					break;
				}
			}
			return count;
		}

		template<typename It>
		size_t try_dequeue_bulk(consumer_token_t& token, It itemFirst, size_t max)
		{
			if (token.desiredProducer == nullptr || token.lastKnownGlobalOffset != globalExplicitConsumerOffset.load(std::memory_order_relaxed)) {
				if (!update_current_producer_after_rotation(token)) {
					return 0;
				}
			}

			size_t count = static_cast<ProducerBase*>(token.currentProducer)->dequeue_bulk(itemFirst, max);
			if (count == max) {
				if ((token.itemsConsumedFromCurrent += static_cast<std::uint32_t>(max)) >= EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE) {
					globalExplicitConsumerOffset.fetch_add(1, std::memory_order_relaxed);
				}
				return max;
			}
			token.itemsConsumedFromCurrent += static_cast<std::uint32_t>(count);
			max -= count;

			auto tail = producerListTail.load(std::memory_order_acquire);
			auto ptr = static_cast<ProducerBase*>(token.currentProducer)->next_prod();
			if (ptr == nullptr) {
				ptr = tail;
			}
			while (ptr != static_cast<ProducerBase*>(token.currentProducer)) {
				auto dequeued = ptr->dequeue_bulk(itemFirst, max);
				count += dequeued;
				if (dequeued != 0) {
					token.currentProducer = ptr;
					token.itemsConsumedFromCurrent = static_cast<std::uint32_t>(dequeued);
				}
				if (dequeued == max) {
					break;
				}
				max -= dequeued;
				ptr = ptr->next_prod();
				if (ptr == nullptr) {
					ptr = tail;
				}
			}
			return count;
		}

		template<typename U>
		inline bool try_dequeue_from_producer(producer_token_t const& producer, U& item)
		{
			return static_cast<ExplicitProducer*>(producer.producer)->dequeue(item);
		}

		template<typename It>
		inline size_t try_dequeue_bulk_from_producer(producer_token_t const& producer, It itemFirst, size_t max)
		{
			return static_cast<ExplicitProducer*>(producer.producer)->dequeue_bulk(itemFirst, max);
		}

		size_t size_approx() const
		{
			size_t size = 0;
			for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
				size += ptr->size_approx();
			}
			return size;
		}

		static bool is_lock_free()
		{
			return
				details::static_is_lock_free<bool>::value == 2 &&
				details::static_is_lock_free<size_t>::value == 2 &&
				details::static_is_lock_free<std::uint32_t>::value == 2 &&
				details::static_is_lock_free<index_t>::value == 2 &&
				details::static_is_lock_free<void*>::value == 2 &&
				details::static_is_lock_free<typename details::thread_id_converter<details::thread_id_t>::thread_id_numeric_size_t>::value == 2;
		}


	private:
		friend struct ProducerToken;
		friend struct ConsumerToken;
		struct ExplicitProducer;
		friend struct ExplicitProducer;
		struct ImplicitProducer;
		friend struct ImplicitProducer;
		friend class ConcurrentQueueTests;

		enum AllocationMode { CanAlloc, CannotAlloc };

		template<AllocationMode canAlloc, typename U>
		inline bool inner_enqueue(producer_token_t const& token, U&& element)
		{
			return static_cast<ExplicitProducer*>(token.producer)->ConcurrentQueue::ExplicitProducer::template enqueue<canAlloc>(std::forward<U>(element));
		}

		template<AllocationMode canAlloc, typename U>
		inline bool inner_enqueue(U&& element)
		{
			auto producer = get_or_add_implicit_producer();
			return producer == nullptr ? false : producer->ConcurrentQueue::ImplicitProducer::template enqueue<canAlloc>(std::forward<U>(element));
		}

		template<AllocationMode canAlloc, typename It>
		inline bool inner_enqueue_bulk(producer_token_t const& token, It itemFirst, size_t count)
		{
			return static_cast<ExplicitProducer*>(token.producer)->ConcurrentQueue::ExplicitProducer::template enqueue_bulk<canAlloc>(itemFirst, count);
		}

		template<AllocationMode canAlloc, typename It>
		inline bool inner_enqueue_bulk(It itemFirst, size_t count)
		{
			auto producer = get_or_add_implicit_producer();
			return producer == nullptr ? false : producer->ConcurrentQueue::ImplicitProducer::template enqueue_bulk<canAlloc>(itemFirst, count);
		}

		inline bool update_current_producer_after_rotation(consumer_token_t& token)
		{
			auto tail = producerListTail.load(std::memory_order_acquire);
			if (token.desiredProducer == nullptr && tail == nullptr) {
				return false;
			}
			auto prodCount = producerCount.load(std::memory_order_relaxed);
			auto globalOffset = globalExplicitConsumerOffset.load(std::memory_order_relaxed);
			if ((details::unlikely)(token.desiredProducer == nullptr)) {
				std::uint32_t offset = prodCount - 1 - (token.initialOffset % prodCount);
				token.desiredProducer = tail;
				for (std::uint32_t i = 0; i != offset; ++i) {
					token.desiredProducer = static_cast<ProducerBase*>(token.desiredProducer)->next_prod();
					if (token.desiredProducer == nullptr) {
						token.desiredProducer = tail;
					}
				}
			}

			std::uint32_t delta = globalOffset - token.lastKnownGlobalOffset;
			if (delta >= prodCount) {
				delta = delta % prodCount;
			}
			for (std::uint32_t i = 0; i != delta; ++i) {
				token.desiredProducer = static_cast<ProducerBase*>(token.desiredProducer)->next_prod();
				if (token.desiredProducer == nullptr) {
					token.desiredProducer = tail;
				}
			}

			token.lastKnownGlobalOffset = globalOffset;
			token.currentProducer = token.desiredProducer;
			token.itemsConsumedFromCurrent = 0;
			return true;
		}

		template <typename N>
		struct FreeListNode
		{
			FreeListNode() : freeListRefs(0), freeListNext(nullptr) { }

			std::atomic<std::uint32_t> freeListRefs;
			std::atomic<N*> freeListNext;
		};

		template<typename N>
		struct FreeList
		{
			FreeList() : freeListHead(nullptr) { }
			FreeList(FreeList&& other) : freeListHead(other.freeListHead.load(std::memory_order_relaxed)) { other.freeListHead.store(nullptr, std::memory_order_relaxed); }
			void swap(FreeList& other) { details::swap_relaxed(freeListHead, other.freeListHead); }

			FreeList(FreeList const&) MOODYCAMEL_DELETE_FUNCTION;
			FreeList& operator=(FreeList const&) MOODYCAMEL_DELETE_FUNCTION;

			inline void add(N* node)
			{
#ifdef MCDBGQ_NOLOCKFREE_FREELIST
				debug::DebugLock lock(mutex);
#endif		

				if (node->freeListRefs.fetch_add(SHOULD_BE_ON_FREELIST, std::memory_order_acq_rel) == 0) {
					add_knowing_refcount_is_zero(node);
				}
			}

			inline N* try_get()
			{
#ifdef MCDBGQ_NOLOCKFREE_FREELIST
				debug::DebugLock lock(mutex);
#endif		
				auto head = freeListHead.load(std::memory_order_acquire);
				while (head != nullptr) {
					auto prevHead = head;
					auto refs = head->freeListRefs.load(std::memory_order_relaxed);
					if ((refs & REFS_MASK) == 0 || !head->freeListRefs.compare_exchange_strong(refs, refs + 1, std::memory_order_acquire, std::memory_order_relaxed)) {
						head = freeListHead.load(std::memory_order_acquire);
						continue;
					}

					auto next = head->freeListNext.load(std::memory_order_relaxed);
					if (freeListHead.compare_exchange_strong(head, next, std::memory_order_acquire, std::memory_order_relaxed)) {
						assert((head->freeListRefs.load(std::memory_order_relaxed) & SHOULD_BE_ON_FREELIST) == 0);

						head->freeListRefs.fetch_sub(2, std::memory_order_release);
						return head;
					}
					refs = prevHead->freeListRefs.fetch_sub(1, std::memory_order_acq_rel);
					if (refs == SHOULD_BE_ON_FREELIST + 1) {
						add_knowing_refcount_is_zero(prevHead);
					}
				}

				return nullptr;
			}

			N* head_unsafe() const { return freeListHead.load(std::memory_order_relaxed); }

		private:
			inline void add_knowing_refcount_is_zero(N* node)
			{
				auto head = freeListHead.load(std::memory_order_relaxed);
				while (true) {
					node->freeListNext.store(head, std::memory_order_relaxed);
					node->freeListRefs.store(1, std::memory_order_release);
					if (!freeListHead.compare_exchange_strong(head, node, std::memory_order_release, std::memory_order_relaxed)) {
						if (node->freeListRefs.fetch_add(SHOULD_BE_ON_FREELIST - 1, std::memory_order_release) == 1) {
							continue;
						}
					}
					return;
				}
			}

		private:
			std::atomic<N*> freeListHead;

			static const std::uint32_t REFS_MASK = 0x7FFFFFFF;
			static const std::uint32_t SHOULD_BE_ON_FREELIST = 0x80000000;

#ifdef MCDBGQ_NOLOCKFREE_FREELIST
			debug::DebugMutex mutex;
#endif
		};
		enum InnerQueueContext { implicit_context = 0, explicit_context = 1 };

		struct Block
		{
			Block()
				: next(nullptr), elementsCompletelyDequeued(0), freeListRefs(0), freeListNext(nullptr), shouldBeOnFreeList(false), dynamicallyAllocated(true)
			{
#ifdef MCDBGQ_TRACKMEM
				owner = nullptr;
#endif
			}

			template<InnerQueueContext context>
			inline bool is_empty() const
			{
				if (context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
					for (size_t i = 0; i < BLOCK_SIZE; ++i) {
						if (!emptyFlags[i].load(std::memory_order_relaxed)) {
							return false;
						}
					}
					std::atomic_thread_fence(std::memory_order_acquire);
					return true;
				}
				else {
					if (elementsCompletelyDequeued.load(std::memory_order_relaxed) == BLOCK_SIZE) {
						std::atomic_thread_fence(std::memory_order_acquire);
						return true;
					}
					assert(elementsCompletelyDequeued.load(std::memory_order_relaxed) <= BLOCK_SIZE);
					return false;
				}
			}
			template<InnerQueueContext context>
			inline bool set_empty(index_t i)
			{
				if (context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
					// Set flag
					assert(!emptyFlags[BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1))].load(std::memory_order_relaxed));
					emptyFlags[BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1))].store(true, std::memory_order_release);
					return false;
				}
				else {
					// Increment counter
					auto prevVal = elementsCompletelyDequeued.fetch_add(1, std::memory_order_release);
					assert(prevVal < BLOCK_SIZE);
					return prevVal == BLOCK_SIZE - 1;
				}
			}

			template<InnerQueueContext context>
			inline bool set_many_empty(index_t i, size_t count)
			{
				if (context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
					// Set flags
					std::atomic_thread_fence(std::memory_order_release);
					i = BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1)) - count + 1;
					for (size_t j = 0; j != count; ++j) {
						assert(!emptyFlags[i + j].load(std::memory_order_relaxed));
						emptyFlags[i + j].store(true, std::memory_order_relaxed);
					}
					return false;
				}
				else {
					// Increment counter
					auto prevVal = elementsCompletelyDequeued.fetch_add(count, std::memory_order_release);
					assert(prevVal + count <= BLOCK_SIZE);
					return prevVal + count == BLOCK_SIZE;
				}
			}

			template<InnerQueueContext context>
			inline void set_all_empty()
			{
				if (context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
					// Set all flags
					for (size_t i = 0; i != BLOCK_SIZE; ++i) {
						emptyFlags[i].store(true, std::memory_order_relaxed);
					}
				}
				else {
					elementsCompletelyDequeued.store(BLOCK_SIZE, std::memory_order_relaxed);
				}
			}

			template<InnerQueueContext context>
			inline void reset_empty()
			{
				if (context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
					for (size_t i = 0; i != BLOCK_SIZE; ++i) {
						emptyFlags[i].store(false, std::memory_order_relaxed);
					}
				}
				else {
					elementsCompletelyDequeued.store(0, std::memory_order_relaxed);
				}
			}

			inline T* operator[](index_t idx) MOODYCAMEL_NOEXCEPT { return static_cast<T*>(static_cast<void*>(elements)) + static_cast<size_t>(idx & static_cast<index_t>(BLOCK_SIZE - 1)); }
			inline T const* operator[](index_t idx) const MOODYCAMEL_NOEXCEPT { return static_cast<T const*>(static_cast<void const*>(elements)) + static_cast<size_t>(idx & static_cast<index_t>(BLOCK_SIZE - 1)); }

		private:
			static_assert(std::alignment_of<T>::value <= std::alignment_of<details::max_align_t>::value, "The queue does not support super-aligned types at this time");

			union {
				char elements[sizeof(T) * BLOCK_SIZE];
				details::max_align_t dummy;
			};
		public:
			Block* next;
			std::atomic<size_t> elementsCompletelyDequeued;
			std::atomic<bool> emptyFlags[BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD ? BLOCK_SIZE : 1];
		public:
			std::atomic<std::uint32_t> freeListRefs;
			std::atomic<Block*> freeListNext;
			std::atomic<bool> shouldBeOnFreeList;
			bool dynamicallyAllocated;		// Perhaps a better name for this would be 'isNotPartOfInitialBlockPool'

#ifdef MCDBGQ_TRACKMEM
			void* owner;
#endif
		};
		static_assert(std::alignment_of<Block>::value >= std::alignment_of<details::max_align_t>::value, "Internal error: Blocks must be at least as aligned as the type they are wrapping");


#ifdef MCDBGQ_TRACKMEM
	public:
		struct MemStats;
	private:
#endif

		struct ProducerBase : public details::ConcurrentQueueProducerTypelessBase
		{
			ProducerBase(ConcurrentQueue* parent_, bool isExplicit_) :
				tailIndex(0),
				headIndex(0),
				dequeueOptimisticCount(0),
				dequeueOvercommit(0),
				tailBlock(nullptr),
				isExplicit(isExplicit_),
				parent(parent_)
			{
			}

			virtual ~ProducerBase() { };

			template<typename U>
			inline bool dequeue(U& element)
			{
				if (isExplicit) {
					return static_cast<ExplicitProducer*>(this)->dequeue(element);
				}
				else {
					return static_cast<ImplicitProducer*>(this)->dequeue(element);
				}
			}

			template<typename It>
			inline size_t dequeue_bulk(It& itemFirst, size_t max)
			{
				if (isExplicit) {
					return static_cast<ExplicitProducer*>(this)->dequeue_bulk(itemFirst, max);
				}
				else {
					return static_cast<ImplicitProducer*>(this)->dequeue_bulk(itemFirst, max);
				}
			}

			inline ProducerBase* next_prod() const { return static_cast<ProducerBase*>(next); }

			inline size_t size_approx() const
			{
				auto tail = tailIndex.load(std::memory_order_relaxed);
				auto head = headIndex.load(std::memory_order_relaxed);
				return details::circular_less_than(head, tail) ? static_cast<size_t>(tail - head) : 0;
			}

			inline index_t getTail() const { return tailIndex.load(std::memory_order_relaxed); }
		protected:
			std::atomic<index_t> tailIndex;	
			std::atomic<index_t> headIndex;

			std::atomic<index_t> dequeueOptimisticCount;
			std::atomic<index_t> dequeueOvercommit;

			Block* tailBlock;

		public:
			bool isExplicit;
			ConcurrentQueue* parent;

		protected:
#ifdef MCDBGQ_TRACKMEM
			friend struct MemStats;
#endif
		};


		struct ExplicitProducer : public ProducerBase
		{
			explicit ExplicitProducer(ConcurrentQueue* parent) :
			ProducerBase(parent, true),
				blockIndex(nullptr),
				pr_blockIndexSlotsUsed(0),
				pr_blockIndexSize(EXPLICIT_INITIAL_INDEX_SIZE >> 1),
				pr_blockIndexFront(0),
				pr_blockIndexEntries(nullptr),
				pr_blockIndexRaw(nullptr)
			{
				size_t poolBasedIndexSize = details::ceil_to_pow_2(parent->initialBlockPoolSize) >> 1;
				if (poolBasedIndexSize > pr_blockIndexSize) {
					pr_blockIndexSize = poolBasedIndexSize;
				}

				new_block_index(0);
			}

			~ExplicitProducer()
			{

				if (this->tailBlock != nullptr) {		
					Block* halfDequeuedBlock = nullptr;
					if ((this->headIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1)) != 0) {

						size_t i = (pr_blockIndexFront - pr_blockIndexSlotsUsed) & (pr_blockIndexSize - 1);
						while (details::circular_less_than<index_t>(pr_blockIndexEntries[i].base + BLOCK_SIZE, this->headIndex.load(std::memory_order_relaxed))) {
							i = (i + 1) & (pr_blockIndexSize - 1);
						}
						assert(details::circular_less_than<index_t>(pr_blockIndexEntries[i].base, this->headIndex.load(std::memory_order_relaxed)));
						halfDequeuedBlock = pr_blockIndexEntries[i].block;
					}

					auto block = this->tailBlock;
					do {
						block = block->next;
						if (block->ConcurrentQueue::Block::template is_empty<explicit_context>()) {
							continue;
						}

						size_t i = 0;	// Offset into block
						if (block == halfDequeuedBlock) {
							i = static_cast<size_t>(this->headIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1));
						}


						auto lastValidIndex = (this->tailIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1)) == 0 ? BLOCK_SIZE : static_cast<size_t>(this->tailIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1));
						while (i != BLOCK_SIZE && (block != this->tailBlock || i != lastValidIndex)) {
							(*block)[i++]->~T();
						}
					} while (block != this->tailBlock);
				}

				// Destroy all blocks that we own
				if (this->tailBlock != nullptr) {
					auto block = this->tailBlock;
					do {
						auto nextBlock = block->next;
						if (block->dynamicallyAllocated) {
							destroy(block);
						}
						else {
							this->parent->add_block_to_free_list(block);
						}
						block = nextBlock;
					} while (block != this->tailBlock);
				}

				auto header = static_cast<BlockIndexHeader*>(pr_blockIndexRaw);
				while (header != nullptr) {
					auto prev = static_cast<BlockIndexHeader*>(header->prev);
					header->~BlockIndexHeader();
					(Traits::free)(header);
					header = prev;
				}
			}

			template<AllocationMode allocMode, typename U>
			inline bool enqueue(U&& element)
			{
				index_t currentTailIndex = this->tailIndex.load(std::memory_order_relaxed);
				index_t newTailIndex = 1 + currentTailIndex;
				if ((currentTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0) {

					auto startBlock = this->tailBlock;
					auto originalBlockIndexSlotsUsed = pr_blockIndexSlotsUsed;
					if (this->tailBlock != nullptr && this->tailBlock->next->ConcurrentQueue::Block::template is_empty<explicit_context>()) {

						this->tailBlock = this->tailBlock->next;
						this->tailBlock->ConcurrentQueue::Block::template reset_empty<explicit_context>();

					}
					else {

						auto head = this->headIndex.load(std::memory_order_relaxed);
						assert(!details::circular_less_than<index_t>(currentTailIndex, head));
						if (!details::circular_less_than<index_t>(head, currentTailIndex + BLOCK_SIZE)
							|| (MAX_SUBQUEUE_SIZE != details::const_numeric_max<size_t>::value && (MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head))) {

								return false;
						}
						// We're going to need a new block; check that the block index has room
						if (pr_blockIndexRaw == nullptr || pr_blockIndexSlotsUsed == pr_blockIndexSize) {

							if (allocMode == CannotAlloc || !new_block_index(pr_blockIndexSlotsUsed)) {
								return false;
							}
						}

						// Insert a new block in the circular linked list
						auto newBlock = this->parent->ConcurrentQueue::template requisition_block<allocMode>();
						if (newBlock == nullptr) {
							return false;
						}
#ifdef MCDBGQ_TRACKMEM
						newBlock->owner = this;
#endif
						newBlock->ConcurrentQueue::Block::template reset_empty<explicit_context>();
						if (this->tailBlock == nullptr) {
							newBlock->next = newBlock;
						}
						else {
							newBlock->next = this->tailBlock->next;
							this->tailBlock->next = newBlock;
						}
						this->tailBlock = newBlock;
						++pr_blockIndexSlotsUsed;
					}

					if (!MOODYCAMEL_NOEXCEPT_CTOR(T, U, new ((T*)nullptr) T(std::forward<U>(element)))) {				
						MOODYCAMEL_TRY {
							new ((*this->tailBlock)[currentTailIndex]) T(std::forward<U>(element));
						}
						MOODYCAMEL_CATCH (...) {						
							pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
							this->tailBlock = startBlock == nullptr ? this->tailBlock : startBlock;
							MOODYCAMEL_RETHROW;
						}
					}
					else {
						(void)startBlock;
						(void)originalBlockIndexSlotsUsed;
					}

					auto& entry = blockIndex.load(std::memory_order_relaxed)->entries[pr_blockIndexFront];
					entry.base = currentTailIndex;
					entry.block = this->tailBlock;
					blockIndex.load(std::memory_order_relaxed)->front.store(pr_blockIndexFront, std::memory_order_release);
					pr_blockIndexFront = (pr_blockIndexFront + 1) & (pr_blockIndexSize - 1);

					if (!MOODYCAMEL_NOEXCEPT_CTOR(T, U, new ((T*)nullptr) T(std::forward<U>(element)))) {
						this->tailIndex.store(newTailIndex, std::memory_order_release);
						return true;
					}
				}

				new ((*this->tailBlock)[currentTailIndex]) T(std::forward<U>(element));

				this->tailIndex.store(newTailIndex, std::memory_order_release);
				return true;
			}

			template<typename U>
			bool dequeue(U& element)
			{
				auto tail = this->tailIndex.load(std::memory_order_relaxed);
				auto overcommit = this->dequeueOvercommit.load(std::memory_order_relaxed);
				if (details::circular_less_than<index_t>(this->dequeueOptimisticCount.load(std::memory_order_relaxed) - overcommit, tail)) {			
					std::atomic_thread_fence(std::memory_order_acquire);

					auto myDequeueCount = this->dequeueOptimisticCount.fetch_add(1, std::memory_order_relaxed);

					tail = this->tailIndex.load(std::memory_order_acquire);
					if ((details::likely)(details::circular_less_than<index_t>(myDequeueCount - overcommit, tail))) {					
						auto index = this->headIndex.fetch_add(1, std::memory_order_acq_rel);
						auto localBlockIndex = blockIndex.load(std::memory_order_acquire);
						auto localBlockIndexHead = localBlockIndex->front.load(std::memory_order_acquire);

						auto headBase = localBlockIndex->entries[localBlockIndexHead].base;
						auto blockBaseIndex = index & ~static_cast<index_t>(BLOCK_SIZE - 1);
						auto offset = static_cast<size_t>(static_cast<typename std::make_signed<index_t>::type>(blockBaseIndex - headBase) / BLOCK_SIZE);
						auto block = localBlockIndex->entries[(localBlockIndexHead + offset) & (localBlockIndex->size - 1)].block;

						// Dequeue
						auto& el = *((*block)[index]);
						if (!MOODYCAMEL_NOEXCEPT_ASSIGN(T, T&&, element = std::move(el))) {

							struct Guard {
								Block* block;
								index_t index;

								~Guard()
								{
									(*block)[index]->~T();
									block->ConcurrentQueue::Block::template set_empty<explicit_context>(index);
								}
							} guard = { block, index };

							element = std::move(el); // NOLINT
						}
						else {
							element = std::move(el); // NOLINT
							el.~T(); // NOLINT
							block->ConcurrentQueue::Block::template set_empty<explicit_context>(index);
						}

						return true;
					}
					else {

						this->dequeueOvercommit.fetch_add(1, std::memory_order_release);
					}
				}

				return false;
			}

			template<AllocationMode allocMode, typename It>
			bool enqueue_bulk(It itemFirst, size_t count)
			{

				index_t startTailIndex = this->tailIndex.load(std::memory_order_relaxed);
				auto startBlock = this->tailBlock;
				auto originalBlockIndexFront = pr_blockIndexFront;
				auto originalBlockIndexSlotsUsed = pr_blockIndexSlotsUsed;

				Block* firstAllocatedBlock = nullptr;

				size_t blockBaseDiff = ((startTailIndex + count - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1)) - ((startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1));
				index_t currentTailIndex = (startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1);
				if (blockBaseDiff > 0) {
					// Allocate as many blocks as possible from ahead
					while (blockBaseDiff > 0 && this->tailBlock != nullptr && this->tailBlock->next != firstAllocatedBlock && this->tailBlock->next->ConcurrentQueue::Block::template is_empty<explicit_context>()) {
						blockBaseDiff -= static_cast<index_t>(BLOCK_SIZE);
						currentTailIndex += static_cast<index_t>(BLOCK_SIZE);

						this->tailBlock = this->tailBlock->next;
						firstAllocatedBlock = firstAllocatedBlock == nullptr ? this->tailBlock : firstAllocatedBlock;

						auto& entry = blockIndex.load(std::memory_order_relaxed)->entries[pr_blockIndexFront];
						entry.base = currentTailIndex;
						entry.block = this->tailBlock;
						pr_blockIndexFront = (pr_blockIndexFront + 1) & (pr_blockIndexSize - 1);
					}

					// Now allocate as many blocks as necessary from the block pool
					while (blockBaseDiff > 0) {
						blockBaseDiff -= static_cast<index_t>(BLOCK_SIZE);
						currentTailIndex += static_cast<index_t>(BLOCK_SIZE);

						auto head = this->headIndex.load(std::memory_order_relaxed);
						assert(!details::circular_less_than<index_t>(currentTailIndex, head));
						bool full = !details::circular_less_than<index_t>(head, currentTailIndex + BLOCK_SIZE) || (MAX_SUBQUEUE_SIZE != details::const_numeric_max<size_t>::value && (MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head));
						if (pr_blockIndexRaw == nullptr || pr_blockIndexSlotsUsed == pr_blockIndexSize || full) {
							if (allocMode == CannotAlloc || full || !new_block_index(originalBlockIndexSlotsUsed)) {
								// Failed to allocate, undo changes (but keep injected blocks)
								pr_blockIndexFront = originalBlockIndexFront;
								pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
								this->tailBlock = startBlock == nullptr ? firstAllocatedBlock : startBlock;
								return false;
							}


							originalBlockIndexFront = originalBlockIndexSlotsUsed;
						}

						// Insert a new block in the circular linked list
						auto newBlock = this->parent->ConcurrentQueue::template requisition_block<allocMode>();
						if (newBlock == nullptr) {
							pr_blockIndexFront = originalBlockIndexFront;
							pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
							this->tailBlock = startBlock == nullptr ? firstAllocatedBlock : startBlock;
							return false;
						}

#ifdef MCDBGQ_TRACKMEM
						newBlock->owner = this;
#endif
						newBlock->ConcurrentQueue::Block::template set_all_empty<explicit_context>();
						if (this->tailBlock == nullptr) {
							newBlock->next = newBlock;
						}
						else {
							newBlock->next = this->tailBlock->next;
							this->tailBlock->next = newBlock;
						}
						this->tailBlock = newBlock;
						firstAllocatedBlock = firstAllocatedBlock == nullptr ? this->tailBlock : firstAllocatedBlock;

						++pr_blockIndexSlotsUsed;

						auto& entry = blockIndex.load(std::memory_order_relaxed)->entries[pr_blockIndexFront];
						entry.base = currentTailIndex;
						entry.block = this->tailBlock;
						pr_blockIndexFront = (pr_blockIndexFront + 1) & (pr_blockIndexSize - 1);
					}

					// Excellent, all allocations succeeded. Reset each block's emptiness before we fill them up, and
					// publish the new block index front
					auto block = firstAllocatedBlock;
					while (true) {
						block->ConcurrentQueue::Block::template reset_empty<explicit_context>();
						if (block == this->tailBlock) {
							break;
						}
						block = block->next;
					}

					if (MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new ((T*)nullptr) T(details::deref_noexcept(itemFirst)))) {
						blockIndex.load(std::memory_order_relaxed)->front.store((pr_blockIndexFront - 1) & (pr_blockIndexSize - 1), std::memory_order_release);
					}
				}

				// Enqueue, one block at a time
				index_t newTailIndex = startTailIndex + static_cast<index_t>(count);
				currentTailIndex = startTailIndex;
				auto endBlock = this->tailBlock;
				this->tailBlock = startBlock;
				assert((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) != 0 || firstAllocatedBlock != nullptr || count == 0);
				if ((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0 && firstAllocatedBlock != nullptr) {
					this->tailBlock = firstAllocatedBlock;
				}
				while (true) {
					auto stopIndex = (currentTailIndex & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
					if (details::circular_less_than<index_t>(newTailIndex, stopIndex)) {
						stopIndex = newTailIndex;
					}
					if (MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new ((T*)nullptr) T(details::deref_noexcept(itemFirst)))) {
						while (currentTailIndex != stopIndex) {
							new ((*this->tailBlock)[currentTailIndex++]) T(*itemFirst++);
						}
					}
					else {
						MOODYCAMEL_TRY {
							while (currentTailIndex != stopIndex) {

								new ((*this->tailBlock)[currentTailIndex]) T(details::nomove_if<(bool)!MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new ((T*)nullptr) T(details::deref_noexcept(itemFirst)))>::eval(*itemFirst));
								++currentTailIndex;
								++itemFirst;
							}
						}
						MOODYCAMEL_CATCH (...) {

							auto constructedStopIndex = currentTailIndex;
							auto lastBlockEnqueued = this->tailBlock;

							pr_blockIndexFront = originalBlockIndexFront;
							pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
							this->tailBlock = startBlock == nullptr ? firstAllocatedBlock : startBlock;

							if (!details::is_trivially_destructible<T>::value) {
								auto block = startBlock;
								if ((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0) {
									block = firstAllocatedBlock;
								}
								currentTailIndex = startTailIndex;
								while (true) {
									stopIndex = (currentTailIndex & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
									if (details::circular_less_than<index_t>(constructedStopIndex, stopIndex)) {
										stopIndex = constructedStopIndex;
									}
									while (currentTailIndex != stopIndex) {
										(*block)[currentTailIndex++]->~T();
									}
									if (block == lastBlockEnqueued) {
										break;
									}
									block = block->next;
								}
							}
							MOODYCAMEL_RETHROW;
						}
					}

					if (this->tailBlock == endBlock) {
						assert(currentTailIndex == newTailIndex);
						break;
					}
					this->tailBlock = this->tailBlock->next;
				}

				if (!MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new ((T*)nullptr) T(details::deref_noexcept(itemFirst))) && firstAllocatedBlock != nullptr) {
					blockIndex.load(std::memory_order_relaxed)->front.store((pr_blockIndexFront - 1) & (pr_blockIndexSize - 1), std::memory_order_release);
				}

				this->tailIndex.store(newTailIndex, std::memory_order_release);
				return true;
			}

			template<typename It>
			size_t dequeue_bulk(It& itemFirst, size_t max)
			{
				auto tail = this->tailIndex.load(std::memory_order_relaxed);
				auto overcommit = this->dequeueOvercommit.load(std::memory_order_relaxed);
				auto desiredCount = static_cast<size_t>(tail - (this->dequeueOptimisticCount.load(std::memory_order_relaxed) - overcommit));
				if (details::circular_less_than<size_t>(0, desiredCount)) {
					desiredCount = desiredCount < max ? desiredCount : max;
					std::atomic_thread_fence(std::memory_order_acquire);

					auto myDequeueCount = this->dequeueOptimisticCount.fetch_add(desiredCount, std::memory_order_relaxed);;

					tail = this->tailIndex.load(std::memory_order_acquire);
					auto actualCount = static_cast<size_t>(tail - (myDequeueCount - overcommit));
					if (details::circular_less_than<size_t>(0, actualCount)) {
						actualCount = desiredCount < actualCount ? desiredCount : actualCount;
						if (actualCount < desiredCount) {
							this->dequeueOvercommit.fetch_add(desiredCount - actualCount, std::memory_order_release);
						}


						auto firstIndex = this->headIndex.fetch_add(actualCount, std::memory_order_acq_rel);


						auto localBlockIndex = blockIndex.load(std::memory_order_acquire);
						auto localBlockIndexHead = localBlockIndex->front.load(std::memory_order_acquire);

						auto headBase = localBlockIndex->entries[localBlockIndexHead].base;
						auto firstBlockBaseIndex = firstIndex & ~static_cast<index_t>(BLOCK_SIZE - 1);
						auto offset = static_cast<size_t>(static_cast<typename std::make_signed<index_t>::type>(firstBlockBaseIndex - headBase) / BLOCK_SIZE);
						auto indexIndex = (localBlockIndexHead + offset) & (localBlockIndex->size - 1);

						// Iterate the blocks and dequeue
						auto index = firstIndex;
						do {
							auto firstIndexInBlock = index;
							auto endIndex = (index & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
							endIndex = details::circular_less_than<index_t>(firstIndex + static_cast<index_t>(actualCount), endIndex) ? firstIndex + static_cast<index_t>(actualCount) : endIndex;
							auto block = localBlockIndex->entries[indexIndex].block;
							if (MOODYCAMEL_NOEXCEPT_ASSIGN(T, T&&, details::deref_noexcept(itemFirst) = std::move((*(*block)[index])))) {
								while (index != endIndex) {
									auto& el = *((*block)[index]);
									*itemFirst++ = std::move(el);
									el.~T();
									++index;
								}
							}
							else {
								MOODYCAMEL_TRY {
									while (index != endIndex) {
										auto& el = *((*block)[index]);
										*itemFirst = std::move(el);
										++itemFirst;
										el.~T();
										++index;
									}
								}
								MOODYCAMEL_CATCH (...) {

									do {
										block = localBlockIndex->entries[indexIndex].block;
										while (index != endIndex) {
											(*block)[index++]->~T();
										}
										block->ConcurrentQueue::Block::template set_many_empty<explicit_context>(firstIndexInBlock, static_cast<size_t>(endIndex - firstIndexInBlock));
										indexIndex = (indexIndex + 1) & (localBlockIndex->size - 1);

										firstIndexInBlock = index;
										endIndex = (index & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
										endIndex = details::circular_less_than<index_t>(firstIndex + static_cast<index_t>(actualCount), endIndex) ? firstIndex + static_cast<index_t>(actualCount) : endIndex;
									} while (index != firstIndex + actualCount);

									MOODYCAMEL_RETHROW;
								}
							}
							block->ConcurrentQueue::Block::template set_many_empty<explicit_context>(firstIndexInBlock, static_cast<size_t>(endIndex - firstIndexInBlock));
							indexIndex = (indexIndex + 1) & (localBlockIndex->size - 1);
						} while (index != firstIndex + actualCount);

						return actualCount;
					}
					else {
						this->dequeueOvercommit.fetch_add(desiredCount, std::memory_order_release);
					}
				}

				return 0;
			}

		private:
			struct BlockIndexEntry
			{
				index_t base;
				Block* block;
			};

			struct BlockIndexHeader
			{
				size_t size;
				std::atomic<size_t> front;
				BlockIndexEntry* entries;
				void* prev;
			};


			bool new_block_index(size_t numberOfFilledSlotsToExpose)
			{
				auto prevBlockSizeMask = pr_blockIndexSize - 1;

				// Create the new block
				pr_blockIndexSize <<= 1;
				auto newRawPtr = static_cast<char*>((Traits::malloc)(sizeof(BlockIndexHeader) + std::alignment_of<BlockIndexEntry>::value - 1 + sizeof(BlockIndexEntry) * pr_blockIndexSize));
				if (newRawPtr == nullptr) {
					pr_blockIndexSize >>= 1;		// Reset to allow graceful retry
					return false;
				}

				auto newBlockIndexEntries = reinterpret_cast<BlockIndexEntry*>(details::align_for<BlockIndexEntry>(newRawPtr + sizeof(BlockIndexHeader)));

				// Copy in all the old indices, if any
				size_t j = 0;
				if (pr_blockIndexSlotsUsed != 0) {
					auto i = (pr_blockIndexFront - pr_blockIndexSlotsUsed) & prevBlockSizeMask;
					do {
						newBlockIndexEntries[j++] = pr_blockIndexEntries[i];
						i = (i + 1) & prevBlockSizeMask;
					} while (i != pr_blockIndexFront);
				}

				// Update everything
				auto header = new (newRawPtr) BlockIndexHeader;
				header->size = pr_blockIndexSize;
				header->front.store(numberOfFilledSlotsToExpose - 1, std::memory_order_relaxed);
				header->entries = newBlockIndexEntries;
				header->prev = pr_blockIndexRaw;		// we link the new block to the old one so we can free it later

				pr_blockIndexFront = j;
				pr_blockIndexEntries = newBlockIndexEntries;
				pr_blockIndexRaw = newRawPtr;
				blockIndex.store(header, std::memory_order_release);

				return true;
			}

		private:
			std::atomic<BlockIndexHeader*> blockIndex;

			// To be used by producer only -- consumer must use the ones in referenced by blockIndex
			size_t pr_blockIndexSlotsUsed;
			size_t pr_blockIndexSize;
			size_t pr_blockIndexFront;		// Next slot (not current)
			BlockIndexEntry* pr_blockIndexEntries;
			void* pr_blockIndexRaw;

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
		public:
			ExplicitProducer* nextExplicitProducer;
		private:
#endif

#ifdef MCDBGQ_TRACKMEM
			friend struct MemStats;
#endif
		};	
		struct ImplicitProducer : public ProducerBase
		{			
			ImplicitProducer(ConcurrentQueue* parent) :
				ProducerBase(parent, false),
				nextBlockIndexCapacity(IMPLICIT_INITIAL_INDEX_SIZE),
				blockIndex(nullptr)
			{
				new_block_index();
			}

			~ImplicitProducer()
			{
#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
				// Unregister ourselves for thread termination notification
				if (!this->inactive.load(std::memory_order_relaxed)) {
					details::ThreadExitNotifier::unsubscribe(&threadExitListener);
				}
#endif
				auto tail = this->tailIndex.load(std::memory_order_relaxed);
				auto index = this->headIndex.load(std::memory_order_relaxed);
				Block* block = nullptr;
				assert(index == tail || details::circular_less_than(index, tail));
				bool forceFreeLastBlock = index != tail;		// If we enter the loop, then the last (tail) block will not be freed
				while (index != tail) {
					if ((index & static_cast<index_t>(BLOCK_SIZE - 1)) == 0 || block == nullptr) {
						if (block != nullptr) {
							// Free the old block
							this->parent->add_block_to_free_list(block);
						}

						block = get_block_index_entry_for_index(index)->value.load(std::memory_order_relaxed);
					}

					((*block)[index])->~T();
					++index;
				}
				if (this->tailBlock != nullptr && (forceFreeLastBlock || (tail & static_cast<index_t>(BLOCK_SIZE - 1)) != 0)) {
					this->parent->add_block_to_free_list(this->tailBlock);
				}

				// Destroy block index
				auto localBlockIndex = blockIndex.load(std::memory_order_relaxed);
				if (localBlockIndex != nullptr) {
					for (size_t i = 0; i != localBlockIndex->capacity; ++i) {
						localBlockIndex->index[i]->~BlockIndexEntry();
					}
					do {
						auto prev = localBlockIndex->prev;
						localBlockIndex->~BlockIndexHeader();
						(Traits::free)(localBlockIndex);
						localBlockIndex = prev;
					} while (localBlockIndex != nullptr);
				}
			}

			template<AllocationMode allocMode, typename U>
			inline bool enqueue(U&& element)
			{
				index_t currentTailIndex = this->tailIndex.load(std::memory_order_relaxed);
				index_t newTailIndex = 1 + currentTailIndex;
				if ((currentTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0) {
					// We reached the end of a block, start a new one
					auto head = this->headIndex.load(std::memory_order_relaxed);
					assert(!details::circular_less_than<index_t>(currentTailIndex, head));
					if (!details::circular_less_than<index_t>(head, currentTailIndex + BLOCK_SIZE) || (MAX_SUBQUEUE_SIZE != details::const_numeric_max<size_t>::value && (MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head))) {
						return false;
					}
#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
					debug::DebugLock lock(mutex);
#endif
					BlockIndexEntry* idxEntry;
					if (!insert_block_index_entry<allocMode>(idxEntry, currentTailIndex)) {
						return false;
					}

					auto newBlock = this->parent->ConcurrentQueue::template requisition_block<allocMode>();
					if (newBlock == nullptr) {
						rewind_block_index_tail();
						idxEntry->value.store(nullptr, std::memory_order_relaxed);
						return false;
					}
#ifdef MCDBGQ_TRACKMEM
					newBlock->owner = this;
#endif
					newBlock->ConcurrentQueue::Block::template reset_empty<implicit_context>();

					if (!MOODYCAMEL_NOEXCEPT_CTOR(T, U, new ((T*)nullptr) T(std::forward<U>(element)))) {
						// May throw, try to insert now before we publish the fact that we have this new block
						MOODYCAMEL_TRY {
							new ((*newBlock)[currentTailIndex]) T(std::forward<U>(element));
						}
						MOODYCAMEL_CATCH (...) {
							rewind_block_index_tail();
							idxEntry->value.store(nullptr, std::memory_order_relaxed);
							this->parent->add_block_to_free_list(newBlock);
							MOODYCAMEL_RETHROW;
						}
					}

					// Insert the new block into the index
					idxEntry->value.store(newBlock, std::memory_order_relaxed);

					this->tailBlock = newBlock;

					if (!MOODYCAMEL_NOEXCEPT_CTOR(T, U, new ((T*)nullptr) T(std::forward<U>(element)))) {
						this->tailIndex.store(newTailIndex, std::memory_order_release);
						return true;
					}
				}

				// Enqueue
				new ((*this->tailBlock)[currentTailIndex]) T(std::forward<U>(element));

				this->tailIndex.store(newTailIndex, std::memory_order_release);
				return true;
			}

			template<typename U>
			bool dequeue(U& element)
			{
				index_t tail = this->tailIndex.load(std::memory_order_relaxed);
				index_t overcommit = this->dequeueOvercommit.load(std::memory_order_relaxed);
				if (details::circular_less_than<index_t>(this->dequeueOptimisticCount.load(std::memory_order_relaxed) - overcommit, tail)) {
					std::atomic_thread_fence(std::memory_order_acquire);

					index_t myDequeueCount = this->dequeueOptimisticCount.fetch_add(1, std::memory_order_relaxed);
					tail = this->tailIndex.load(std::memory_order_acquire);
					if ((details::likely)(details::circular_less_than<index_t>(myDequeueCount - overcommit, tail))) {
						index_t index = this->headIndex.fetch_add(1, std::memory_order_acq_rel);

						// Determine which block the element is in
						auto entry = get_block_index_entry_for_index(index);

						// Dequeue
						auto block = entry->value.load(std::memory_order_relaxed);
						auto& el = *((*block)[index]);

						if (!MOODYCAMEL_NOEXCEPT_ASSIGN(T, T&&, element = std::move(el))) {
#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
							debug::DebugLock lock(producer->mutex);
#endif
							struct Guard {
								Block* block;
								index_t index;
								BlockIndexEntry* entry;
								ConcurrentQueue* parent;

								~Guard()
								{
									(*block)[index]->~T();
									if (block->ConcurrentQueue::Block::template set_empty<implicit_context>(index)) {
										entry->value.store(nullptr, std::memory_order_relaxed);
										parent->add_block_to_free_list(block);
									}
								}
							} guard = { block, index, entry, this->parent };

							element = std::move(el); // NOLINT
						}
						else {
							element = std::move(el); // NOLINT
							el.~T(); // NOLINT

							if (block->ConcurrentQueue::Block::template set_empty<implicit_context>(index)) {
								{
#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
									debug::DebugLock lock(mutex);
#endif
									// Save the block back into the global free pool (and remove from block index)
									entry->value.store(nullptr, std::memory_order_relaxed);
								}
								this->parent->add_block_to_free_list(block);		// releases the above store
							}
						}

						return true;
					}
					else {
						this->dequeueOvercommit.fetch_add(1, std::memory_order_release);
					}
				}

				return false;
			}

			template<AllocationMode allocMode, typename It>
			bool enqueue_bulk(It itemFirst, size_t count)
			{

				index_t startTailIndex = this->tailIndex.load(std::memory_order_relaxed);
				auto startBlock = this->tailBlock;
				Block* firstAllocatedBlock = nullptr;
				auto endBlock = this->tailBlock;

				size_t blockBaseDiff = ((startTailIndex + count - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1)) - ((startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1));
				index_t currentTailIndex = (startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1);
				if (blockBaseDiff > 0) {
#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
					debug::DebugLock lock(mutex);
#endif
					do {
						blockBaseDiff -= static_cast<index_t>(BLOCK_SIZE);
						currentTailIndex += static_cast<index_t>(BLOCK_SIZE);
						BlockIndexEntry* idxEntry = nullptr;  // initialization here unnecessary but compiler can't always tell
						Block* newBlock;
						bool indexInserted = false;
						auto head = this->headIndex.load(std::memory_order_relaxed);
						assert(!details::circular_less_than<index_t>(currentTailIndex, head));
						bool full = !details::circular_less_than<index_t>(head, currentTailIndex + BLOCK_SIZE) || (MAX_SUBQUEUE_SIZE != details::const_numeric_max<size_t>::value && (MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head));
						if (full || !(indexInserted = insert_block_index_entry<allocMode>(idxEntry, currentTailIndex)) || (newBlock = this->parent->ConcurrentQueue::template requisition_block<allocMode>()) == nullptr) {					
							if (indexInserted) {
								rewind_block_index_tail();
								idxEntry->value.store(nullptr, std::memory_order_relaxed);
							}
							currentTailIndex = (startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1);
							for (auto block = firstAllocatedBlock; block != nullptr; block = block->next) {
								currentTailIndex += static_cast<index_t>(BLOCK_SIZE);
								idxEntry = get_block_index_entry_for_index(currentTailIndex);
								idxEntry->value.store(nullptr, std::memory_order_relaxed);
								rewind_block_index_tail();
							}
							this->parent->add_blocks_to_free_list(firstAllocatedBlock);
							this->tailBlock = startBlock;

							return false;
						}

#ifdef MCDBGQ_TRACKMEM
						newBlock->owner = this;
#endif
						newBlock->ConcurrentQueue::Block::template reset_empty<implicit_context>();
						newBlock->next = nullptr;			
						idxEntry->value.store(newBlock, std::memory_order_relaxed);

						if ((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) != 0 || firstAllocatedBlock != nullptr) {
							assert(this->tailBlock != nullptr);
							this->tailBlock->next = newBlock;
						}
						this->tailBlock = newBlock;
						endBlock = newBlock;
						firstAllocatedBlock = firstAllocatedBlock == nullptr ? newBlock : firstAllocatedBlock;
					} while (blockBaseDiff > 0);
				}

				index_t newTailIndex = startTailIndex + static_cast<index_t>(count);
				currentTailIndex = startTailIndex;
				this->tailBlock = startBlock;
				assert((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) != 0 || firstAllocatedBlock != nullptr || count == 0);
				if ((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0 && firstAllocatedBlock != nullptr) {
					this->tailBlock = firstAllocatedBlock;
				}
				while (true) {
					auto stopIndex = (currentTailIndex & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
					if (details::circular_less_than<index_t>(newTailIndex, stopIndex)) {
						stopIndex = newTailIndex;
					}
					if (MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new ((T*)nullptr) T(details::deref_noexcept(itemFirst)))) {
						while (currentTailIndex != stopIndex) {
							new ((*this->tailBlock)[currentTailIndex++]) T(*itemFirst++);
						}
					}
					else {
						MOODYCAMEL_TRY {
							while (currentTailIndex != stopIndex) {
								new ((*this->tailBlock)[currentTailIndex]) T(details::nomove_if<(bool)!MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new ((T*)nullptr) T(details::deref_noexcept(itemFirst)))>::eval(*itemFirst));
								++currentTailIndex;
								++itemFirst;
							}
						}
						MOODYCAMEL_CATCH (...) {
							auto constructedStopIndex = currentTailIndex;
							auto lastBlockEnqueued = this->tailBlock;

							if (!details::is_trivially_destructible<T>::value) {
								auto block = startBlock;
								if ((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0) {
									block = firstAllocatedBlock;
								}
								currentTailIndex = startTailIndex;
								while (true) {
									stopIndex = (currentTailIndex & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
									if (details::circular_less_than<index_t>(constructedStopIndex, stopIndex)) {
										stopIndex = constructedStopIndex;
									}
									while (currentTailIndex != stopIndex) {
										(*block)[currentTailIndex++]->~T();
									}
									if (block == lastBlockEnqueued) {
										break;
									}
									block = block->next;
								}
							}

							currentTailIndex = (startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1);
							for (auto block = firstAllocatedBlock; block != nullptr; block = block->next) {
								currentTailIndex += static_cast<index_t>(BLOCK_SIZE);
								auto idxEntry = get_block_index_entry_for_index(currentTailIndex);
								idxEntry->value.store(nullptr, std::memory_order_relaxed);
								rewind_block_index_tail();
							}
							this->parent->add_blocks_to_free_list(firstAllocatedBlock);
							this->tailBlock = startBlock;
							MOODYCAMEL_RETHROW;
						}
					}

					if (this->tailBlock == endBlock) {
						assert(currentTailIndex == newTailIndex);
						break;
					}
					this->tailBlock = this->tailBlock->next;
				}
				this->tailIndex.store(newTailIndex, std::memory_order_release);
				return true;
			}

			template<typename It>
			size_t dequeue_bulk(It& itemFirst, size_t max)
			{
				auto tail = this->tailIndex.load(std::memory_order_relaxed);
				auto overcommit = this->dequeueOvercommit.load(std::memory_order_relaxed);
				auto desiredCount = static_cast<size_t>(tail - (this->dequeueOptimisticCount.load(std::memory_order_relaxed) - overcommit));
				if (details::circular_less_than<size_t>(0, desiredCount)) {
					desiredCount = desiredCount < max ? desiredCount : max;
					std::atomic_thread_fence(std::memory_order_acquire);

					auto myDequeueCount = this->dequeueOptimisticCount.fetch_add(desiredCount, std::memory_order_relaxed);

					tail = this->tailIndex.load(std::memory_order_acquire);
					auto actualCount = static_cast<size_t>(tail - (myDequeueCount - overcommit));
					if (details::circular_less_than<size_t>(0, actualCount)) {
						actualCount = desiredCount < actualCount ? desiredCount : actualCount;
						if (actualCount < desiredCount) {
							this->dequeueOvercommit.fetch_add(desiredCount - actualCount, std::memory_order_release);
						}

						auto firstIndex = this->headIndex.fetch_add(actualCount, std::memory_order_acq_rel);
						auto index = firstIndex;
						BlockIndexHeader* localBlockIndex;
						auto indexIndex = get_block_index_index_for_index(index, localBlockIndex);
						do {
							auto blockStartIndex = index;
							auto endIndex = (index & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
							endIndex = details::circular_less_than<index_t>(firstIndex + static_cast<index_t>(actualCount), endIndex) ? firstIndex + static_cast<index_t>(actualCount) : endIndex;

							auto entry = localBlockIndex->index[indexIndex];
							auto block = entry->value.load(std::memory_order_relaxed);
							if (MOODYCAMEL_NOEXCEPT_ASSIGN(T, T&&, details::deref_noexcept(itemFirst) = std::move((*(*block)[index])))) {
								while (index != endIndex) {
									auto& el = *((*block)[index]);
									*itemFirst++ = std::move(el);
									el.~T();
									++index;
								}
							}
							else {
								MOODYCAMEL_TRY {
									while (index != endIndex) {
										auto& el = *((*block)[index]);
										*itemFirst = std::move(el);
										++itemFirst;
										el.~T();
										++index;
									}
								}
								MOODYCAMEL_CATCH (...) {
									do {
										entry = localBlockIndex->index[indexIndex];
										block = entry->value.load(std::memory_order_relaxed);
										while (index != endIndex) {
											(*block)[index++]->~T();
										}

										if (block->ConcurrentQueue::Block::template set_many_empty<implicit_context>(blockStartIndex, static_cast<size_t>(endIndex - blockStartIndex))) {
#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
											debug::DebugLock lock(mutex);
#endif
											entry->value.store(nullptr, std::memory_order_relaxed);
											this->parent->add_block_to_free_list(block);
										}
										indexIndex = (indexIndex + 1) & (localBlockIndex->capacity - 1);

										blockStartIndex = index;
										endIndex = (index & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
										endIndex = details::circular_less_than<index_t>(firstIndex + static_cast<index_t>(actualCount), endIndex) ? firstIndex + static_cast<index_t>(actualCount) : endIndex;
									} while (index != firstIndex + actualCount);

									MOODYCAMEL_RETHROW;
								}
							}
							if (block->ConcurrentQueue::Block::template set_many_empty<implicit_context>(blockStartIndex, static_cast<size_t>(endIndex - blockStartIndex))) {
								{
#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
									debug::DebugLock lock(mutex);
#endif
									entry->value.store(nullptr, std::memory_order_relaxed);
								}
								this->parent->add_block_to_free_list(block);		// releases the above store
							}
							indexIndex = (indexIndex + 1) & (localBlockIndex->capacity - 1);
						} while (index != firstIndex + actualCount);

						return actualCount;
					}
					else {
						this->dequeueOvercommit.fetch_add(desiredCount, std::memory_order_release);
					}
				}

				return 0;
			}

		private:
			static const index_t INVALID_BLOCK_BASE = 1;

			struct BlockIndexEntry
			{
				std::atomic<index_t> key;
				std::atomic<Block*> value;
			};

			struct BlockIndexHeader
			{
				size_t capacity;
				std::atomic<size_t> tail;
				BlockIndexEntry* entries;
				BlockIndexEntry** index;
				BlockIndexHeader* prev;
			};

			template<AllocationMode allocMode>
			inline bool insert_block_index_entry(BlockIndexEntry*& idxEntry, index_t blockStartIndex)
			{
				auto localBlockIndex = blockIndex.load(std::memory_order_relaxed);		// We're the only writer thread, relaxed is OK
				if (localBlockIndex == nullptr) {
					return false;  // this can happen if new_block_index failed in the constructor
				}
				auto newTail = (localBlockIndex->tail.load(std::memory_order_relaxed) + 1) & (localBlockIndex->capacity - 1);
				idxEntry = localBlockIndex->index[newTail];
				if (idxEntry->key.load(std::memory_order_relaxed) == INVALID_BLOCK_BASE ||
					idxEntry->value.load(std::memory_order_relaxed) == nullptr) {

						idxEntry->key.store(blockStartIndex, std::memory_order_relaxed);
						localBlockIndex->tail.store(newTail, std::memory_order_release);
						return true;
				}

				// No room in the old block index, try to allocate another one!
				if (allocMode == CannotAlloc || !new_block_index()) {
					return false;
				}
				localBlockIndex = blockIndex.load(std::memory_order_relaxed);
				newTail = (localBlockIndex->tail.load(std::memory_order_relaxed) + 1) & (localBlockIndex->capacity - 1);
				idxEntry = localBlockIndex->index[newTail];
				assert(idxEntry->key.load(std::memory_order_relaxed) == INVALID_BLOCK_BASE);
				idxEntry->key.store(blockStartIndex, std::memory_order_relaxed);
				localBlockIndex->tail.store(newTail, std::memory_order_release);
				return true;
			}

			inline void rewind_block_index_tail()
			{
				auto localBlockIndex = blockIndex.load(std::memory_order_relaxed);
				localBlockIndex->tail.store((localBlockIndex->tail.load(std::memory_order_relaxed) - 1) & (localBlockIndex->capacity - 1), std::memory_order_relaxed);
			}

			inline BlockIndexEntry* get_block_index_entry_for_index(index_t index) const
			{
				BlockIndexHeader* localBlockIndex;
				auto idx = get_block_index_index_for_index(index, localBlockIndex);
				return localBlockIndex->index[idx];
			}

			inline size_t get_block_index_index_for_index(index_t index, BlockIndexHeader*& localBlockIndex) const
			{
#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
				debug::DebugLock lock(mutex);
#endif
				index &= ~static_cast<index_t>(BLOCK_SIZE - 1);
				localBlockIndex = blockIndex.load(std::memory_order_acquire);
				auto tail = localBlockIndex->tail.load(std::memory_order_acquire);
				auto tailBase = localBlockIndex->index[tail]->key.load(std::memory_order_relaxed);
				assert(tailBase != INVALID_BLOCK_BASE);
				// Note: Must use division instead of shift because the index may wrap around, causing a negative
				// offset, whose negativity we want to preserve
				auto offset = static_cast<size_t>(static_cast<typename std::make_signed<index_t>::type>(index - tailBase) / BLOCK_SIZE);
				size_t idx = (tail + offset) & (localBlockIndex->capacity - 1);
				assert(localBlockIndex->index[idx]->key.load(std::memory_order_relaxed) == index && localBlockIndex->index[idx]->value.load(std::memory_order_relaxed) != nullptr);
				return idx;
			}

			bool new_block_index()
			{
				auto prev = blockIndex.load(std::memory_order_relaxed);
				size_t prevCapacity = prev == nullptr ? 0 : prev->capacity;
				auto entryCount = prev == nullptr ? nextBlockIndexCapacity : prevCapacity;
				auto raw = static_cast<char*>((Traits::malloc)(
					sizeof(BlockIndexHeader) +
					std::alignment_of<BlockIndexEntry>::value - 1 + sizeof(BlockIndexEntry) * entryCount +
					std::alignment_of<BlockIndexEntry*>::value - 1 + sizeof(BlockIndexEntry*) * nextBlockIndexCapacity));
				if (raw == nullptr) {
					return false;
				}

				auto header = new (raw) BlockIndexHeader;
				auto entries = reinterpret_cast<BlockIndexEntry*>(details::align_for<BlockIndexEntry>(raw + sizeof(BlockIndexHeader)));
				auto index = reinterpret_cast<BlockIndexEntry**>(details::align_for<BlockIndexEntry*>(reinterpret_cast<char*>(entries) + sizeof(BlockIndexEntry) * entryCount));
				if (prev != nullptr) {
					auto prevTail = prev->tail.load(std::memory_order_relaxed);
					auto prevPos = prevTail;
					size_t i = 0;
					do {
						prevPos = (prevPos + 1) & (prev->capacity - 1);
						index[i++] = prev->index[prevPos];
					} while (prevPos != prevTail);
					assert(i == prevCapacity);
				}
				for (size_t i = 0; i != entryCount; ++i) {
					new (entries + i) BlockIndexEntry;
					entries[i].key.store(INVALID_BLOCK_BASE, std::memory_order_relaxed);
					index[prevCapacity + i] = entries + i;
				}
				header->prev = prev;
				header->entries = entries;
				header->index = index;
				header->capacity = nextBlockIndexCapacity;
				header->tail.store((prevCapacity - 1) & (nextBlockIndexCapacity - 1), std::memory_order_relaxed);

				blockIndex.store(header, std::memory_order_release);

				nextBlockIndexCapacity <<= 1;

				return true;
			}

		private:
			size_t nextBlockIndexCapacity;
			std::atomic<BlockIndexHeader*> blockIndex;

#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
		public:
			details::ThreadExitListener threadExitListener;
		private:
#endif

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
		public:
			ImplicitProducer* nextImplicitProducer;
		private:
#endif

#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
			mutable debug::DebugMutex mutex;
#endif
#ifdef MCDBGQ_TRACKMEM
			friend struct MemStats;
#endif
		};

		void populate_initial_block_list(size_t blockCount)
		{
			initialBlockPoolSize = blockCount;
			if (initialBlockPoolSize == 0) {
				initialBlockPool = nullptr;
				return;
			}

			initialBlockPool = create_array<Block>(blockCount);
			if (initialBlockPool == nullptr) {
				initialBlockPoolSize = 0;
			}
			for (size_t i = 0; i < initialBlockPoolSize; ++i) {
				initialBlockPool[i].dynamicallyAllocated = false;
			}
		}

		inline Block* try_get_block_from_initial_pool()
		{
			if (initialBlockPoolIndex.load(std::memory_order_relaxed) >= initialBlockPoolSize) {
				return nullptr;
			}

			auto index = initialBlockPoolIndex.fetch_add(1, std::memory_order_relaxed);

			return index < initialBlockPoolSize ? (initialBlockPool + index) : nullptr;
		}

		inline void add_block_to_free_list(Block* block)
		{
#ifdef MCDBGQ_TRACKMEM
			block->owner = nullptr;
#endif
			freeList.add(block);
		}

		inline void add_blocks_to_free_list(Block* block)
		{
			while (block != nullptr) {
				auto next = block->next;
				add_block_to_free_list(block);
				block = next;
			}
		}

		inline Block* try_get_block_from_free_list()
		{
			return freeList.try_get();
		}

		template<AllocationMode canAlloc>
		Block* requisition_block()
		{
			auto block = try_get_block_from_initial_pool();
			if (block != nullptr) {
				return block;
			}

			block = try_get_block_from_free_list();
			if (block != nullptr) {
				return block;
			}

			if (canAlloc == CanAlloc) {
				return create<Block>();
			}

			return nullptr;
		}


#ifdef MCDBGQ_TRACKMEM
	public:
		struct MemStats {
			size_t allocatedBlocks;
			size_t usedBlocks;
			size_t freeBlocks;
			size_t ownedBlocksExplicit;
			size_t ownedBlocksImplicit;
			size_t implicitProducers;
			size_t explicitProducers;
			size_t elementsEnqueued;
			size_t blockClassBytes;
			size_t queueClassBytes;
			size_t implicitBlockIndexBytes;
			size_t explicitBlockIndexBytes;

			friend class ConcurrentQueue;

		private:
			static MemStats getFor(ConcurrentQueue* q)
			{
				MemStats stats = { 0 };

				stats.elementsEnqueued = q->size_approx();

				auto block = q->freeList.head_unsafe();
				while (block != nullptr) {
					++stats.allocatedBlocks;
					++stats.freeBlocks;
					block = block->freeListNext.load(std::memory_order_relaxed);
				}

				for (auto ptr = q->producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
					bool implicit = dynamic_cast<ImplicitProducer*>(ptr) != nullptr;
					stats.implicitProducers += implicit ? 1 : 0;
					stats.explicitProducers += implicit ? 0 : 1;

					if (implicit) {
						auto prod = static_cast<ImplicitProducer*>(ptr);
						stats.queueClassBytes += sizeof(ImplicitProducer);
						auto head = prod->headIndex.load(std::memory_order_relaxed);
						auto tail = prod->tailIndex.load(std::memory_order_relaxed);
						auto hash = prod->blockIndex.load(std::memory_order_relaxed);
						if (hash != nullptr) {
							for (size_t i = 0; i != hash->capacity; ++i) {
								if (hash->index[i]->key.load(std::memory_order_relaxed) != ImplicitProducer::INVALID_BLOCK_BASE && hash->index[i]->value.load(std::memory_order_relaxed) != nullptr) {
									++stats.allocatedBlocks;
									++stats.ownedBlocksImplicit;
								}
							}
							stats.implicitBlockIndexBytes += hash->capacity * sizeof(typename ImplicitProducer::BlockIndexEntry);
							for (; hash != nullptr; hash = hash->prev) {
								stats.implicitBlockIndexBytes += sizeof(typename ImplicitProducer::BlockIndexHeader) + hash->capacity * sizeof(typename ImplicitProducer::BlockIndexEntry*);
							}
						}
						for (; details::circular_less_than<index_t>(head, tail); head += BLOCK_SIZE) {
							//auto block = prod->get_block_index_entry_for_index(head);
							++stats.usedBlocks;
						}
					}
					else {
						auto prod = static_cast<ExplicitProducer*>(ptr);
						stats.queueClassBytes += sizeof(ExplicitProducer);
						auto tailBlock = prod->tailBlock;
						bool wasNonEmpty = false;
						if (tailBlock != nullptr) {
							auto block = tailBlock;
							do {
								++stats.allocatedBlocks;
								if (!block->ConcurrentQueue::Block::template is_empty<explicit_context>() || wasNonEmpty) {
									++stats.usedBlocks;
									wasNonEmpty = wasNonEmpty || block != tailBlock;
								}
								++stats.ownedBlocksExplicit;
								block = block->next;
							} while (block != tailBlock);
						}
						auto index = prod->blockIndex.load(std::memory_order_relaxed);
						while (index != nullptr) {
							stats.explicitBlockIndexBytes += sizeof(typename ExplicitProducer::BlockIndexHeader) + index->size * sizeof(typename ExplicitProducer::BlockIndexEntry);
							index = static_cast<typename ExplicitProducer::BlockIndexHeader*>(index->prev);
						}
					}
				}

				auto freeOnInitialPool = q->initialBlockPoolIndex.load(std::memory_order_relaxed) >= q->initialBlockPoolSize ? 0 : q->initialBlockPoolSize - q->initialBlockPoolIndex.load(std::memory_order_relaxed);
				stats.allocatedBlocks += freeOnInitialPool;
				stats.freeBlocks += freeOnInitialPool;

				stats.blockClassBytes = sizeof(Block) * stats.allocatedBlocks;
				stats.queueClassBytes += sizeof(ConcurrentQueue);

				return stats;
			}
		};

		// For debugging only. Not thread-safe.
		MemStats getMemStats()
		{
			return MemStats::getFor(this);
		}
	private:
		friend struct MemStats;
#endif

		ProducerBase* recycle_or_create_producer(bool isExplicit)
		{
			bool recycled;
			return recycle_or_create_producer(isExplicit, recycled);
		}

		ProducerBase* recycle_or_create_producer(bool isExplicit, bool& recycled)
		{
#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODHASH
			debug::DebugLock lock(implicitProdMutex);
#endif
			// Try to re-use one first
			for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
				if (ptr->inactive.load(std::memory_order_relaxed) && ptr->isExplicit == isExplicit) {
					bool expected = true;
					if (ptr->inactive.compare_exchange_strong(expected, /* desired */ false, std::memory_order_acquire, std::memory_order_relaxed)) {
						// We caught one! It's been marked as activated, the caller can have it
						recycled = true;
						return ptr;
					}
				}
			}

			recycled = false;
			return add_producer(isExplicit ? static_cast<ProducerBase*>(create<ExplicitProducer>(this)) : create<ImplicitProducer>(this));
		}

		ProducerBase* add_producer(ProducerBase* producer)
		{
			if (producer == nullptr) {
				return nullptr;
			}

			producerCount.fetch_add(1, std::memory_order_relaxed);
			auto prevTail = producerListTail.load(std::memory_order_relaxed);
			do {
				producer->next = prevTail;
			} while (!producerListTail.compare_exchange_weak(prevTail, producer, std::memory_order_release, std::memory_order_relaxed));

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
			if (producer->isExplicit) {
				auto prevTailExplicit = explicitProducers.load(std::memory_order_relaxed);
				do {
					static_cast<ExplicitProducer*>(producer)->nextExplicitProducer = prevTailExplicit;
				} while (!explicitProducers.compare_exchange_weak(prevTailExplicit, static_cast<ExplicitProducer*>(producer), std::memory_order_release, std::memory_order_relaxed));
			}
			else {
				auto prevTailImplicit = implicitProducers.load(std::memory_order_relaxed);
				do {
					static_cast<ImplicitProducer*>(producer)->nextImplicitProducer = prevTailImplicit;
				} while (!implicitProducers.compare_exchange_weak(prevTailImplicit, static_cast<ImplicitProducer*>(producer), std::memory_order_release, std::memory_order_relaxed));
			}
#endif

			return producer;
		}

		void reown_producers()
		{
			for (auto ptr = producerListTail.load(std::memory_order_relaxed); ptr != nullptr; ptr = ptr->next_prod()) {
				ptr->parent = this;
			}
		}

		struct ImplicitProducerKVP
		{
			std::atomic<details::thread_id_t> key;
			ImplicitProducer* value;

			ImplicitProducerKVP() : value(nullptr) { }

			ImplicitProducerKVP(ImplicitProducerKVP&& other) MOODYCAMEL_NOEXCEPT
			{
				key.store(other.key.load(std::memory_order_relaxed), std::memory_order_relaxed);
				value = other.value;
			}

			inline ImplicitProducerKVP& operator=(ImplicitProducerKVP&& other) MOODYCAMEL_NOEXCEPT
			{
				swap(other);
				return *this;
			}

			inline void swap(ImplicitProducerKVP& other) MOODYCAMEL_NOEXCEPT
			{
				if (this != &other) {
					details::swap_relaxed(key, other.key);
					std::swap(value, other.value);
				}
			}
		};

		template<typename XT, typename XTraits>
		friend void MultiThread::swap(typename ConcurrentQueue<XT, XTraits>::ImplicitProducerKVP&, typename ConcurrentQueue<XT, XTraits>::ImplicitProducerKVP&) MOODYCAMEL_NOEXCEPT;

		struct ImplicitProducerHash
		{
			size_t capacity;
			ImplicitProducerKVP* entries;
			ImplicitProducerHash* prev;
		};

		inline void populate_initial_implicit_producer_hash()
		{
			if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return;

			implicitProducerHashCount.store(0, std::memory_order_relaxed);
			auto hash = &initialImplicitProducerHash;
			hash->capacity = INITIAL_IMPLICIT_PRODUCER_HASH_SIZE;
			hash->entries = &initialImplicitProducerHashEntries[0];
			for (size_t i = 0; i != INITIAL_IMPLICIT_PRODUCER_HASH_SIZE; ++i) {
				initialImplicitProducerHashEntries[i].key.store(details::invalid_thread_id, std::memory_order_relaxed);
			}
			hash->prev = nullptr;
			implicitProducerHash.store(hash, std::memory_order_relaxed);
		}

		void swap_implicit_producer_hashes(ConcurrentQueue& other)
		{
			if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return;
			initialImplicitProducerHashEntries.swap(other.initialImplicitProducerHashEntries);
			initialImplicitProducerHash.entries = &initialImplicitProducerHashEntries[0];
			other.initialImplicitProducerHash.entries = &other.initialImplicitProducerHashEntries[0];

			details::swap_relaxed(implicitProducerHashCount, other.implicitProducerHashCount);

			details::swap_relaxed(implicitProducerHash, other.implicitProducerHash);
			if (implicitProducerHash.load(std::memory_order_relaxed) == &other.initialImplicitProducerHash) {
				implicitProducerHash.store(&initialImplicitProducerHash, std::memory_order_relaxed);
			}
			else {
				ImplicitProducerHash* hash;
				for (hash = implicitProducerHash.load(std::memory_order_relaxed); hash->prev != &other.initialImplicitProducerHash; hash = hash->prev) {
					continue;
				}
				hash->prev = &initialImplicitProducerHash;
			}
			if (other.implicitProducerHash.load(std::memory_order_relaxed) == &initialImplicitProducerHash) {
				other.implicitProducerHash.store(&other.initialImplicitProducerHash, std::memory_order_relaxed);
			}
			else {
				ImplicitProducerHash* hash;
				for (hash = other.implicitProducerHash.load(std::memory_order_relaxed); hash->prev != &initialImplicitProducerHash; hash = hash->prev) {
					continue;
				}
				hash->prev = &other.initialImplicitProducerHash;
			}
		}

		ImplicitProducer* get_or_add_implicit_producer()
		{
#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODHASH
			debug::DebugLock lock(implicitProdMutex);
#endif

			auto id = details::thread_id();
			auto hashedId = details::hash_thread_id(id);

			auto mainHash = implicitProducerHash.load(std::memory_order_acquire);
			for (auto hash = mainHash; hash != nullptr; hash = hash->prev) {
				auto index = hashedId;
				while (true) {
					index &= hash->capacity - 1;

					auto probedKey = hash->entries[index].key.load(std::memory_order_relaxed);
					if (probedKey == id) {
						auto value = hash->entries[index].value;
						if (hash != mainHash) {
							index = hashedId;
							while (true) {
								index &= mainHash->capacity - 1;
								probedKey = mainHash->entries[index].key.load(std::memory_order_relaxed);
								auto empty = details::invalid_thread_id;
#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
								auto reusable = details::invalid_thread_id2;
								if ((probedKey == empty    && mainHash->entries[index].key.compare_exchange_strong(empty,    id, std::memory_order_relaxed, std::memory_order_relaxed)) ||
									(probedKey == reusable && mainHash->entries[index].key.compare_exchange_strong(reusable, id, std::memory_order_acquire, std::memory_order_acquire))) {
#else
								if ((probedKey == empty    && mainHash->entries[index].key.compare_exchange_strong(empty,    id, std::memory_order_relaxed, std::memory_order_relaxed))) {
#endif
									mainHash->entries[index].value = value;
									break;
								}
								++index;
							}
						}

						return value;
					}
					if (probedKey == details::invalid_thread_id) {
						break;		// Not in this hash table
					}
					++index;
				}
			}

			// Insert!
			auto newCount = 1 + implicitProducerHashCount.fetch_add(1, std::memory_order_relaxed);
			while (true) {
				if (newCount >= (mainHash->capacity >> 1) && !implicitProducerHashResizeInProgress.test_and_set(std::memory_order_acquire)) {
					mainHash = implicitProducerHash.load(std::memory_order_acquire);
					if (newCount >= (mainHash->capacity >> 1)) {
						auto newCapacity = mainHash->capacity << 1;
						while (newCount >= (newCapacity >> 1)) {
							newCapacity <<= 1;
						}
						auto raw = static_cast<char*>((Traits::malloc)(sizeof(ImplicitProducerHash) + std::alignment_of<ImplicitProducerKVP>::value - 1 + sizeof(ImplicitProducerKVP) * newCapacity));
						if (raw == nullptr) {
							// Allocation failed
							implicitProducerHashCount.fetch_sub(1, std::memory_order_relaxed);
							implicitProducerHashResizeInProgress.clear(std::memory_order_relaxed);
							return nullptr;
						}

						auto newHash = new (raw) ImplicitProducerHash;
						newHash->capacity = newCapacity;
						newHash->entries = reinterpret_cast<ImplicitProducerKVP*>(details::align_for<ImplicitProducerKVP>(raw + sizeof(ImplicitProducerHash)));
						for (size_t i = 0; i != newCapacity; ++i) {
							new (newHash->entries + i) ImplicitProducerKVP;
							newHash->entries[i].key.store(details::invalid_thread_id, std::memory_order_relaxed);
						}
						newHash->prev = mainHash;
						implicitProducerHash.store(newHash, std::memory_order_release);
						implicitProducerHashResizeInProgress.clear(std::memory_order_release);
						mainHash = newHash;
					}
					else {
						implicitProducerHashResizeInProgress.clear(std::memory_order_release);
					}
				}

				if (newCount < (mainHash->capacity >> 1) + (mainHash->capacity >> 2)) {
					bool recycled;
					auto producer = static_cast<ImplicitProducer*>(recycle_or_create_producer(false, recycled));
					if (producer == nullptr) {
						implicitProducerHashCount.fetch_sub(1, std::memory_order_relaxed);
						return nullptr;
					}
					if (recycled) {
						implicitProducerHashCount.fetch_sub(1, std::memory_order_relaxed);
					}

#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
					producer->threadExitListener.callback = &ConcurrentQueue::implicit_producer_thread_exited_callback;
					producer->threadExitListener.userData = producer;
					details::ThreadExitNotifier::subscribe(&producer->threadExitListener);
#endif

					auto index = hashedId;
					while (true) {
						index &= mainHash->capacity - 1;
						auto probedKey = mainHash->entries[index].key.load(std::memory_order_relaxed);

						auto empty = details::invalid_thread_id;
#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
						auto reusable = details::invalid_thread_id2;
						if ((probedKey == empty    && mainHash->entries[index].key.compare_exchange_strong(empty,    id, std::memory_order_relaxed, std::memory_order_relaxed)) ||
							(probedKey == reusable && mainHash->entries[index].key.compare_exchange_strong(reusable, id, std::memory_order_acquire, std::memory_order_acquire))) {
#else
						if ((probedKey == empty    && mainHash->entries[index].key.compare_exchange_strong(empty,    id, std::memory_order_relaxed, std::memory_order_relaxed))) {
#endif
							mainHash->entries[index].value = producer;
							break;
						}
						++index;
					}
					return producer;
				}
				mainHash = implicitProducerHash.load(std::memory_order_acquire);
			}
		}

#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
		void implicit_producer_thread_exited(ImplicitProducer* producer)
		{
			details::ThreadExitNotifier::unsubscribe(&producer->threadExitListener);
#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODHASH
			debug::DebugLock lock(implicitProdMutex);
#endif
			auto hash = implicitProducerHash.load(std::memory_order_acquire);
			assert(hash != nullptr);
			auto id = details::thread_id();
			auto hashedId = details::hash_thread_id(id);
			details::thread_id_t probedKey;

			for (; hash != nullptr; hash = hash->prev) {
				auto index = hashedId;
				do {
					index &= hash->capacity - 1;
					probedKey = hash->entries[index].key.load(std::memory_order_relaxed);
					if (probedKey == id) {
						hash->entries[index].key.store(details::invalid_thread_id2, std::memory_order_release);
						break;
					}
					++index;
				} while (probedKey != details::invalid_thread_id);
			}
			producer->inactive.store(true, std::memory_order_release);
		}

		static void implicit_producer_thread_exited_callback(void* userData)
		{
			auto producer = static_cast<ImplicitProducer*>(userData);
			auto queue = producer->parent;
			queue->implicit_producer_thread_exited(producer);
		}
#endif

		template<typename U>
		static inline U* create_array(size_t count)
		{
			assert(count > 0);
			auto p = static_cast<U*>((Traits::malloc)(sizeof(U) * count));
			if (p == nullptr) {
				return nullptr;
			}

			for (size_t i = 0; i != count; ++i) {
				new (p + i) U();
			}
			return p;
		}

		template<typename U>
		static inline void destroy_array(U* p, size_t count)
		{
			if (p != nullptr) {
				assert(count > 0);
				for (size_t i = count; i != 0; ) {
					(p + --i)->~U();
				}
				(Traits::free)(p);
			}
		}

		template<typename U>
		static inline U* create()
		{
			auto p = (Traits::malloc)(sizeof(U));
			return p != nullptr ? new (p) U : nullptr;
		}

		template<typename U, typename A1>
		static inline U* create(A1&& a1)
		{
			auto p = (Traits::malloc)(sizeof(U));
			return p != nullptr ? new (p) U(std::forward<A1>(a1)) : nullptr;
		}

		template<typename U>
		static inline void destroy(U* p)
		{
			if (p != nullptr) {
				p->~U();
			}
			(Traits::free)(p);
		}

	private:
		std::atomic<ProducerBase*> producerListTail;
		std::atomic<std::uint32_t> producerCount;

		std::atomic<size_t> initialBlockPoolIndex;
		Block* initialBlockPool;
		size_t initialBlockPoolSize;

#if !MCDBGQ_USEDEBUGFREELIST
		FreeList<Block> freeList;
#else
		debug::DebugFreeList<Block> freeList;
#endif

		std::atomic<ImplicitProducerHash*> implicitProducerHash;
		std::atomic<size_t> implicitProducerHashCount;		// Number of slots logically used
		ImplicitProducerHash initialImplicitProducerHash;
		std::array<ImplicitProducerKVP, INITIAL_IMPLICIT_PRODUCER_HASH_SIZE> initialImplicitProducerHashEntries;
		std::atomic_flag implicitProducerHashResizeInProgress;

		std::atomic<std::uint32_t> nextExplicitConsumerId;
		std::atomic<std::uint32_t> globalExplicitConsumerOffset;

#ifdef MCDBGQ_NOLOCKFREE_IMPLICITPRODHASH
		debug::DebugMutex implicitProdMutex;
#endif

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
		std::atomic<ExplicitProducer*> explicitProducers;
		std::atomic<ImplicitProducer*> implicitProducers;
#endif
	};


	template<typename T, typename Traits>
	ProducerToken::ProducerToken(ConcurrentQueue<T, Traits>& queue)
		: producer(queue.recycle_or_create_producer(true))
	{
		if (producer != nullptr) {
			producer->token = this;
		}
	}

	template<typename T, typename Traits>
	ProducerToken::ProducerToken(BlockingConcurrentQueue<T, Traits>& queue)
		: producer(reinterpret_cast<ConcurrentQueue<T, Traits>*>(&queue)->recycle_or_create_producer(true))
	{
		if (producer != nullptr) {
			producer->token = this;
		}
	}

	template<typename T, typename Traits>
	ConsumerToken::ConsumerToken(ConcurrentQueue<T, Traits>& queue)
		: itemsConsumedFromCurrent(0), currentProducer(nullptr), desiredProducer(nullptr)
	{
		initialOffset = queue.nextExplicitConsumerId.fetch_add(1, std::memory_order_release);
		lastKnownGlobalOffset = -1;
	}

	template<typename T, typename Traits>
	ConsumerToken::ConsumerToken(BlockingConcurrentQueue<T, Traits>& queue)
		: itemsConsumedFromCurrent(0), currentProducer(nullptr), desiredProducer(nullptr)
	{
		initialOffset = reinterpret_cast<ConcurrentQueue<T, Traits>*>(&queue)->nextExplicitConsumerId.fetch_add(1, std::memory_order_release);
		lastKnownGlobalOffset = -1;
	}

	template<typename T, typename Traits>
	inline void swap(ConcurrentQueue<T, Traits>& a, ConcurrentQueue<T, Traits>& b) MOODYCAMEL_NOEXCEPT
	{
		a.swap(b);
	}

	inline void swap(ProducerToken& a, ProducerToken& b) MOODYCAMEL_NOEXCEPT
	{
		a.swap(b);
	}

	inline void swap(ConsumerToken& a, ConsumerToken& b) MOODYCAMEL_NOEXCEPT
	{
		a.swap(b);
	}

	template<typename T, typename Traits>
	inline void swap(typename ConcurrentQueue<T, Traits>::ImplicitProducerKVP& a, typename ConcurrentQueue<T, Traits>::ImplicitProducerKVP& b) MOODYCAMEL_NOEXCEPT
	{
		a.swap(b);
	}

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
