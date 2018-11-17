#pragma once
#include "utils.h"



namespace lyf
{

	template<typename Valt, typename Node>
	class _BaseBinarySearchTree;

	template<typename Valt>
	class BinarySearchTree;

	template<typename Valt, typename Base, typename nodeptr>
	class _BaseBSTNode : public Base
	{
	public:
		using _MyBase = Base;

		_BaseBSTNode &operator=(const _BaseBSTNode &) = delete;

		const Valt &value()
		{
			return _MyBase::value();
		}

		nodeptr parent() const
		{
			this->_ensureInCont();
			return _parent;
		}

		nodeptr left() const
		{
			this->_ensureInCont();
			return _left;
		}

		nodeptr right() const
		{
			this->_ensureInCont();
			return _right;
		}

	protected:
		nodeptr _parent = nullptr;
		nodeptr _left = nullptr;
		nodeptr _right = nullptr;

		void _reset_neighbors()
		{
			_parent = _left = _right = nullptr;
		}

		void _set_neighbors(nodeptr np)
		{
			_parent = _left = _right = np;
		}

		_BaseBSTNode()
		{
		}

		_BaseBSTNode(nodeptr neighbor)
			: _parent(neighbor), _left(neighbor), _right(neighbor)
		{
		}

		_BaseBSTNode(nodeptr neighbor, void *pCont, const Valt &value)
			: _MyBase(pCont, value), 
			_parent(neighbor), _left(neighbor), _right(neighbor)
		{
		}

		_BaseBSTNode(nodeptr neighbor, void *pCont, Valt &&value)
			: _MyBase(pCont, std::move(value)), 
			_parent(neighbor), _left(neighbor), _right(neighbor)
		{
		}

		template<typename... Types>
		_BaseBSTNode(nodeptr neighbor, void *pCont, Types&&... args)
			: _MyBase(pCont, std::forward<Types>(args)...), 
			_parent(neighbor), _left(neighbor), _right(neighbor)
		{
		}

		_BaseBSTNode(const _BaseBSTNode &rhs)
			: _MyBase(rhs), _parent(), _left(), _right()
		{
		}
	};

	// Node class of binary search tree
	template<typename Valt, typename Base>
	class BSTNode : public _BaseBSTNode<Valt, Base, 
		typename _CheckedNodeContainer<BSTNode<Valt, Base>>::nodeptr>
	{
		friend class _CheckedNodeContainer<BSTNode>;
		friend class _BaseBinarySearchTree<Valt, BSTNode>;
		friend class BinarySearchTree<Valt>;

	public:
		using nodeptr = typename _CheckedNodeContainer<BSTNode<Valt, Base>>::nodeptr;
		using _MyBase = _BaseBSTNode<Valt, Base, nodeptr>;

	private:
		inline static nodeptr const _pNullNode = nullptr;
		unsigned char _same_flag = 0;

		void _reverse_flag()
		{
			_same_flag = 1 - _same_flag;
		}

		using _MyBase::_MyBase;

		BSTNode()
			: _MyBase(_pNullNode)
		{
		}

		BSTNode(const BSTNode &rhs)
			: _MyBase(rhs), _same_flag(rhs._same_flag)
		{
		}
	};


	template<typename Valt, typename Node>
	class _BaseBinarySearchTree : public _CheckedNodeContainer<Node>
	{
	public:
		using value_type = Valt;
		using _MyBase = _CheckedNodeContainer<Node>;
		using check_t = _CheckedNodeContainer<Node>;
		using nodeptr = typename check_t::nodeptr;

	protected:
		_BaseBinarySearchTree()
			: _root(Node::_pNullNode), _size()
		{
		}

		_BaseBinarySearchTree(_BaseBinarySearchTree &&rhs)
			: _MyBase(std::move(rhs)), _root(std::move(rhs._root)), _size(rhs._size)
		{
			rhs._root = Node::_pNullNode;
			rhs._size = 0;
		}

		_BaseBinarySearchTree &operator=(_BaseBinarySearchTree &&rhs)
		{
			if (this != &rhs)
			{
				this->_destroy_subtree();
				_root = std::move(rhs._root);
				_size = rhs._size;
				rhs._root = Node::_pNullNode;
				rhs._size = 0;
			}
			return *this;
		}

