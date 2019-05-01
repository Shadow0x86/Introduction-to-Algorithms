#pragma once
#include "utils.h"



namespace lyf
{
	struct _HeapMax {};
	struct _HeapMin {};

	template<typename HeapType, typename Iter, typename Key>
	struct _Heap_traits;

	template<typename Iter, typename Key>
	struct _Heap_traits<_HeapMax, Iter, Key>
	{
		INLINE static bool compare(Iter it1, Iter it2, Key key)
		{
			return iter_less(it2, it1, key);
		}
	};

	template<typename Iter, typename Key>
	struct _Heap_traits<_HeapMin, Iter, Key>
	{
		INLINE static bool compare(Iter it1, Iter it2, Key key)
		{
			return iter_less(it1, it2, key);
		}
	};

	template<typename HeapType, typename Iter, typename Key = void*>
	class BaseHeap
	{
		template<typename HeapType, typename Iter, typename Key>
		friend std::ostream&
		operator<<(std::ostream &out, const BaseHeap<HeapType, Iter, Key> &heap)
		{
			for (auto it = heap.begin; it != heap.end; it++)
				out << *it << " ";
			return out;
		}
	public:
		using value_type = typename Iter_traits<Iter>::value_type;
		using difference_type = typename Iter_traits<Iter>::difference_type;

		explicit BaseHeap(Key key = nullptr)
			: key(key)
		{
		}

		BaseHeap(Iter begin, Iter end, Key key = nullptr)
			: begin(begin), end(end), key(key)
		{
			_ensureValidRange(begin, end);
		}

		static BaseHeap build(Iter begin, Iter end, Key key = nullptr)
		{
			auto ret = BaseHeap(begin, end, key);
			ret.build();
			return ret;
		}

		bool reset(Iter begin, Iter end)
		{
			if (!_isValidRange(begin, end))
				return false;
			this->begin = begin;
			this->end = end;
			return true;
		}

		static void sort(Iter begin, Iter end, Key key = nullptr)
		{
			if (!_isValidRange(begin, end))
				return;
			BaseHeap(begin, end, key).sort();
		}

		INLINE Iter parent(const Iter it) const
		{
			return (begin < it && it < end) ? ((it - begin - 1) / 2 + begin) : end;
		}
		INLINE Iter left(const Iter it) const
		{
#if DEBUG
			if ((it - begin) * 2 + 1 > size())
				return end;
#endif
			return (it - begin) * 2 + begin + 1;
		}
		INLINE Iter right(const Iter it) const
		{
#if DEBUG
			if ((it - begin) * 2 + 2 > size())
				return end;
#endif
			return (it - begin) * 2 + begin + 2;
		}

		INLINE difference_type size() const { return end - begin; }

		INLINE bool inRange(const Iter it) const
		{
			return begin <= it && it < end;
		}

		void heapify_down(Iter it) const
		{	// Maintain the heap property from root to leaf in O(lgn) time.
			if (!inRange(it))
				return;
			auto most = it;
			while (1)
			{
				auto l = left(it);
				auto r = right(it);
				if (inRange(l) && _Heap_traits<HeapType, Iter, Key>::compare(l, it, key))
					most = l;
				if (inRange(r) && _Heap_traits<HeapType, Iter, Key>::compare(r, most, key))
					most = r;
				if (most != it)
				{
					swap_by_iter(most, it);
					it = most;
				}
				else
					break;
			}
		}

		void heapify_up(Iter it) const
		{	// Maintain the heap property from leaf to root in O(lgn) time.
			auto p = parent(it);
			while (inRange(p) && _Heap_traits<HeapType, Iter, Key>::compare(it, p, key))
			{
				swap_by_iter(p, it);
				it = p;
				p = parent(p);
			}
		}

		bool build() const
		{	// Build the heap in O(n) time.
			if (size() <= 1)
				return false;
			auto it = (end - begin) / 2 + begin - 1;
			while (1)
			{
				heapify_down(it);
				if (it == begin)
					break;
				--it;
			}
			return true;
		}

		void sort()
		{
			if (!build())
				return;
			while (--end != begin)
			{
				swap_by_iter(begin, end);
				heapify_down(begin);
			}
		}

