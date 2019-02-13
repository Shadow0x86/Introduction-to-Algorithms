#pragma once
#include <vector>
#include <deque>
#include "utils.h"


namespace lyf
{
	template<typename T>
	class _AbsColl
	{
	public:
		virtual ~_AbsColl() {}

		virtual size_t size() const
		{
			return _size;
		}
		bool empty()
		{
			return size() == 0;
		}
		virtual void clear() = 0;

		virtual void push(const T &value) = 0;
		virtual void push(T &&value) = 0;
		virtual T pop() = 0;

	protected:
		size_t _size = 0;
	};

	template<typename T, typename Container>
	class _StackQueue : public _AbsColl<T>
	{
	public:
		_StackQueue() {}

		_StackQueue(const _StackQueue &rhs)
			:data(rhs.data.begin(), rhs.data.end())
		{
		}

		_StackQueue(_StackQueue &&rhs)
			: data(std::move(rhs.data))
		{
		}

		_StackQueue &operator=(const _StackQueue &rhs)
		{
			if (this != &rhs)
			{
				data.assign(rhs.data.begin(), rhs.data.end());
			}
			return *this;
		}

		_StackQueue &operator=(_StackQueue &&rhs)
		{
			if (this != &rhs)
			{
				data = std::move(rhs.data);
			}
			return *this;
		}

		size_t size() const
		{
			return data.size();
		}
		void clear()
		{
			data.clear();
		}
		void push(const T &value)
		{
			data.push_back(value);
		}
		void push(T &&value)
		{
			data.push_back(std::move(value));
		}

	protected:
		Container data;
	};

	template<typename T, typename Container = std::vector<T>>
	class Stack : public _StackQueue<T, Container>
	{
	public:
		T top() const
		{
			return this->data.back();
		}
		T pop()
		{
			T ret = top();
			this->data.pop_back();
			return ret;
		}
	};

	template<typename T, typename Container = std::deque<T>>
	class Queue : public _StackQueue<T, Container>
	{
	public:
		T head() const
		{
			return this->data.front();
		}
		T pop()
		{
			T ret = head();
			this->data.pop_front();
			return ret;
		}
	};


	template<typename Valt, typename Node>
	class _BaseLinkedList;

	template<typename Valt, typename BaseNode>
	class ForwardLinkedList;

	template<typename Valt, typename BaseNode>
	class LinkedList;

	template<typename Valt, typename Base>
	class ForwardLinkedListNode : public Base
	{
		friend class _CheckedNodeContainer<ForwardLinkedListNode>;
		friend class _BaseLinkedList<Valt, ForwardLinkedListNode>;
		friend class ForwardLinkedList<Valt, Base>;
	public:
		using _MyBase = Base;
		using nodeptr = typename _CheckedNodeContainer<ForwardLinkedListNode>::nodeptr;

		ForwardLinkedListNode &operator=(const ForwardLinkedListNode &rhs)
		{	// not change next
			this->_ensureInCont();
			if (this != &rhs)
			{
				_MyBase::operator=(rhs);
			}
			return *this;
		}

		nodeptr next() const
		{
			this->_ensureInCont();
			return _next;
		}

	private:
		nodeptr _next = nullptr;

		ForwardLinkedListNode() {}

		ForwardLinkedListNode(const ForwardLinkedListNode &rhs)
			: _MyBase(rhs), _next()
		{
		}

		ForwardLinkedListNode(void *pCont, const Valt &value)
			: _MyBase(pCont, value)
		{
		}

		ForwardLinkedListNode(void *pCont, Valt &&value)
			: _MyBase(pCont, std::move(value))
		{
		}

		template<typename... Types>
		ForwardLinkedListNode(void *pCont, Types&&... args)
			: _MyBase(pCont, std::forward<Types>(args)...)
		{
		}

		void _reset_neighbors()
		{
			_next = nullptr;
		}

	};

	template<typename Valt, typename Base>
	class LinkedListNode : public Base
	{
		friend class _CheckedNodeContainer<LinkedListNode>;
		friend class _BaseLinkedList<Valt, LinkedListNode>;
		friend class LinkedList<Valt, Base>;
	public:
		using _MyBase = Base;
		using nodeptr = typename _CheckedNodeContainer<LinkedListNode>::nodeptr;

		LinkedListNode &operator=(const LinkedListNode &rhs)
		{	// not change prev and next
			this->_ensureInCont();
			if (this != &rhs)
			{
				_MyBase::operator=(rhs);
			}
			return *this;
		}
		
