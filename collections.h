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

		virtual size_t size() const = 0;
		bool empty()
		{
			return size() == 0;
		}
		virtual void clear() = 0;

		virtual void push(const T &value) = 0;
		virtual void push(T &&value) = 0;
		virtual T pop() = 0;
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


	template<typename T>
	class ForwardLinkedList : public _AbsColl<T>
	{
	public:
		class Node;
#if DEBUG
		using nodeptr = std::shared_ptr<Node>;
#else
		using nodeptr = Node * ;
#endif
		
		class Node
		{
			template<typename T>
			friend class ForwardLinkedList;
		public:
			nodeptr next() const
			{
				_ensureValid();
				return _next;
			}
			T &value()
			{
				_ensureValid();
				return _value;
			}
			
		private:
			Node(const T &value)
				: _value(value)
			{
			}
			Node(T &&value)
				: _value(std::move(value))
			{
			}

			template<typename... Types>
			Node(Types&&... args)
				: _value(std::forward<Types>(args)...)
			{
			}

			T _value;
			nodeptr _next = nullptr;
#if DEBUG
			bool _in_list = true;
#endif
			void _ensureValid() const
			{
#if DEBUG
				if (!_in_list)
					throw std::runtime_error("The node is not in list!");
#endif
			}
		};

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
		~ForwardLinkedList()
		{
			_destroy();
		}

		ForwardLinkedList &operator=(const ForwardLinkedList &rhs)
		{
			if (this != &rhs)
			{
				_destroy();
				_size = rhs._size;
				_head = ForwardLinkedList::_copy(rhs);
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

		size_t size() const
		{
			return _size;
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
		void clear()
		{
			_destroy();
			_size = 0;
		}
		T head() const
		{
			return _head->_value;
		}
		nodeptr head_node() const
		{
			return _head;
		}

		void push(const T &value)
		{	// push the value to list head
			_add_node_front(new Node(value));
		}
		void push(T &&value)
		{	// push the value to list head
			_add_node_front(new Node(std::move(value)));
		}

		template<typename... Types>
		void emplace_front(Types&&... args)
		{	// construct node at list head
			_add_node_front(new Node(std::forward<Types>(args)...));
		}

		T pop()
		{	// pop the value at list head
			T ret = _head->_value;
			nodeptr oh = _head;
			_head = _head->_next;
#if DEBUG
			oh.reset();
#else
			delete oh;
#endif
			_size--;
			return ret;
		}

		nodeptr search(const T &value) const
		{
			auto find = this->_find_node_and_prev(value);
			return find.first;
		}
		bool insert(nodeptr node, const T &value)
		{	// insert a value before the node
#if DEBUG
			nodeptr new_node(new Node(value));
#else
			nodeptr new_node = new Node(value);
#endif
			return _insert_node(node, new_node);
		}
		
		template<typename... Types>
		bool emplace(nodeptr node, Types... args)
		{	// construct a value before the node
#if DEBUG
			nodeptr new_node(new Node(std::forward<Types>(args)...));
#else
			nodeptr new_node = new Node(std::forward<Types>(args)...);
#endif
			return _insert_node(node, new_node);
		}

		bool _insert_node(nodeptr tofind, nodeptr toinsert)
		{
			bool ret = false;
			std::pair<nodeptr, nodeptr> find = this->_find_node_and_prev(tofind);
			if (find.first || !tofind)
			{
				ret = true;
				toinsert->_next = find.first;
				if (find.second)
					find.second->_next = toinsert;
				else
					_head = toinsert;
				_size++;
			}
			return ret;
		}
		bool _remove_node(nodeptr toremove, nodeptr prev)
		{
			bool ret = false;
			if (toremove)
			{
				if (prev)
					prev->_next = toremove->_next;
				else
					_head = toremove->_next;
#if DEBUG
				toremove->_in_list = false;
#endif
				toremove->_next = nullptr;
				_size--;
				ret = true;
			}
			return ret;
		}
		bool remove(nodeptr node)
		{
			std::pair<nodeptr, nodeptr> find = this->_find_node_and_prev(node);
			return this->_remove_node(find.first, find.second);
		}
		bool remove(const T &value)
		{	// remove the first node which value equals the argument
			std::pair<nodeptr, nodeptr> find = this->_find_node_and_prev(value);
			return this->_remove_node(find.first, find.second);
		}
		size_t remove_all(const T &value)
		{
			size_t count = 0;
			nodeptr curr = _head, prev = nullptr;
			while (curr)
			{
				if (curr->_value == value)
				{
#if DEBUG
					curr->_in_list = false;
#endif
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
		
	protected:
		size_t _size = 0;
		nodeptr _head = nullptr;

		void _add_node_front(Node *node)
		{
			node->_next = _head;
#if DEBUG
			_head.reset(node);
#else
			_head = node;
#endif
			_size++;
		}

		static nodeptr _copy(const ForwardLinkedList &rhs)
		{
			nodeptr new_head, curr;
			nodeptr nd = rhs.head_node();
			if (nd)
			{
#if DEBUG
				new_head.reset(new Node(nd->_value));
#else
				new_head = new Node(nd->_value);
#endif
				curr = new_head;
				nd = nd->_next;
			}
			while (nd)
			{
#if DEBUG
				curr->_next.reset(new Node(nd->_value));
#else
				curr->_next = new Node(nd->_value);
#endif
				curr = curr->_next;
				nd = nd->_next;
			}
			return new_head;
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

		std::pair<nodeptr, nodeptr> _find_node_and_prev(const T &tofind) const
		{
			nodeptr node = _head, prev = nullptr;
			while (node)
			{
				if (node->_value == tofind)
					break;
				prev = node;
				node = node->_next;
			}
			return std::pair<nodeptr, nodeptr>(node, prev);
		}

		void _destroy()
		{
#if DEBUG
			nodeptr np = _head;
			_head.reset();
			while (np)
			{
				if (np.use_count() > 1)
					np->_in_list = false;
				np = np->_next;
			}
#else
			while (_head)
			{
				nodeptr next = _head->_next;
				delete _head;
				_head = next;
			}
#endif
		}
	};

}

