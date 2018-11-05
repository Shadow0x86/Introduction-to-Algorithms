#pragma once
#include "utils.h"




namespace lyf
{

	template<typename Iter,
		typename ResultType = typename Iter_traits<Iter>::value_type,
		typename Key = void*>
	std::pair<Iter, Iter>
	find_max_subarray_On2(Iter begin, Iter end, ResultType *pResult = nullptr, Key key = nullptr)
	{	// find max subarray in O(n2) time
		using K = _Predicate<Key, typename Iter_traits<Iter>::value_type>;

		if (!_isValidRange(begin, end))
			return std::make_pair(begin, end);

		ResultType max = K::get(key, *begin);
		ResultType zero = max - max, sum = zero;
		Iter left = begin, right = begin + 1;
		if (end - begin > 1)
		{
			auto pre_end = end - 1;
			for (auto outerit = begin; outerit != pre_end; outerit++)
			{
				sum = zero;
				for (auto innerit = outerit; innerit != end; innerit++)
				{
					sum += K::get(key, *innerit);
					if (max < sum)
					{
						max = sum;
						left = outerit;
						right = innerit + 1;
					}
				}
			}
		}
		if (pResult)
			*pResult = max;
		return std::make_pair(left, right);
	}

	template<typename Iter,
		typename ResultType = typename Iter_traits<Iter>::value_type,
		typename Key = void*>
	std::pair<Iter, Iter>
	find_max_subarray_Onlgn(Iter begin, Iter end, ResultType *pResult = nullptr, Key key = nullptr)
	{	// find max subarray in O(nlgn) time
		using K = _Predicate<Key, typename Iter_traits<Iter>::value_type>;

		if (!_isValidRange(begin, end))
			return std::make_pair(begin, end);

		if (end - begin <= 20)
		{
			return find_max_subarray_On2(begin, end, pResult, key);
		}

		auto pair = std::make_pair(begin, end);
		auto mid = begin + (end - begin) / 2;
		ResultType LResult, RResult;
		auto left_pair = find_max_subarray_Onlgn(begin, mid, &LResult, key);
		auto right_pair = find_max_subarray_Onlgn(mid, end, &RResult, key);
		// calc left half subarray
		auto it = mid - 1;
		auto left_result = K::get(key, *it);
		auto left_max = left_result;
		auto left_bound = it;
		if (it > begin)
		{
			while (1)
			{
				left_result = left_result + K::get(key, *--it);
				if (left_max < left_result)
				{
					left_max = left_result;
					left_bound = it;
				}
				if (it == begin)
					break;
			}
		}
		// calc right half subarray
		auto right_result = K::get(key, *mid);
		auto right_max = right_result;
		auto right_bound = mid + 1;
		for (auto it = mid + 1; it != end; it++)
		{
			right_result = right_result + K::get(key, *it);
			if (right_max < right_result)
			{
				right_max = right_result;
				right_bound = it + 1;
			}
		}
		// merge the two half results
		auto cross_result = left_max + right_max;

		if (cross_result < LResult && RResult < LResult)
		{
			if (pResult)
				*pResult = LResult;
			return left_pair;
		}
		else if (cross_result < RResult && LResult < RResult)
		{
			if (pResult)
				*pResult = RResult;
			return right_pair;
		}
		else
		{
			if (pResult)
				*pResult = cross_result;
			return std::make_pair(left_bound, right_bound);
		}

	}

	template<typename Iter,
		typename ResultType = typename Iter_traits<Iter>::value_type,
		typename Key = void*>
	std::pair<Iter, Iter>
	find_max_subarray_On(Iter begin, Iter end, ResultType *pResult = nullptr, Key key = nullptr)
	{	// find max subarray in O(n) time
		using K = _Predicate<Key, typename Iter_traits<Iter>::value_type>;

		if (!_isValidRange(begin, end))
			return std::make_pair(begin, end);

		ResultType max = K::get(key, *begin);
		ResultType sum = max, zero = max - max;
		Iter left = begin, right = begin + 1;
		Iter inner_left = left;
		if (sum < zero)
		{
			sum = zero;
			inner_left = right;
		}
		for (auto it = right; it != end; it++)
		{
			sum += K::get(key, *it);
			if (max < sum)
			{
				max = sum;
				left = inner_left;
				right = it + 1;
			}
			if (sum < zero)
			{
				sum = zero;
				inner_left = it + 1;
			}
		}
		if (pResult)
			*pResult = max;
		return std::make_pair(left, right);
	}

	void test_find_max_subarray()
	{
		using testType = int;

		const size_t n = 100000;
		//Person *arr = new Person[n];
		testType *arr1 = new testType[n];
		testType *arr2 = new testType[n];
		testType *arr3 = new testType[n];

		for (size_t i = 0; i != n; i++)
		{
			auto p = TestType<testType>::newObj();
			arr1[i] = p;
			arr2[i] = p;
			arr3[i] = p;
		}

		testType r1 = 0, r2 = 0, r3 = 0;

		auto t1 = system_clock::now();
		auto pair1 = lyf::find_max_subarray_On2(arr1, arr1 + n, &r1);
		auto t2 = system_clock::now();
		auto pair2 = lyf::find_max_subarray_Onlgn(arr2, arr2 + n, &r2);
		auto t3 = system_clock::now();
		auto pair3 = lyf::find_max_subarray_On(arr3, arr3 + n, &r3);
		auto t4 = system_clock::now();
		auto d = duration<double>(t2 - t1);
		cout << "cost1: " << d.count() << endl;
		cout << "sum1: " << r1 << endl;
		cout << "begin1: " << pair1.first - arr1 << "\tend1: " << pair1.second - arr1 << endl;
		//for (auto i = 0; i != n; i++)
		//	cout << arr1[i] << " ";
		//cout << endl;
		d = duration<double>(t3 - t2);
		cout << "cost2: " << d.count() << endl;
		cout << "sum2: " << r2 << endl;
		cout << "begin2: " << pair2.first - arr2 << "\tend2: " << pair2.second - arr2 << endl;
		//for (auto i = 0; i != n; i++)
		//	cout << arr2[i] << " ";
		//cout << endl;
		d = duration<double>(t4 - t3);
		cout << "cost3: " << d.count() << endl;
		cout << "sum3: " << r2 << endl;
		cout << "begin3: " << pair3.first - arr3 << "\tend3: " << pair3.second - arr3 << endl;
	}


}
