#pragma once
#include "utils.h"
#include "heap.h"




namespace lyf
{

	template<typename Iter, typename Key>
	void quick_sort(Iter begin, Iter end, Key key);


	template<typename E, unsigned N, typename Key = void*>
	void sort(E(&arr)[N], Key key = nullptr)
	{
		quick_sort(arr, arr + N, key);
	}



	template<typename Iter, typename Key = void*>
	void insertion_sort(Iter begin, Iter end, Key key = nullptr)
	{	// Sort range [begin, end) in O(n²) time in place stably.
		if (!_isValidRange(begin, end))
			return;

		using K = _Sort_Key<Key, typename Iter_traits<Iter>::value_type>;

		for (auto outerit = begin + 1; outerit != end; outerit++)
		{
			auto innerit = outerit;
			auto v = MOVE(*outerit);
			while (1)
			{
				innerit--;
				if (!(K::get(key, v) < K::get(key, *innerit)))
				{
					innerit++;
					break;
				}
				*(innerit + 1) = MOVE(*innerit);
				if (innerit == begin)
					break;
			}
			*innerit = MOVE(v);
		}
	}



	template<typename Iter, typename Key = void*>
	void merge_sort(Iter begin, Iter end, Key key = nullptr)
	{	// Sort range [begin, end) in O(nlgn) time in place stably.
		if (!_isValidRange(begin, end))
			return;

		using E = typename Iter_traits<Iter>::value_type;
		using K = _Sort_Key<Key, E>;

		if (end - begin <= 100)
			return insertion_sort(begin, end, key);

		auto middle = begin + (end - begin) / 2;
		merge_sort(begin, middle, key);
		merge_sort(middle, end, key);

		size_t n1 = middle - begin;
		size_t n2 = end - middle;
		E *arr1 = new E[n1];
		E *arr2 = new E[n2];
		auto it = begin;
		for (size_t i = 0; i != n1; i++)
			arr1[i] = MOVE(*(it++));
		for (size_t i = 0; i != n2; i++)
			arr2[i] = MOVE(*(it++));

		it = begin;
		E *p1 = arr1, *end1 = arr1 + n1, *p2 = arr2, *end2 = arr2 + n2;
		E *rest = nullptr;
		while (it != end)
		{
			if (!(K::get(key, *p2) < K::get(key, *p1)))
			{
				*(it++) = MOVE(*p1);
				if (++p1 == end1)
				{
					rest = p2;
					break;
				}
			}
			else
			{
				*(it++) = MOVE(*p2);
				if (++p2 == end2)
				{
					rest = p1;
					break;
				}
			}
		}
		if (rest)
			while (it != end)
				*(it++) = MOVE(*(rest++));

		delete[] arr1;
		delete[] arr2;
	}



	template<typename Iter, typename Key = void*>
	void bubble_sort(Iter begin, Iter end, Key key = nullptr)
	{	// Sort range [begin, end) in O(n²) time in place stably.
		if (!_isValidRange(begin, end))
			return;

		auto before_end = end - 1;
		for (auto outerit = begin; outerit != before_end; outerit++)
		{
			for (auto innerit = before_end; innerit != outerit; innerit--)
			{
				auto prev = innerit - 1;
				if (iter_less(innerit, prev))
				{
					swap_by_iter(perv, innerit);
				}
			}
		}
	}



	template<typename Iter, typename Key = void*>
	void heap_sort(Iter begin, Iter end, Key key = nullptr)
	{	// Sort range [begin, end) in O(nlgn) time in place
		MaxHeap<Iter, Key>::sort(begin, end, key);
	}