		~_BaseBinarySearchTree()
		{
			_destroy_subtree();
		}

	public:
		_BaseBinarySearchTree(const _BaseBinarySearchTree &) = delete;
		_BaseBinarySearchTree &operator=(const _BaseBinarySearchTree &) = delete;

		size_t size() const
		{
			return _size;
		}

		bool empty() const
		{
			return size() == 0;
		}

		void clear()
		{
			_destroy_subtree();
		}

		nodeptr root() const
		{
			return this->_conv_null_np(_root);
		}

		nodeptr search(const value_type &value) const
		{
			nodeptr np = _root;
			while (np != Node::_pNullNode)
			{
				if (np->value() == value)
					break;
				else if (np->value() < value)
					np = np->_right;
				else
					np = np->_left;
			}
			return this->_conv_null_np(np);
		}

		// The node of maximum value in subtree which rooted at the given node(default root)
		nodeptr max_node(nodeptr np = nullptr) const
		{
			if (!np || np == Node::_pNullNode)
				np = _root;
			if (np != Node::_pNullNode)
			{
				this->_ensureInTree(np);
				while (np->_right != Node::_pNullNode)
					np = np->_right;
			}
			return this->_conv_null_np(np);
		}

		// The maximum value in subtree which rooted at the given node(default root)
		value_type max_value(nodeptr np = nullptr) const
		{
			return this->max_node(np)->value();
		}

		// The node of minimum value in subtree which rooted at the given node(default root)
		nodeptr min_node(nodeptr np = nullptr) const
		{
			if (!np || np == Node::_pNullNode)
				np = _root;
			if (np != Node::_pNullNode)
			{
				this->_ensureInTree(np);
				while (np->_left != Node::_pNullNode)
					np = np->_left;
			}
			return this->_conv_null_np(np);
		}

		// The minimum value in subtree which rooted at the given node(default root)
		value_type min_value(nodeptr np = nullptr) const
		{
			return this->min_node(np)->value();
		}

		// The node with the smallest key greater than the key of the given node
		nodeptr successor(nodeptr np) const
		{
			this->_ensureInTree(np);
			if (np->_right != Node::_pNullNode)
				return this->min_node(np->_right);
			nodeptr p;
			while ((p = np->_parent) != Node::_pNullNode && p->_left != np)
				np = p;
			return this->_conv_null_np(p);
		}

		// The node with the largest key less than the key of the given node
		nodeptr predecessor(nodeptr np) const
		{
			this->_ensureInTree(np);
			if (np->_left != Node::_pNullNode)
				return this->max_node(np->_left);
			nodeptr p;
			while ((p = np->_parent) != Node::_pNullNode && p->_right != np)
				np = p;
			return this->_conv_null_np(p);
		}

	protected:
		nodeptr _root = Node::_pNullNode;
		size_t _size = 0;

		void _ensureInTree(const nodeptr &np) const
		{
			np->_ensureInCont(this);
		}

		static void _set_neighbors_null(nodeptr np)
		{
			np->_set_neighbors(Node::_pNullNode);
		}

		static nodeptr _conv_null_np(nodeptr np)
		{
			return np == Node::_pNullNode ? nullptr : np;
		}

		void _destroy_subtree_recursive(nodeptr np)
		{
			nodeptr right;
			while (np != Node::_pNullNode)
			{
				_destroy_subtree_recursive(np->_left);
				right = np->_right;
				this->_delete_node(np);
				_size--;
				np = right;
			}
		}

		void _destroy_subtree(nodeptr np = nullptr)
		{
			if (!np || np == Node::_pNullNode)
				np = _root;
			if (np != Node::_pNullNode && np->_parent != Node::_pNullNode)
			{
				if (np->_parent->_left == np)
					np->_parent->_left = Node::_pNullNode;
				else
					np->_parent->_right = Node::_pNullNode;
			}
			else
				_root = Node::_pNullNode;
			_destroy_subtree_recursive(np);
		}