	private:
		Iter begin;
		Iter end;
		Key key = nullptr;
	};


	template<typename Iter, typename Key = void*>
	using MaxHeap = BaseHeap<_HeapMax, Iter, Key>;

	template<typename Iter, typename Key = void*>
	using MinHeap = BaseHeap<_HeapMin, Iter, Key>;


	template<typename HeapType,
		typename Ele,
		typename Key = void*,
		typename Container = std::vector<Ele>>
	class BasePriorityQueue
	{	// priority queue by heap
	private:
		using value_type = Ele;
		using iter_type = typename Container::iterator;
		using heap_type = BaseHeap<HeapType, iter_type, Key>;
	public:
		explicit BasePriorityQueue(Key key = nullptr)
			:heap(heap_type(key))
		{
		}

		explicit BasePriorityQueue(size_t init_cap, Key key = nullptr)
			:BasePriorityQueue(key)
		{
			data.reserve(init_cap);
		}

		template<typename Iter>
		BasePriorityQueue(Iter begin, Iter end, Key key = nullptr)
			: heap(heap_type(key)), data(Container(begin, end))
		{
			reset_heap();
			heap.build();
		}

		size_t size() const { return data.size(); }

		void insert(value_type v)
		{
			data.push_back(v);
			reset_heap();
			heap.heapify_up(data.end() - 1);
		}

		value_type get() const
		{
			return data.front();
		}

		value_type pop()
		{
			auto r = data.front();
			swap_by_iter(data.begin(), data.end() - 1);
			data.pop_back();
			if (reset_heap())
				heap.heapify_down(data.begin());
			return r;
		}

		void clear() { data.clear(); }

		bool empty() const { return size() == 0; }

	private:
		Container data;
		heap_type heap;

		bool reset_heap()
		{
			return heap.reset(data.begin(), data.end());
		}

	};


	template<typename Ele,
		typename Key = void*,
		typename Container = std::vector<Ele>>
	using MaxPriorityQueue = BasePriorityQueue<_HeapMax, Ele, Key, Container>;

	template<typename Ele,
		typename Key = void*,
		typename Container = std::vector<Ele>>
	using MinPriorityQueue = BasePriorityQueue<_HeapMin, Ele, Key, Container>;

	template<typename Ele,
		typename Key = void*,
		typename Container = std::vector<Ele>,
		typename X = typename std::enable_if<std::is_class_v<Key> || std::is_pointer_v<Key>>::type>
	auto newMaxPriorityQueue(Key key = nullptr)
	{
		return MaxPriorityQueue<Ele, Key, Container>(key);
	}

	template<typename Ele,
		typename Key = void*,
		typename Container = std::vector<Ele>>
	auto newMaxPriorityQueue(size_t init_cap, Key key = nullptr)
	{
		return MaxPriorityQueue<Ele, Key, Container>(init_cap, key);
	}

	template<typename Iter,
		typename Ele = typename Iter_traits<Iter>::value_type,
		typename Key = void*,
		typename Container = std::vector<Ele>>
	auto newMaxPriorityQueue(Iter begin, Iter end, Key key = nullptr)
	{
		return MaxPriorityQueue<Ele, Key, Container>(begin, end, key);
	}

	template<typename Ele,
		typename Key = void*,
		typename Container = std::vector<Ele>,
		typename X = typename std::enable_if<std::is_class_v<Key> || std::is_pointer_v<Key>>::type>
	auto newMinPriorityQueue(Key key = nullptr)
	{
		return MinPriorityQueue<Ele, Key, Container>(key);
	}

	template<typename Ele,
		typename Key = void*,
		typename Container = std::vector<Ele>>
	auto newMinPriorityQueue(size_t init_cap, Key key = nullptr)
	{
		return MinPriorityQueue<Ele, Key, Container>(init_cap, key);
	}

	template<typename Iter,
		typename Ele = typename Iter_traits<Iter>::value_type,
		typename Key = void*,
		typename Container = std::vector<Ele>>
	auto newMinPriorityQueue(Iter begin, Iter end, Key key = nullptr)
	{
		return MinPriorityQueue<Ele, Key, Container>(begin, end, key);
	}
}