		nodeptr prev() const
		{
			this->_ensureInCont();
			return _prev;
		}
		nodeptr next() const
		{
			this->_ensureInCont();
			return _next;
		}

	private:
		nodeptr _prev = nullptr;
		nodeptr _next = nullptr;

		LinkedListNode() {}

		LinkedListNode(const LinkedListNode &rhs)
			: _MyBase(rhs), _prev(), _next()
		{
		}

		LinkedListNode(void *pCont, const Valt &value)
			: _MyBase(pCont, value)
		{
		}

		LinkedListNode(void *pCont, Valt &&value)
			: _MyBase(pCont, std::move(value))
		{
		}

		template<typename... Types>
		LinkedListNode(void *pCont, Types&&... args)
			: _MyBase(pCont, std::forward<Types>(args)...)
		{
		}

		void _reset_neighbors()
		{
			_prev = _next = nullptr;
		}

	};


	template<typename Valt, typename Node>
	class _BaseLinkedList : public _AbsColl<Valt>, public _CheckedNodeContainer<Node>
	{
	public:
		using value_type = Valt;
		using check_t = _CheckedNodeContainer<Node>;
		using nodeptr = typename check_t::nodeptr;

		~_BaseLinkedList()
		{
			_destroy();
		}

		void clear()
		{
			this->_destroy();
			_size = 0;
		}

		value_type head() const
		{
			return _head->value();
		}

		nodeptr head_node() const
		{
			return _head;
		}

		value_type &operator[](size_t index)
		{
#if DEBUG
			if (index >= _size)
				throw std::runtime_error("index out of range");
#endif
			nodeptr nd = _head;
			while (index)
			{
				nd = nd->_next;
				index--;
			}
			return nd->value();
		}

		nodeptr search(const Valt &value) const
		{
			return this->_find_node_by_value(value);
		}

		template<typename Func>
		void map(Func func)
		{
			auto np = _head;
			while (np)
			{
				func(np->value());
				np = np->_next;
			}
		}

	protected:
		nodeptr _head = nullptr;

		void _ensureInList(nodeptr node) const
		{
			node->_ensureInCont(this);
		}

		nodeptr _find_node_by_value(const Valt &tofind) const
		{
			nodeptr np = _head;
			while (np)
			{
				if (np->value() == tofind)
					break;
				np = np->_next;
			}
			return np;
		}

		void _destroy()
		{
			while (_head)
			{
				nodeptr next = _head->_next;
				this->_delete_node(_head);
				_head = next;
			}
		}

	};


	// forward linked list
	template<typename Valt, typename BaseNode = UniqueNode<Valt>>
	class ForwardLinkedList : public _BaseLinkedList<Valt, ForwardLinkedListNode<Valt, BaseNode>>
	{
	public:
		using value_type = Valt;
		using Node = ForwardLinkedListNode<value_type, BaseNode>;
		using _MyBase = _BaseLinkedList<value_type, Node>;
		using check_t = typename _MyBase::check_t;
		using nodeptr = typename _MyBase::nodeptr;

	public:
		ForwardLinkedList() {}

		ForwardLinkedList(std::initializer_list<value_type> list)
			: ForwardLinkedList(list.begin(), list.end())
		{
		}

		template<typename Iter>
		ForwardLinkedList(Iter begin, Iter end)
		{
			auto _riters = riters(begin, end);
			for (auto it = _riters.first; it != _riters.second; it++)
			{
				this->push(*it);
			}
		}

		ForwardLinkedList(const ForwardLinkedList &rhs)
		{
			ForwardLinkedList::_copy_sublist(*this, rhs, rhs.head_node());
		}

		ForwardLinkedList(ForwardLinkedList &&rhs)
			: _MyBase(std::move(rhs))
		{
			rhs._size = 0;
			rhs._head = nullptr;
		}

		ForwardLinkedList &operator=(const ForwardLinkedList &rhs)
		{
			if (this != &rhs)
			{
				_copy_sublist(*this, rhs, rhs.head_node());
			}
			return *this;
		}

		ForwardLinkedList &operator=(ForwardLinkedList &&rhs)
		{
			if (this != &rhs)
			{
				this->_destroy();
				_head = rhs._head;
				_size = rhs._size;
				rhs._head = nullptr;
				rhs._size = 0;
			}
			return *this;
		}

		void reverse()
		{
			if (_size <= 1)
				return;
			nodeptr left = _head, mid = _head->_next, right;
			_head->_next = nullptr;
			while (mid)
			{
				right = mid->_next;
				mid->_next = left;
				left = mid;
				mid = right;
			}
			_head = left;
		}

