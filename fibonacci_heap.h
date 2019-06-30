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
				: _pValue(new _Ty(value)), _pParent(nullptr), _Degree(0),
				_pChild(nullptr), _pLeft(nullptr), _pRight(nullptr), _Mark(false)
			{
			}

			Node(_Ty &&value)
				: _pValue(new _Ty(std::move(value))), _pParent(nullptr), _Degree(0),
				_pChild(nullptr), _pLeft(nullptr), _pRight(nullptr), _Mark(false)
			{
			}

			const _Ty &value() const noexcept
			{
				return *_pValue;
			}

			value_ptr _pValue;
			node_ptr _pParent;
			node_ptr _pChild;
			node_ptr _pLeft;
			node_ptr _pRight;
			size_t _Degree;
			bool _Mark;
		};

		using value_type = _Ty;
		using node_ptr = typename Node::node_ptr;
		using value_ptr = typename Node::value_ptr;
		
	public:
		FibonacciHeap()
			: _pRoot(nullptr), _Size(0)
		{
		}

		FibonacciHeap(FibonacciHeap &&rhs)
			: _pRoot(rhs._pRoot), _Size(rhs._Size)
		{
			rhs._pRoot = nullptr;
			rhs._Size = 0;
		}

		FibonacciHeap &operator=(FibonacciHeap &&rhs)
		{
			if (this != &rhs)
			{
				_pRoot = rhs._pRoot;
				_Size = rhs._Size;
				rhs._pRoot = nullptr;
				rhs._Size = 0;
			}
			return *this;
		}

		FibonacciHeap(const FibonacciHeap &) = delete;
		FibonacciHeap &operator=(const FibonacciHeap &) = delete;
		FibonacciHeap &operator+=(const FibonacciHeap &rhs) = delete;

		FibonacciHeap &operator+=(FibonacciHeap &&rhs);

		void insert(const value_type &value) noexcept
		{
			_insert_node(new Node(value));
		}

		void insert(value_type &&value) noexcept
		{
			_insert_node(new Node(std::move(value)));
		}

		value_ptr extract_min();

		size_t size() const noexcept
		{
			return _Size;
		}

		bool empty() const noexcept
		{
			return _Size == 0;
		}

		const value_type &min() const noexcept
		{
			return _pRoot->value();
		}

	private:
		node_ptr _pRoot;
		size_t _Size;

		void _insert_node(node_ptr np) noexcept
		{
			if (!_pRoot)
			{
				_pRoot = np;
				np->_pLeft = np->_pRight = np;
			}
			else
			{
				_pRoot->_pRight->_Left = np;
				np->_pRight = _pRoot->_pRight;
				np->_pLeft = _pRoot;
				_pRoot->pRight = np;
				if (np->value() < _pRoot->value())
					_pRoot = np;
			}
			_Size++;
		}
	};


	template<class _Ty>
	FibonacciHeap<_Ty> operator+(FibonacciHeap<_Ty> &&lhs, FibonacciHeap<_Ty> &&rhs)
	{
		FibonacciHeap<_Ty> ret(std::move(lhs));
		ret += std::move(rhs);
		return ret;
	}
}