		static void _copy_subtree_recursive(nodeptr dst_np, nodeptr src_np, size_t &size)
		{
			while (src_np != Node::_pNullNode)
			{
				if (src_np->_left != Node::_pNullNode)
				{
					dst_np->_left = check_t::_new_node(new Node(*(src_np->_left)));
					dst_np->_left->_parent = dst_np;
					dst_np->_left->_setCont(&dst_np);
					size++;
				}
				if (src_np->_right != Node::_pNullNode)
				{
					dst_np->_right = check_t::_new_node(new Node(*(src_np->_right)));
					dst_np->_right->_parent = dst_np;
					dst_np->_right->_setCont(&dst_np);
					size++;
				}
				_copy_subtree_recursive(dst_np->_left, src_np->_left, size);
				dst_np = dst_np->_right;
				src_np = src_np->_right;
			}
		}

		static void _copy_subtree(_BaseBinarySearchTree &dst, const _BaseBinarySearchTree &src, nodeptr np)
		{
			if (!np || np == Node::_pNullNode)
				throw std::runtime_error("Subtree's root can't be null");
			src._ensureInTree(np);
			dst._destroy_subtree();
			nodeptr curr;
			if (np != Node::_pNullNode)
			{
				curr = check_t::_new_node(new Node(*np));
				dst._root = curr;
				curr->_setCont(&dst);
				dst._size++;
			}
			_copy_subtree_recursive(dst._root, np, dst._size);
		}
	};


	template<typename Valt>
	class BinarySearchTree : public _BaseBinarySearchTree<Valt, BSTNode<Valt, UniqueNode<Valt>>>
	{
	public:
		using value_type = Valt;
		using Node = BSTNode<Valt, UniqueNode<Valt>>;
		using _MyBase = _BaseBinarySearchTree<Valt, Node>;
		using check_t = typename _MyBase::check_t;
		using nodeptr = typename check_t::nodeptr;

	public:
		using _MyBase::_MyBase;

		BinarySearchTree()
			: _MyBase()
		{
		}

		BinarySearchTree(std::initializer_list<value_type> list)
			: BinarySearchTree(list.begin(), list.end())
		{
		}

		template<typename Iter>
		BinarySearchTree(Iter begin, Iter end)
		{
			for (auto it = begin; it != end; it++)
			{
				this->insert(*it);
			}
		}

		BinarySearchTree(const BinarySearchTree &rhs)
			: BinarySearchTree()
		{
			this->_copy_subtree(*this, rhs, rhs._root);
		}

		BinarySearchTree &operator=(const BinarySearchTree &rhs)
		{
			if (this != &rhs)
			{
				this->_copy_subtree(*this, rhs, rhs._root);
			}
			return *this;
		}

		// A copy of the subtree rooted at the given node
		BinarySearchTree subtree(nodeptr np)
		{
			BinarySearchTree ret;
			this->_copy_subtree(ret, *this, np);
			return ret;
		}

		void insert(const value_type &value)
		{
			this->_insert_node(new Node(Node::_pNullNode, this, value));
		}

		void insert(value_type &&value)
		{
			this->_insert_node(new Node(Node::_pNullNode, this, std::move(value)));
		}

		template<typename... Types>
		void emplace(Types&&... args)
		{
			this->_insert_node(new Node(Node::_pNullNode, this, std::forward<Types>(args)...));
		}

		void remove(nodeptr np)
		{
			this->_ensureInTree(np);
			nodeptr new_np;
			if (np->_left == Node::_pNullNode || np->_right == Node::_pNullNode)
			{
				new_np = np->_left != Node::_pNullNode ? np->_left : np->_right;
			}
			else
			{
				new_np = this->min_node(np->_right);
				if (new_np != np->_right)
				{
					new_np->_parent->_left = new_np->_right;
					if (new_np->_right != Node::_pNullNode)
						new_np->_right->_parent = new_np->_parent;
					new_np->_right = np->_right;
					np->_right->_parent = new_np;
				}
				new_np->_left = np->_left;
				np->_left->_parent = new_np;
			}
			if (np->_parent != Node::_pNullNode)
			{
				if (np->_parent->_left == np)
					np->_parent->_left = new_np;
				else
					np->_parent->_right = new_np;
			}
			else
				_root = new_np == nullptr ? Node::_pNullNode : new_np;
			if (new_np != Node::_pNullNode)
				new_np->_parent = np->_parent;

			this->_delete_node(np);
			_size--;
		}

