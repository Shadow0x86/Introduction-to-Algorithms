#pragma once
#include <vector>
#include <deque>


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
#if DEBUG
				if (!_in_list)
					throw std::runtime_error("The node is not in list!");
#endif
				return _next;
			}

			T value;

		private:
			Node(const T &value)
				: value(value)
			{
			}
			Node(T &&value)
				: value(std::move(value))
			{
			}

			template<typename... Types>
			Node(Types&&... args)
				: value(std::forward<Types>(args)...)
			{
			}

			nodeptr _next = nullptr;
#if DEBUG
			bool _in_list = true;
#endif
		};

	public:
		ForwardLinkedList() {}
		//ForwardLinkedList(const ForwardLinkedList &rhs);
		ForwardLinkedList(ForwardLinkedList &&rhs)
			: _size(rhs._size), _head(rhs._head)
		{
			rhs._size = 0;
			rhs._head = nullptr;
		}

		//ForwardLinkedList &operator=(const ForwardLinkedList &rhs);
		ForwardLinkedList &operator=(ForwardLinkedList &&rhs)
		{
			if (this != &rhs)
			{
				clear();
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
			//TODO
		}
		void clear()
		{
#if DEBUG
			nodeptr curr = _head, np = _head;
			_head.reset();
			while (np)
			{
				np = curr->_next;
				if (curr.use_count() > 1)
				{
					curr->_in_list = false;
					curr->_next.reset();
				}
				curr = np;
			}
#else
			while (_head)
			{
				nodeptr next = _head->_next;
				delete _head;
				_head = next;
			}
#endif
			_size = 0;
		}
		T head() const
		{
			return _head->value;
		}
		nodeptr head_node() const
		{
			return _head;
		}

		void push(const T &value)
		{
			_add_node_front(new Node(value));
		}
		void push(T &&value)
		{
			_add_node_front(new Node(std::move(value)));
		}

		template<typename... Types>
		void emplace(Types&&... args)
		{
			_add_node_front(new Node(std::forward<Types>(args)...));
		}

		T pop()
		{
			T ret = _head->value;
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

		nodeptr search(const T &value)
		{
			nodeptr ret = nullptr;
			nodeptr node = _head;
			while (node)
			{
				if (node->value == value)
				{
					ret = node;
					break;
				}
				node = node->_next;
			}
			return ret;
		}
		bool insert(nodeptr node, const T &value)
		{	// insert a value before the node
			bool ret = false;
			nodeptr curr = _head, prev = nullptr;
			while (curr)
			{
				if (curr == node)
				{
#if DEBUG
					nodeptr new_node(new Node(value));
#else
					nodeptr new_node = new Node(value);
#endif
					new_node->_next = curr;
					if (prev)
						prev->_next = new_node;
					else
						_head = new_node;
					ret = true;
					break;
				}
				prev = curr;
				curr = curr->_next;
			}
			return ret;
		}
		bool remove(nodeptr node)
		{
			bool ret = false;
			nodeptr curr = _head, prev = nullptr;
			while (curr)
			{
				if (curr == node)
				{
#if DEBUG
					curr->_in_list = false;
#endif
					nodeptr next = curr->_next;
					curr = nullptr;
					if (prev)
						prev->_next = next;
					else
						_head = next;
					ret = true;
					break;
				}
				prev = curr;
				curr = curr->_next;
			}
			return ret;
		}
		bool remove(const T &value)
		{
			bool ret = false;
			nodeptr curr = _head, prev = nullptr;
			while (curr)
			{
				if (curr->value == value)
				{
#if DEBUG
					curr->_in_list = false;
#endif
					nodeptr next = curr->_next;
					curr = nullptr;
					if (prev)
						prev->_next = next;
					else
						_head = next;
					ret = true;
					break;
				}
				prev = curr;
				curr = curr->_next;
			}
			return ret;
		}
		size_t remove_all(const T &value)
		{
			size_t count = 0;
			nodeptr curr = _head, prev = nullptr;
			while (curr)
			{
				if (curr->value == value)
				{
#if DEBUG
					curr->_in_list = false;
#endif
					nodeptr next = curr->_next;
					curr = nullptr;
					if (prev)
						prev->_next = next;
					else
						_head = next;
					count++;
					curr = next;
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
	};

}

