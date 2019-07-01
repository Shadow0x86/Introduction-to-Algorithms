#pragma once
#include "utils.h"



namespace lyf
{
	template<class _Ty>
	class FibonacciHeap
	{
	private:

		struct Node
		{
			using node_ptr = Node *;
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

			~Node()
			{
				if (_pChild)
				{
					node_ptr curr = _pChild;
					do
					{
						node_ptr next = curr->_pRight;
						delete curr;
						curr = next;
					} while (curr != _pChild);
				}
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

	public:
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

		~FibonacciHeap()
		{
			delete _pRoot;
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

		FibonacciHeap &operator+=(FibonacciHeap &&rhs) noexcept
		{
			if (this != &rhs)
			{
				if (_pRoot && rhs._pRoot)
				{
					_concatenate_childs(_pRoot, rhs._pRoot);
				}
				if (!_pRoot || (rhs._pRoot && rhs.min() < min()))
				{
					_pRoot = rhs._pRoot;
				}
				_Size += rhs._Size;
				rhs._pRoot = nullptr;
				rhs._Size = 0;
			}
			return *this;
		}

		FibonacciHeap &merge(FibonacciHeap &&rhs) noexcept
		{
			return (*this) += std::move(rhs);
		}

		void insert(const value_type &value) noexcept
		{
			_insert_node(new Node(value));
		}

		void insert(value_type &&value) noexcept
		{
			_insert_node(new Node(std::move(value)));
		}

		value_ptr extract_min() noexcept
		{
			auto np = _pRoot;
			value_ptr ret = nullptr;
			if (np)
			{
				node_ptr cp = np->_pChild;
				if (cp)
				{
					node_ptr curr = cp;
					do
					{
						curr->_pParent = nullptr;
						curr = curr->_pRight;
					} while (curr != cp);
					_concatenate_childs(np, cp);
				}
				np->_pLeft->_pRight = np->_pRight;
				np->_pRight->_pLeft = np->_pLeft;
				if (np == np->_pRight)
				{
					_pRoot = nullptr;
				}
				else
				{
					_pRoot = np->_pRight;
					_consolidate();
				}
				_Size--;
				ret = std::move(np->_pValue);
				delete np;
			}
			return ret;
		}

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

		void _concatenate_childs(node_ptr lc, node_ptr rc) noexcept
		{
			lc->_pRight->_pLeft = rc->_pLeft;
			rc->_pLeft->_pRight = lc->_pRight;
			lc->_pRight = rc;
			rc->_pLeft = lc;
		}

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

		void _consolidate()
		{

		}
	};


	template<class _Ty>
	FibonacciHeap<_Ty> operator+(FibonacciHeap<_Ty> &&lhs, FibonacciHeap<_Ty> &&rhs) noexcept
	{
		FibonacciHeap<_Ty> ret(std::move(lhs));
		return ret += std::move(rhs);
	}
}