		ForwardLinkedList sublist(nodeptr begin, nodeptr end = nullptr) const
		{
			ForwardLinkedList ret;
			this->_copy_sublist(ret, *this, begin, end);
			return ret;
		}

		// push the value to list head
		void push(const value_type &value)
		{
			_add_node_front(new Node(this, value));
		}

		// push the value to list head
		void push(value_type &&value)
		{
			_add_node_front(new Node(this, std::move(value)));
		}

		// construct node at list head
		template<typename... Types>
		void emplace_front(Types&&... args)
		{
			_add_node_front(new Node(this, std::forward<Types>(args)...));
		}

		// pop the node at list head
		value_type pop()
		{
			if (!_size)
				throw std::runtime_error("pop from empty list");
			value_type ret = _head->value();
			nodeptr oh = _head;
			_head = _head->_next;
			this->_delete_node(oh);
			_size--;
			return ret;
		}

		// insert a value before the node
		bool insert(nodeptr node, const value_type &value)
		{
			return _insert_node(node, this->_new_node(new Node(this, value)));
		}

		// construct a value before the node
		template<typename... Types>
		bool emplace(nodeptr node, Types... args)
		{
			return _insert_node(node, this->_new_node(new Node(this, std::forward<Types>(args)...)));
		}

		bool remove(nodeptr node)
		{
			if (!node)
				return false;
			std::pair<nodeptr, nodeptr> found = this->_find_node_and_prev(node);
			return this->_remove_node(found.first, found.second);
		}

		// remove the first node which value equals the argument
		bool remove(const value_type &value)
		{
			std::pair<nodeptr, nodeptr> found = this->_find_node_and_prev(value);
			return this->_remove_node(found.first, found.second);
		}

		size_t remove_all(const value_type &value)
		{
			size_t count = 0;
			nodeptr curr = _head, prev = nullptr;
			while (curr)
			{
				if (curr->value() == value)
				{
					nodeptr next = curr->_next;
					if (prev)
						prev->_next = next;
					else
						_head = next;
					this->_delete_node(curr);
					curr = next;
					count++;
					_size--;
				}
				else
				{
					prev = curr;
					curr = curr->_next;
				}
			}
			return count;
		}

	private:
		static void _copy_sublist(ForwardLinkedList &dst, const ForwardLinkedList &src, nodeptr begin, nodeptr end = nullptr)
		{
			if (begin)
				src._ensureInList(begin);
			if (end)
				src._ensureInList(end);
			dst.clear();
			nodeptr curr;
			nodeptr nd = begin;
			if (nd && nd != end)
			{
				curr = check_t::_new_node(new Node(*nd));
				dst._head = curr;
				curr->_setCont(&dst);
				nd = nd->_next;
				dst._size++;
			}
			while (nd != end)
			{	// if end is ahead of begin, a null pointer will be dereferenced
				curr->_next = check_t::_new_node(new Node(*nd));
				curr = curr->_next;
				curr->_setCont(&dst);
				nd = nd->_next;
				dst._size++;
			}
		}

		void _add_node_front(Node *node)
		{
			node->_next = _head;
			this->_assign_node(_head, node);
			_size++;
		}

		bool _insert_node(nodeptr tofind, nodeptr toinsert)
		{
			if (tofind)
				this->_ensureInList(tofind);
			bool ret = false;
			std::pair<nodeptr, nodeptr> found = this->_find_node_and_prev(tofind);
			if (found.first || !tofind)
			{
				ret = true;
				toinsert->_next = found.first;
				if (found.second)
					found.second->_next = toinsert;
				else
					_head = toinsert;
				_size++;
			}
			return ret;
		}

		bool _remove_node(nodeptr toremove, nodeptr prev)
		{
			if (toremove)
				this->_ensureInList(toremove);
			bool ret = false;
			if (toremove)
			{
				if (prev)
					prev->_next = toremove->_next;
				else
					_head = toremove->_next;
				this->_delete_node(toremove);
				_size--;
				ret = true;
			}
			return ret;
		}

		std::pair<nodeptr, nodeptr> _find_node_and_prev(nodeptr tofind) const
		{
			nodeptr node = _head, prev = nullptr;
			while (node)
			{
				if (node == tofind)
					break;
				prev = node;
				node = node->_next;
			}
			return std::pair<nodeptr, nodeptr>(node, prev);
		}

		std::pair<nodeptr, nodeptr> _find_node_and_prev(const value_type &tofind) const
		{
			nodeptr node = _head, prev = nullptr;
			while (node)
			{
				if (node->value() == tofind)
					break;
				prev = node;
				node = node->_next;
			}
			return std::pair<nodeptr, nodeptr>(node, prev);
		}

	};