		// Remove the value, returns whether it's in the tree or not
		bool remove(const value_type &value)
		{
			nodeptr np = this->search(value);
			if (!np || np == Node::_pNullNode)
				return false;
			this->remove(np);
			return true;
		}

	private:

		void _insert_node(Node *pNode)
		{
			nodeptr np = this->_new_node(pNode);
			const value_type &npv = np->value();
			nodeptr x = _root, y = Node::_pNullNode;
			while (x != Node::_pNullNode)
			{
				y = x;
				if (x->value() < npv)
					x = x->_right;
				else if (x->value() > npv)
					x = x->_left;
				else
				{
					x->_reverse_flag();
					x = x->_same_flag ? x->_left : x->_right;
				}
			}
			np->_parent = y;
			if (y == Node::_pNullNode)
				_root = np;
			else if (y->value() < npv)
				y->_right = np;
			else if (y->value() > npv)
				y->_left = np;
			else
			{
				if (y->_same_flag)
					y->_left = np;
				else
					y->_right = np;
			}
			_size++;
		}

	};


	template<typename Valt>
	class RedBlackTree;

	enum class RBTNodeColor { RED, BLACK };

	// Node class of red-black tree
	template<typename Valt, typename Base>
	class RBTNode : public _BaseBSTNode<Valt, Base,
		typename _CheckedNodeContainer<RBTNode<Valt, Base>>::nodeptr>
	{
		friend class _CheckedNodeContainer<RBTNode>;
		friend class _BaseBinarySearchTree<Valt, RBTNode>;
		friend class RedBlackTree<Valt>;

	public:
		using check_t = _CheckedNodeContainer<RBTNode>;
		using nodeptr = typename check_t::nodeptr;
		using _MyBase = _BaseBSTNode<Valt, Base, nodeptr>;

		RBTNodeColor color() const
		{
			this->_ensureInCont();
			return _color;
		}

	private:
		inline static nodeptr const _pNullNode = check_t::_new_node(new RBTNode(RBTNodeColor::BLACK));
		RBTNodeColor _color = RBTNodeColor::RED;

		using _MyBase::_MyBase;

		RBTNode()
			: _MyBase(_pNullNode), _color(RBTNodeColor::RED)
		{
		}

		RBTNode(RBTNodeColor color)
			: _MyBase(), _color(color)
		{
		}

		RBTNode(const RBTNode &rhs)
			: _MyBase(rhs), _color(rhs._color)
		{
			_parent = _left = _right = _pNullNode;
		}
	};

	template<typename Valt>
	class RedBlackTree : public _BaseBinarySearchTree<Valt, RBTNode<Valt, UniqueNode<Valt>>>
	{
	public:
		using value_type = Valt;
		using Node = RBTNode<Valt, UniqueNode<Valt>>;
		using _MyBase = _BaseBinarySearchTree<Valt, Node>;
		using check_t = typename _MyBase::check_t;
		using nodeptr = typename check_t::nodeptr;

	public:
		RedBlackTree()
			: _MyBase()
		{
		}

		RedBlackTree(std::initializer_list<value_type> list)
			: RedBlackTree(list.begin(), list.end())
		{
		}

		template<typename Iter>
		RedBlackTree(Iter begin, Iter end)
		{
			for (auto it = begin; it != end; it++)
			{
				this->insert(*it);
			}
		}

		RedBlackTree(const RedBlackTree &rhs)
			: RedBlackTree()
		{
			this->_copy_subtree(*this, rhs, rhs._root);
		}

		RedBlackTree &operator=(const RedBlackTree &rhs)
		{
			if (this != &rhs)
			{
				this->_copy_subtree(*this, rhs, rhs._root);
			}
			return *this;
		}

		// A copy of the subtree rooted at the given node
		RedBlackTree subtree(nodeptr np)
		{
			RedBlackTree ret;
			this->_copy_subtree(ret, *this, np);
			return ret;
		}

		void insert(const value_type &value)
		{
			this->_insert_node(new Node(Node::_pNullNode, this, value));
		}

		void insert(value_type &&value)
		{
			this->_insert_node(new Node(Node::_pNullNode, this, std::move(value)));
		}

		template<typename... Types>
		void emplace(Types&&... args)
		{
			this->_insert_node(new Node(Node::_pNullNode, this, std::forward<Types>(args)...));
		}

