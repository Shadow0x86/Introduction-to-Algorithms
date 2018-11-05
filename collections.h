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
	protected:
		class Node
		{
			template<typename T>
			friend class ForwardLinkedList;
		public:
			Node *next() const
			{
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

			Node *_next = nullptr;
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
			if (size() <= 1)
				return;
			//TODO
		}
		void clear()
		{
			while (_head)
			{
				Node *next = _head->_next;
				delete _head;
				_head = next;
			}
			_size = 0;
		}
		T head() const
		{
			return _head->value;
		}
		Node *head_node() const
		{
			return _head;
		}

		void push(const T &value)
		{
			_add_node(new Node(value));
		}
		void push(T &&value)
		{
			_add_node(new Node(std::move(value)));
		}

		template<typename... Types>
		void emplace(Types&&... args)
		{
			_add_node(new Node(std::forward<Types>(args)...));
		}

		T pop()
		{
			T ret = _head->value;
			Node *oh = _head;
			_head = _head->_next;
			delete oh;
			_size--;
			return ret;
		}

		Node *search(const T &value)
		{
			Node *ret = nullptr;
			Node *node = _head;
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
		//void insert(Node *node, const T &value);
		//void remove(Node *node);
		//void remove(const T &value);
		

	protected:
		size_t _size = 0;
		Node *_head = nullptr;

		void _add_node(Node *node)
		{
			node->_next = _head;
			_head = node;
			_size++;
		}
	};
}