	template<typename Valt>
	using UniqueForwardLinkedList = ForwardLinkedList<Valt, UniqueNode<Valt>>;
	template<typename Valt>
	using SharedForwardLinkedList = ForwardLinkedList<Valt, SharedNode<Valt>>;


	// binary linked list
	template<typename Valt, typename BaseNode = UniqueNode<Valt>>
	class LinkedList : public _BaseLinkedList<Valt, LinkedListNode<Valt, BaseNode>>
	{
	public:
		using value_type = Valt;
		using Node = LinkedListNode<value_type, BaseNode>;
		using _MyBase = _BaseLinkedList<value_type, LinkedListNode<value_type, BaseNode>>;
		using check_t = typename _MyBase::check_t;
		using nodeptr = typename _MyBase::nodeptr;

	public:
		LinkedList() {}

		LinkedList(std::initializer_list<value_type> list)
			: LinkedList(list.begin(), list.end())
		{
		}

		template<typename Iter>
		LinkedList(Iter begin, Iter end)
		{
			for (auto it = begin; it != end; it++)
			{
				this->push_back(*it);
			}
		}

		LinkedList(const LinkedList &rhs)
		{
			LinkedList::_copy_sublist(*this, rhs, rhs.head_node());
		}

		LinkedList(LinkedList &&rhs)
			: _MyBase(std::move(rhs)), _tail(std::move(rhs._tail))
		{
			rhs._size = 0;
			rhs._head = nullptr;
			rhs._tail = nullptr;
		}

		~LinkedList()
		{
			this->_destroy();
		}

		LinkedList &operator=(const LinkedList &rhs)
		{
			if (this != &rhs)
			{
				_copy_sublist(*this, rhs, rhs.head_node());
			}
			return *this;
		}

		LinkedList &operator=(LinkedList &&rhs)
		{
			if (this != &rhs)
			{
				this->_destroy();
				_head = rhs._head;
				_tail = rhs._tail;
				_size = rhs._size;
				rhs._head = nullptr;
				rhs._tail = nullptr;
				rhs._size = 0;
			}
			return *this;
		}

		void clear()
		{
			this->_destroy();
			_size = 0;
		}

		void swap(nodeptr ln, nodeptr rn)
		{
			if (!ln || !rn)
				return;
			if (ln == rn)
				return;
			this->_ensureInList(ln);
			this->_ensureInList(rn);
			nodeptr lprev = ln->_prev, lnext = ln->_next;
			nodeptr rprev = rn->_prev, rnext = rn->_next;
			ln->_prev = rprev;
			if (rprev) rprev->_next = ln;
			else _head = ln;
			ln->_next = rnext;
			if (rnext) rnext->_prev = ln;
			else _tail = ln;
			rn->_prev = lprev;
			if (lprev) lprev->_next = rn;
			else _head = rn;
			rn->_next = lnext;
			if (lnext) lnext->_prev = rn;
			else _tail = rn;
		}

		void reverse()
		{
			if (_size <= 1)
				return;
			nodeptr left = _head, right = _head->_next;
			_head = _tail;
			_tail = left;
			while (right)
			{
				left->_next = left->_prev;
				left->_prev = right;
				left = right;
				right = right->_next;
			}
			left->_next = left->_prev;
			left->_prev = right;
		}

		LinkedList sublist(nodeptr begin, nodeptr end = nullptr) const
		{
			LinkedList ret;
			this->_copy_sublist(ret, *this, begin, end);
			return ret;
		}

		// the tail value
		value_type tail() const
		{
			return _tail->value();
		}

		nodeptr tail_node() const
		{
			return _tail;
		}

		// push the value to list head
		void push(const value_type &value)
		{
			_add_node_front(new Node(this, value));
		}

		// push the value to list head
		void push(value_type &&value)
		{
			_add_node_front(new Node(this, std::move(value)));
		}

		// construct node at list head
		template<typename... Types>
		void emplace_front(Types&&... args)
		{
			_add_node_front(new Node(this, std::forward<Types>(args)...));
		}

		// push the value to list tail
		void push_back(const value_type &value)
		{
			_add_node_back(new Node(this, value));
		}

		// push the value to list tail
		void push_back(value_type &&value)
		{
			_add_node_back(new Node(this, std::move(value)));
		}

		// construct node at list tail
		template<typename... Types>
		void emplace_back(Types&&... args)
		{
			_add_node_back(new Node(this, std::forward<Types>(args)...));
		}

		// pop the node at list head
		value_type pop()
		{
			if (!_size)
				throw std::runtime_error("pop from empty list");
			value_type ret = _head->value();
			erase(_head);
			return ret;
		}

