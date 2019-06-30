#pragma once
#include "utils.h"



namespace lyf
{
	template<class _Ty>
	class FibonacciHeap
	{
	public:

		struct Node
		{
			using node_ptr = std::shared_ptr<_Ty>;
			using value_ptr = std::unique_ptr<_Ty>;

			Node(const _Ty &value)
				: _Value(new _Ty(value)), _Parent(nullptr),
				_Child(nullptr), _Left(nullptr), _Right(nullptr)
			{
			}

			Node(_Ty &&value)
				: _Value(new _Ty(std::move(value))), _Parent(nullptr),
				_Child(nullptr), _Left(nullptr), _Right(nullptr)
			{
			}

			value_ptr _pValue;
			node_ptr _Parent;
			node_ptr _Child;
			node_ptr _Left;
			node_ptr _Right;
		};

		using value_type = _Ty;
		using node_ptr = typename Node::node_ptr;
		using value_ptr = typename Node::value_ptr;
		
	public:
		FibonacciHeap()
			: _Root(nullptr)
		{
		}

		FibonacciHeap(FibonacciHeap &&rhs)
			: _Root(rhs._Root)
		{
			rhs._Root = nullptr;
		}

		FibonacciHeap &operator=(FibonacciHeap &&rhs)
		{
			if (this != &rhs)
			{
				_Root = rhs._Root;
				rhs._Root = nullptr;
			}
			return *this;
		}

		FibonacciHeap(const FibonacciHeap &) = delete;
		FibonacciHeap &operator=(const FibonacciHeap &) = delete;
		FibonacciHeap &operator+=(const FibonacciHeap &rhs) = delete;

		FibonacciHeap &operator+=(FibonacciHeap &&rhs);

		void insert(const value_type &value);
		void insert(value_type &&value);
		value_ptr extract_min();

		size_t size() const;
		bool empty() const;
		const value_type &min() const;

	private:
		node_ptr _Root;

	};


	template<class _Ty>
	FibonacciHeap<_Ty> operator+(FibonacciHeap<_Ty> &&lhs, FibonacciHeap<_Ty> &&rhs)
	{
		FibonacciHeap<_Ty> ret(std::move(lhs));
		ret += std::move(rhs);
		return ret;
	}
}
