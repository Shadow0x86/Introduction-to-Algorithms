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


	template<typename Node>
	class _CheckedLinkedList;

	template<typename Valt, typename Base>
	class ForwardLinkedListNode : public Base
	{
		friend class _CheckedLinkedList<ForwardLinkedListNode>;
	public:
		using nodeptr = typename _CheckedLinkedList<ForwardLinkedListNode>::nodeptr;

		nodeptr next() const
		{
			this->_ensureInCont();
			return _next;
		}

	private:
		using Base::Base;
		nodeptr _next = nullptr;
	};

	template<typename Valt, typename Base>
	class LinkedListNode : public Base
	{
		friend class _CheckedLinkedList<LinkedListNode>;
	public:
		using nodeptr = typename _CheckedLinkedList<LinkedListNode>::nodeptr;
		
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
		using Base::Base;
		nodeptr _prev = nullptr;
		nodeptr _next = nullptr;
	};

	template<typename Node>
	class _CheckedLinkedList
	{
	public:
#if DEBUG
		using nodeptr = std::shared_ptr<Node>;
#else
		using nodeptr = Node * ;
#endif

	protected:
		static nodeptr _new_node(Node *pNode)
		{
#if DEBUG
			return nodeptr(pNode);
#else
			return pNode;
#endif
		}

		static void _assign_node(nodeptr &ln, Node *pNode)
		{
#if DEBUG
			ln.reset(pNode);
#else
			ln = pNode;
#endif
		}

		static void _delete_node(nodeptr &node)
		{
#if DEBUG
			node.reset();
#else
			delete node;
#endif
		}

		static void _set_out(nodeptr &node)
		{
			node->_setCont();
		}
	};

	template<typename Valt, typename Node>
	class _BaseLinkedList : public _AbsColl<Valt>, public _CheckedLinkedList<Node>
	{
	public:
		using check_t = _CheckedLinkedList<Node>;
		using nodeptr = typename check_t::nodeptr;

		~_BaseLinkedList()
		{
			_destroy();
		}

		void clear()
		{
			_destroy();
			_size = 0;
		}
		Valt head() const
		{
			return *(_head->_pVal);
		}
		nodeptr head_node() const
		{
			return _head;
		}

		nodeptr search(const Valt &value) const
		{
			return this->_find_node_by_value(value);
		}

	protected:
		nodeptr _head = nullptr;

		void _ensureInList(const nodeptr &node) const
		{
			node->_ensureInCont(this);
		}

		nodeptr _find_node_by_value(const Valt &tofind) const
		{
			nodeptr node = _head;
			while (node)
			{
				if (*(node->_pVal) == tofind)
					break;
				node = node->_next;
			}
			return node;
		}

		void _destroy()
		{
			nodeptr np = _head;
			_head = nullptr;
			while (np)
			{
				nodeptr next = np->_next;
				this->_set_out(np);
				this->_delete_node(np);
				np = next;
			}
		}
	};


	template<typename Valt, typename BaseNode = UniqueNode<Valt>>
	class ForwardLinkedList : public _BaseLinkedList<Valt, ForwardLinkedListNode<Valt, BaseNode>>
	{
	public:
		using Node = ForwardLinkedListNode<Valt, BaseNode>;
		using _MyBase = _BaseLinkedList<Valt, ForwardLinkedListNode<Valt, BaseNode>>;
		using check_t = typename _MyBase::check_t;
		using nodeptr = typename _MyBase::nodeptr;

	public:
		ForwardLinkedList() {}

		ForwardLinkedList(const ForwardLinkedList &rhs)
			: _size(rhs._size), _head(ForwardLinkedList::_copy(rhs))
		{
		}

		ForwardLinkedList(ForwardLinkedList &&rhs)
			: _size(rhs._size), _head(rhs._head)
		{
			rhs._size = 0;
			rhs._head = nullptr;
		}

		ForwardLinkedList &operator=(const ForwardLinkedList &rhs)
		{
			if (this != &rhs)
			{
				_destroy();
				_size = rhs._size;
				_head = this->_copy(rhs);
			}
			return *this;
		}

		ForwardLinkedList &operator=(ForwardLinkedList &&rhs)
		{
			if (this != &rhs)
			{
				_destroy();
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

		void push(const Valt &value)
		{	// push the value to list head
			_add_node_front(new Node(this, value));
		}
		void push(Valt &&value)
		{	// push the value to list head
			_add_node_front(new Node(this, std::move(value)));
		}

		template<typename... Types>
		void emplace_front(Types&&... args)
		{	// construct node at list head
			_add_node_front(new Node(this, std::forward<Types>(args)...));
		}

		Valt pop()
		{	// pop the value at list head
			Valt ret = *(_head->_pVal);
			nodeptr oh = _head;
			_head = _head->_next;
			this->_delete_node(oh);
			_size--;
			return ret;
		}

		bool insert(nodeptr node, const Valt &value)
		{	// insert a value before the node
			return _insert_node(node, this->_new_node(new Node(this, value)));
		}

		template<typename... Types>
		bool emplace(nodeptr node, Types... args)
		{	// construct a value before the node
			return _insert_node(node, this->_new_node(new Node(this, std::forward<Types>(args)...)));
		}

		bool remove(nodeptr node)
		{
			this->_ensureInList(node);
			std::pair<nodeptr, nodeptr> found = this->_find_node_and_prev(node);
			return this->_remove_node(found.first, found.second);
		}
		bool remove(const Valt &value)
		{	// remove the first node which value equals the argument
			std::pair<nodeptr, nodeptr> found = this->_find_node_and_prev(value);
			return this->_remove_node(found.first, found.second);
		}
		size_t remove_all(const Valt &value)
		{
			size_t count = 0;
			nodeptr curr = _head, prev = nullptr;
			while (curr)
			{
				if (*(curr->_pVal) == value)
				{
					this->_set_out(curr);
					nodeptr next = curr->_next;
					if (prev)
						prev->_next = next;
					else
						_head = next;
					curr->_next = nullptr;
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
		static nodeptr _copy(const ForwardLinkedList &rhs)
		{
			nodeptr new_head = nullptr, curr;
			nodeptr nd = rhs.head_node();
			if (nd)
			{
				curr = check_t::new_node(new Node(this, *(nd->_pVal)));
				nd = nd->_next;
			}
			while (nd)
			{
				curr->_next = check_t::new_node(new Node(this, *(nd->_pVal)));
				curr = curr->_next;
				nd = nd->_next;
			}
			return new_head;
		}

		void _add_node_front(Node *node)
		{
			node->_next = _head;
			_head = node;
			_size++;
		}

		bool _insert_node(const nodeptr &tofind, nodeptr &toinsert)
		{
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

		bool _remove_node(nodeptr &toremove, nodeptr &prev)
		{
			bool ret = false;
			if (toremove)
			{
				if (prev)
					prev->_next = toremove->_next;
				else
					_head = toremove->_next;
				this->_set_out(toremove);
				toremove->_next = nullptr;
				_size--;
				ret = true;
			}
			return ret;
		}

		std::pair<nodeptr, nodeptr> _find_node_and_prev(const nodeptr &tofind) const
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

		std::pair<nodeptr, nodeptr> _find_node_and_prev(const Valt &tofind) const
		{
			nodeptr node = _head, prev = nullptr;
			while (node)
			{
				if (*(node->_pVal) == tofind)
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












	

//	template<typename T>
//	class LinkedList :public ForwardLinkedList<T>
//	{
//	public:
//		using Node = LinkedListNode<T>;
//#if DEBUG
//		using nodeptr = std::shared_ptr<Node>;
//#else
//		using nodeptr = Node * ;
//#endif
//
//	};

}