		// pop the node at list tail
		value_type pop_back()
		{
			if (!_size)
				throw std::runtime_error("pop from empty list");
			value_type ret = _tail->value();
			erase(_tail);
			return ret;
		}

		// insert a value before the node
		bool insert(nodeptr node, const value_type &value)
		{
			return _insert_node(node, this->_new_node(new Node(this, value)));
		}

		// construct a value before the node
		template<typename... Types>
		bool emplace(nodeptr node, Types... args)
		{
			return _insert_node(node, this->_new_node(new Node(this, std::forward<Types>(args)...)));
		}

		bool erase(nodeptr node)
		{
			return this->_remove_node(node);
		}

		// remove the first node which value equals the argument
		bool remove(const value_type &value)
		{
			return this->_remove_node(this->_find_node_by_value(value));
		}

		size_t remove_all(const value_type &value)
		{
			size_t count = 0;
			nodeptr curr = _head;
			while (curr)
			{
				auto next = curr->_next;
				if (curr->value() == value)
				{
					erase(curr);
					count++;
				}
				curr = next;
			}
			return count;
		}

	private:
		nodeptr _tail = nullptr;

		void _destroy()
		{
			_MyBase::_destroy();
			_tail = nullptr;
		}

		static void _copy_sublist(LinkedList &dst, const LinkedList &src, nodeptr begin, nodeptr end = nullptr)
		{
			if (begin)
				src._ensureInList(begin);
			if (end)
				src._ensureInList(end);
			dst.clear();
			nodeptr curr;
			nodeptr nd = begin;
			if (nd && nd != end)
			{
				curr = check_t::_new_node(new Node(*nd));
				dst._head = curr;
				curr->_setCont(&dst);
				nd = nd->_next;
				dst._size++;
			}
			while (nd != end)
			{	// if end is ahead of begin, a null pointer will be dereferenced
				curr->_next = check_t::_new_node(new Node(*nd));
				curr->_next->_prev = curr;
				curr = curr->_next;
				curr->_setCont(&dst);
				nd = nd->_next;
				dst._size++;
			}
			dst._tail = curr;
		}

		void _add_node_front(Node *node)
		{
			node->_next = _head;
			auto np = this->_new_node(node);
			if (_head)
				_head->_prev = np;
			_head = np;
			if (!_tail)
				_tail = np;
			_size++;
		}

		void _add_node_back(Node *node)
		{
			node->_prev = _tail;
			auto np = this->_new_node(node);
			if (_tail)
				_tail->_next = np;
			_tail = np;
			if (!_head)
				_head = np;
			_size++;
		}

		bool _insert_node(nodeptr tofind, nodeptr toinsert)
		{
			if (tofind)
			{
				this->_ensureInList(tofind);
				if (tofind->_prev)
					tofind->_prev->_next = toinsert;
				else
					_head = toinsert;
				toinsert->_prev = tofind->_prev;
				toinsert->_next = tofind;
				tofind->_prev = toinsert;
			}
			else
			{
				if (_tail)
					_tail->_next = toinsert;
				toinsert->_prev = _tail;
				_tail = toinsert;
				if (!_head)
					_head = toinsert;
			}
			
			return true;
		}

		bool _remove_node(nodeptr toremove)
		{
			if (toremove)
			{
				this->_ensureInList(toremove);
				if (toremove->_prev)
					toremove->_prev->_next = toremove->_next;
				else
					_head = toremove->_next;
				if (toremove->_next)
					toremove->_next->_prev = toremove->_prev;
				else
					_tail = toremove->_prev;
				this->_delete_node(toremove);
				_size--;
				return true;
			}
			else
				return false;
		}

	};

	template<typename Valt>
	using UniqueLinkedList = LinkedList<Valt, UniqueNode<Valt>>;
	template<typename Valt>
	using SharedLinkedList = LinkedList<Valt, SharedNode<Valt>>;



	template<typename KeyT, typename ValT>
	class HashTable
	{
	public:
		using key_type = KeyT;
		using value_type = ValT;
		using entry_type = std::pair<key_type, value_type>;

	public:
		HashTable();

		~HashTable();

		value_type &operator[](const key_type &key);

		size_t size() const;
		size_t max_size() const;
		bool empty() const
		{
			return size() == 0;
		}
		void clear();

		void insert(const key_type &key, const value_type &value);

		template<typename... Types>
		void emplace(const key_type &key, Types&&... args);

		void remove(const key_type &key);
		key_type search(const value_type &value);
		
	private:

	};
}