		void remove(nodeptr np)
		{
			this->_ensureInTree(np);
			nodeptr new_np, x;
			RBTNodeColor color = np->_color;
			if (np->_left == Node::_pNullNode || np->_right == Node::_pNullNode)
			{
				new_np = np->_left == Node::_pNullNode ? np->_right : np->_left;
				x = new_np;
			}
			else
			{
				new_np = this->min_node(np->_right);
				color = new_np->_color;
				x = new_np->_right;
				if (new_np != np->_right)
				{
					x->_parent = new_np->_parent;
					new_np->_parent->_left = x;
					new_np->_right = np->_right;
					np->_right->_parent = new_np;
				}
				else
					x->_parent = new_np;
				new_np->_left = np->_left;
				np->_left->_parent = new_np;
				new_np->_color = np->_color;
			}
			new_np->_parent = np->_parent;
			if (np->_parent == Node::_pNullNode)
				_root = new_np;
			else if (np == np->_parent->_left)
				np->_parent->_left = new_np;
			else
				np->_parent->_right = new_np;

			this->_delete_node(np);
			_size--;
			if (color == RBTNodeColor::BLACK)
				this->_remove_fixup(x);
		}

		// Remove the value, returns whether it's in the tree or not
		bool remove(const value_type &value)
		{
			nodeptr np = this->search(value);
			if (!np || np == Node::_pNullNode)
				return false;
			this->remove(np);
			return true;
		}

		// check the red-black tree property, returns black-height
		// if the property is false, returns -1
		int check_property() const
		{
			if (_root->_color != RBTNodeColor::BLACK)
				return -1;
			if (_root == Node::_pNullNode)
				return 0;
			int lh = this->_check_property_recursive(_root->_left, 0);
			int rh = this->_check_property_recursive(_root->_right, 0);
			return (lh != rh || lh == -1 || rh == -1) ? -1 : lh;
		}

	private:
		int _check_property_recursive(nodeptr np, int h) const
		{
			if (np == Node::_pNullNode)
				return h + 1;
			if (np->_color == RBTNodeColor::RED)
			{
				if (np->_left->_color == RBTNodeColor::RED || np->_right->_color == RBTNodeColor::RED)
					return -1;
			}
			else if (np->_color == RBTNodeColor::BLACK)
				h++;
			else
				return -1;
			int lh = this->_check_property_recursive(np->_left, h);
			int rh = this->_check_property_recursive(np->_right, h);
			return (lh != rh || lh == -1 || rh == -1) ? -1 : lh;
		}

		// left-rotation, preserving the binary-search-tree property
		// if the right child of the given node is null, do nothing
		void _left_rotate(nodeptr np)
		{
			this->_ensureInTree(np);
			nodeptr right = np->_right;
			if (right == Node::_pNullNode)
				return;
			np->_right = right->_left;
			if (right->_left != Node::_pNullNode)
				right->_left->_parent = np;
			if (np->_parent == Node::_pNullNode)
				_root = right;
			else if (np == np->_parent->_left)
				np->_parent->_left = right;
			else
				np->_parent->_right = right;
			right->_parent = np->_parent;
			right->_left = np;
			np->_parent = right;
		}

		// right-rotation, preserving the binary-search-tree property
		// if the left child of the given node is null, do nothing
		void _right_rotate(nodeptr np)
		{
			this->_ensureInTree(np);
			nodeptr left = np->_left;
			if (left == Node::_pNullNode)
				return;
			np->_left = left->_right;
			if (left->_right != Node::_pNullNode)
				left->_right->_parent = np;
			if (np->_parent == Node::_pNullNode)
				_root = left;
			else if (np == np->_parent->_right)
				np->_parent->_right = left;
			else
				np->_parent->_left = left;
			left->_parent = np->_parent;
			left->_right = np;
			np->_parent = left;
		}

		void _insert_node(Node *pNode)
		{
			nodeptr np = check_t::_new_node(pNode);
			const value_type &npv = np->value();
			nodeptr curr = _root, p = Node::_pNullNode;
			while (curr != Node::_pNullNode)
			{
				p = curr;
				if (curr->value() < npv)
					curr = curr->_right;
				else
					curr = curr->_left;
			}
			if (p == Node::_pNullNode)
				_root = np;
			else
			{
				if (p->value() < npv)
					p->_right = np;
				else
					p->_left = np;
				np->_parent = p;
			}
			_size++;
			_insert_node_fixup(np);
		}