	template<typename Iter, typename Key = void*>
	void quick_sort(Iter begin, Iter end, Key key = nullptr)
	{	// Sort range [begin, end) in O(nlgn) time in place

		using diff_t = typename Iter_traits<Iter>::difference_type;
		diff_t num;
		while ((num = end - begin) > 1)
		{
			if (num <= 32)
				return insertion_sort(begin, end, key);

			Iter mid_first = begin + num / 2;
			Iter mid_last = mid_first;

			while (mid_first != begin)
			{
				if (iter_equal(mid_first - 1, mid_first, key))
					mid_first--;
				else
					break;
			}
			
			while (++mid_last != end && iter_equal(mid_first, mid_last, key));

			Iter left_it = mid_first, right_it = mid_last;
			while (1)
			{
				if (left_it == begin)
					break;
				--left_it;
				if (iter_equal(left_it, mid_first, key))
				{
					swap_by_iter(--mid_first, left_it);
					continue;
				}
				else if (iter_less(mid_first, left_it, key))
				{
					while (right_it != end)
					{
						if (iter_equal(right_it, mid_first, key))
						{
							swap_by_iter(mid_last++, right_it);
						}
						else if (iter_less(right_it, mid_first, key))
							break;
						right_it++;
					}
				}
				else
					continue;
				
				if (right_it == end)
				{
					mid_first--;
					mid_last--;
					if (left_it == mid_first)
						swap_by_iter(mid_first, mid_last);
					else
					{
						auto tmp = MOVE(*mid_first);
						*mid_first = MOVE(*mid_last);
						*mid_last = MOVE(*left_it);
						*left_it = MOVE(tmp);
					}
				}
				else
				{
					swap_by_iter(left_it, right_it++);
				}
			}
			
			while (right_it != end)
			{
				if (iter_equal(right_it, mid_first, key))
				{
					swap_by_iter(mid_last++, right_it);
				}
				else if (iter_less(right_it, mid_first, key))
				{
					if (mid_last == right_it)
						swap_by_iter(mid_first, mid_last);
					else
					{
						auto tmp = MOVE(*mid_last);
						*mid_last = MOVE(*mid_first);
						*mid_first = MOVE(*right_it);
						*right_it = MOVE(tmp);
					}
					mid_first++;
					mid_last++;
				}
				right_it++;
			}

			if (mid_first - begin <= end - mid_last)
			{
				quick_sort(begin, mid_first, key);
				begin = mid_last;
			}
			else
			{
				quick_sort(mid_last, end, key);
				end = mid_first;
			}
		}
	}



	template<typename Iter, typename KeyResult, typename OutIter, typename Key = void*>
	void counting_sort(Iter begin, Iter end, KeyResult limit, OutIter out, Key key = nullptr)
	{	// Sort range [begin, end) in O(nlgn) time stably. The key should map [begin, end) to [0, limit).
		using K = _Sort_Key<Key, typename Iter_traits<Iter>::value_type>;

		if (end - begin <= 1)
			return;
		if (limit <= 1)
			return;

		KeyResult *tp = new KeyResult[limit];
		for (KeyResult i = 0; i < limit; i++)
			tp[i] = 0;
		for (auto it = begin; it != end; it++)
			tp[K::get(key, *it)]++;
		for (KeyResult i = 1; i < limit; i++)
			tp[i] += tp[i - 1];
		Iter it = end - 1;
		while (1)
		{
			*(out + tp[K::get(key, *it)] - 1) = *it;
			tp[K::get(key, *it)]--;
			if (it == begin)
				break;
			it--;
		}

		delete[] tp;
	}



	template<typename Iter, typename Key = void*>
	std::pair<Iter, Iter> minmax_iter(Iter begin, Iter end, Key key = nullptr)
	{	// return the iterators or pointers to the minimum and maximum value in range [begin, end).

		_ensureValidRange(begin, end);

		Iter maxit, minit, it1, it2;
		if ((end - begin) % 2)
		{
			maxit = minit = begin;
			it1 = begin + 1;
		}
		else
		{
			maxit = begin;
			minit = begin + 1;
			if (iter_less(maxit, minit, key))
			{
				maxit = minit;
				minit = begin;
			}
			it1 = begin + 2;
		}
		
		while (it1 != end)
		{
			it2 = it1 + 1;
			bool b = iter_less(it2, it1, key);
			Iter _maxit = b ? it1 : it2;
			Iter _minit = b ? it2 : it1;
			if (iter_less(maxit, _maxit, key))
				maxit = _maxit;
			if (iter_less(_minit, minit, key))
				minit = _minit;
			it1 += 2;
		}

		return std::pair<Iter, Iter>(minit, maxit);
	}

	template<typename Iter, typename Key = void*>
	auto minmax(Iter begin, Iter end, Key key = nullptr)
	{	// return the minimum and maximum value in range [begin, end).
		auto its = minmax_iter(begin, end, key);
		return std::make_pair(*its.first, *its.second);
	}


}