		void _insert_node_fixup(nodeptr np)
		{
			nodeptr y;
			while (np->_parent->_color == RBTNodeColor::RED)
			{
				if (np->_parent == np->_parent->_parent->_left)
				{
					y = np->_parent->_parent->_right;
					if (y->_color == RBTNodeColor::RED)
					{
						np->_parent->_color = RBTNodeColor::BLACK;
						y->_color = RBTNodeColor::BLACK;
						np->_parent->_parent->_color = RBTNodeColor::RED;
						np = np->_parent->_parent;
					}
					else
					{
						if (np == np->_parent->_right)
						{
							np = np->_parent;
							_left_rotate(np);
						}
						np->_parent->_color = RBTNodeColor::BLACK;
						np->_parent->_parent->_color = RBTNodeColor::RED;
						_right_rotate(np->_parent->_parent);
					}
				}
				else
				{
					y = np->_parent->_parent->_left;
					if (y->_color == RBTNodeColor::RED)
					{
						np->_parent->_color = RBTNodeColor::BLACK;
						y->_color = RBTNodeColor::BLACK;
						np->_parent->_parent->_color = RBTNodeColor::RED;
						np = np->_parent->_parent;
					}
					else
					{
						if (np == np->_parent->_left)
						{
							np = np->_parent;
							_right_rotate(np);
						}
						np->_parent->_color = RBTNodeColor::BLACK;
						np->_parent->_parent->_color = RBTNodeColor::RED;
						_left_rotate(np->_parent->_parent);
					}
				}
			}
			_root->_color = RBTNodeColor::BLACK;
		}

		void _remove_fixup(nodeptr np)
		{
			nodeptr w;
			while (np != _root && np->_color == RBTNodeColor::BLACK)
			{
				if (np == np->_parent->_left)
				{
					w = np->_parent->_right;
					if (w->_color == RBTNodeColor::RED)
					{
						w->_color = RBTNodeColor::BLACK;
						np->_parent->_color = RBTNodeColor::RED;
						_left_rotate(np->_parent);
						w = np->_parent->_right;
					}
					if (w->_left->_color == RBTNodeColor::BLACK && w->_right->_color == RBTNodeColor::BLACK)
					{
						w->_color = RBTNodeColor::RED;
						np = np->_parent;
					}
					else
					{
						if (w->_right->_color == RBTNodeColor::BLACK)
						{
							w->_color = RBTNodeColor::RED;
							w->_left->_color = RBTNodeColor::BLACK;
							_right_rotate(w);
							w = w->_parent;
						}
						w->_color = np->_parent->_color;
						np->_parent->_color = RBTNodeColor::BLACK;
						w->_right->_color = RBTNodeColor::BLACK;
						_left_rotate(np->_parent);
						np = _root;
					}
				}
				else
				{
					w = np->_parent->_left;
					if (w->_color == RBTNodeColor::RED)
					{
						w->_color = RBTNodeColor::BLACK;
						np->_parent->_color = RBTNodeColor::RED;
						_right_rotate(np->_parent);
						w = np->_parent->_left;
					}
					if (w->_left->_color == RBTNodeColor::BLACK && w->_right->_color == RBTNodeColor::BLACK)
					{
						w->_color = RBTNodeColor::RED;
						np = np->_parent;
					}
					else
					{
						if (w->_left->_color == RBTNodeColor::BLACK)
						{
							w->_color = RBTNodeColor::RED;
							w->_right->_color = RBTNodeColor::BLACK;
							_left_rotate(w);
							w = w->_parent;
						}
						w->_color = np->_parent->_color;
						np->_parent->_color = RBTNodeColor::BLACK;
						w->_left->_color = RBTNodeColor::BLACK;
						_right_rotate(np->_parent);
						np = _root;
					}
				}
			}
			np->_color = RBTNodeColor::BLACK;
		}

		static void _copy_subtree(RedBlackTree &dst, const RedBlackTree &src, nodeptr np)
		{
			_MyBase::_copy_subtree(dst, src, np);
			dst._root->_color = RBTNodeColor::BLACK;
		}

	};

}
