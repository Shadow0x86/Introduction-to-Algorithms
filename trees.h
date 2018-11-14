#pragma once
#include "utils.h"



namespace lyf
{

	template<typename Valt, typename BaseNode>
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

		void reset_neighbors()
		{
			_parent = _left = _right = nullptr;
		}

		using _MyBase::_MyBase;

		_BaseBSTNode() {}

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
		friend class BinarySearchTree<Valt, Base>;

	public:
		using nodeptr = typename _CheckedNodeContainer<BSTNode>::nodeptr;
		using _MyBase = _BaseBSTNode<Valt, Base, nodeptr>;

	private:
		unsigned char _same_flag = 0;

		void reverse_flag()
		{
			_same_flag = 1 - _same_flag;
		}

		using _MyBase::_MyBase;

		BSTNode() {}

		BSTNode(const BSTNode &rhs)
			: _MyBase(rhs), _same_flag()
		{
		}
	};


	template<typename Valt, typename BaseNode = UniqueNode<Valt>>
	class BinarySearchTree : public _CheckedNodeContainer<BSTNode<Valt, BaseNode>>
	{
	public:
		using value_type = Valt;
		using Node = BSTNode<Valt, BaseNode>;
		using _MyBase = _CheckedNodeContainer<Node>;
		using check_t = _CheckedNodeContainer<Node>;
		using nodeptr = typename check_t::nodeptr;

	public:
		BinarySearchTree()
			: _MyBase(), _root(), _size()
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

		BinarySearchTree(BinarySearchTree &&rhs)
			: _MyBase(std::move(rhs)), _root(std::move(rhs._root)), _size(rhs._size)
		{
			rhs._root = nullptr;
			rhs._size = 0;
		}

		BinarySearchTree &operator=(const BinarySearchTree &rhs)
		{
			_MyBase::operator=(rhs);
			if (this != &rhs)
			{
				this->clear();
				this->_copy_subtree(*this, rhs, rhs._root);
			}
			return *this;
		}

		BinarySearchTree &operator=(BinarySearchTree &&rhs)
		{
			_MyBase::operator=(std::move(rhs));
			if (this != &rhs)
			{
				this->_destroy_subtree();
				_root = std::move(rhs._root);
				_size = rhs._size;
				rhs._root = nullptr;
				rhs._size = 0;
			}
			return *this;
		}

		~BinarySearchTree()
		{
			this->_destroy_subtree();
		}

		// A copy of the subtree rooted at the given node
		BinarySearchTree subtree(nodeptr np)
		{
			BinarySearchTree ret;
			this->_copy_subtree(ret, *this, np);
			return ret;
		}

		size_t size() const
		{
			return _size;
		}

		bool empty() const
		{
			return size() == 0;
		}

		void insert(const value_type &value)
		{
			this->_insert_node(new Node(this, value));
		}

		void insert(value_type &&value)
		{
			this->_insert_node(new Node(this, std::move(value)));
		}

		template<typename... Types>
		void emplace(Types&&... args)
		{
			this->_insert_node(new Node(this, std::forward<Types>(args)...));
		}

		nodeptr root() const
		{
			return _root;
		}

		nodeptr search(const value_type &value) const
		{
			nodeptr np = _root;
			while (np)
			{
				if (np->value() == value)
					break;
				else if (np->value() < value)
					np = np->_right;
				else
					np = np->_left;
			}
			return np;
		}

		// The node of maximum value in subtree which rooted at the given node(default root)
		nodeptr max_node(nodeptr np = nullptr) const
		{
			if (!np)
				np = _root;
			if (np)
			{
				this->_ensureInTree(np);
				while (np->_right)
					np = np->_right;
			}
			return np;
		}

		// The maximum value in subtree which rooted at the given node(default root)
		value_type max_value(nodeptr np = nullptr) const
		{
			return this->max_node(np)->value();
		}

		// The node of minimum value in subtree which rooted at the given node(default root)
		nodeptr min_node(nodeptr np = nullptr) const
		{
			if (!np)
				np = _root;
			if (np)
			{
				this->_ensureInTree(np);
				while (np->_left)
					np = np->_left;
			}
			return np;
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
			if (np->_right)
				return this->min_node(np->_right);
			nodeptr p;
			while ((p = np->_parent) && p->_left != np)
				np = p;
			return p;
		}

		// The node with the largest key less than the key of the given node
		nodeptr predecessor(nodeptr np) const
		{
			this->_ensureInTree(np);
			if (np->_left)
				return this->max_node(np->_left);
			nodeptr p;
			while ((p = np->_parent) && p->_right != np)
				np = p;
			return p;
		}

		void remove(nodeptr np)
		{
			this->_ensureInTree(np);
			nodeptr new_np;
			if (!(np->_left && np->_right))
			{
				new_np = np->_left ? np->_left : np->_right;
			}
			else
			{
				new_np = this->min_node(np->_right);
				if (new_np != np->_right)
				{
					new_np->_parent->_left = new_np->_right;
					new_np->_right->_parent = new_np->_parent;
					new_np->_right = np->_right;
					np->_right->_parent = new_np;
				}
				new_np->_left = np->_left;
				np->_left->_parent = new_np;
			}
			if (np->_parent)
			{
				if (np->_parent->_left == np)
					np->_parent->_left = new_np;
				else
					np->_parent->_right = new_np;
			}
			else
				_root = new_np;
			if (new_np)
				new_np->_parent = np->_parent;

			this->_delete_node(np);
			_size--;
		}

		// Remove the value, returns whether it's in the tree or not
		bool remove(const value_type &value)
		{
			nodeptr np = this->search(value);
			if (!np)
				return false;
			this->remove(np);
			return true;
		}

		void clear()
		{
			_destroy_subtree();
		}

		// left-rotation, preserving the binary-search-tree property
		// if the right child of the given node is null, do nothing
		void left_rotate(nodeptr np)
		{
			this->_ensureInTree(np);
			nodeptr right = np->_right;
			if (!right)
				return;
			np->_right = right->_left;
			if (right->_left)
				right->_left->_parent = np;
			if (!(np->_parent))
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
		void right_rotate(nodeptr np)
		{
			this->_ensureInTree(np);
			nodeptr left = np->_left;
			if (!left)
				return;
			np->_left = left->_right;
			if (left->_right)
				left->_right->_parent = np;
			if (!(np->_parent))
				_root = left;
			else if (np == np->_parent->_right)
				np->_parent->_right = left;
			else
				np->_parent->_left = left;
			left->_parent = np->_parent;
			left->_right = np;
			np->_parent = left;
		}

	private:
		nodeptr _root = nullptr;
		size_t _size = 0;

		void _ensureInTree(const nodeptr &np) const
		{
			np->_ensureInCont(this);
		}

		void _insert_node(Node *pNode)
		{
			nodeptr np = this->_new_node(pNode);
			const value_type &npv = np->value();
			nodeptr x = _root, y = nullptr;
			while (x)
			{
				y = x;
				if (x->value() < npv)
					x = x->_right;
				else if (x->value() > npv)
					x = x->_left;
				else
				{
					x->reverse_flag();
					x = x->_same_flag ? x->_left : x->_right;
				}
			}
			np->_parent = y;
			if (!y)
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

		static void _copy_subtree_recursive(nodeptr dst_np, nodeptr src_np, size_t &size)
		{
			while (src_np)
			{
				if (src_np->_left)
				{
					dst_np->_left = check_t::_new_node(new Node(*(src_np->_left)));
					dst_np->_left->_parent = dst_np;
					dst_np->_left->_setCont(&dst_np);
					size++;
				}
				if (src_np->_right)
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

		static void _copy_subtree(BinarySearchTree &dst, const BinarySearchTree &src, nodeptr np)
		{
			if (!np)
				throw std::runtime_error("Subtree's root can't be null");
			src._ensureInTree(np);
			dst._destroy_subtree();
			nodeptr curr;
			if (np)
			{
				curr = check_t::_new_node(new Node(*np));
				dst._root = curr;
				curr->_setCont(&dst);
				dst._size++;
			}
			_copy_subtree_recursive(dst._root, np, dst._size);
		}

		void _destroy_subtree_recursive(nodeptr np)
		{
			nodeptr right;
			while (np)
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
			if (!np)
				np = _root;
			if (np && np->_parent)
			{
				if (np->_parent->_left == np)
					np->_parent->_left = nullptr;
				else
					np->_parent->_right = nullptr;
			}
			else
				_root = nullptr;
			_destroy_subtree_recursive(np);
		}
	};


	template<typename Valt, typename BaseNode>
	class RedBlackTree;

	enum class RBTNodeColor { RED, BLACK };

	// Node class of red-black tree
	template<typename Valt, typename Base>
	class RBTNode : public _BaseBSTNode<Valt, Base,
		typename _CheckedNodeContainer<RBTNode<Valt, Base>>::nodeptr>
	{
		friend class _CheckedNodeContainer<RBTNode>;
		friend class RedBlackTree<Valt, Base>;

	public:
		using nodeptr = typename _CheckedNodeContainer<RBTNode>::nodeptr;
		using _MyBase = _BaseBSTNode<Valt, Base, nodeptr>;

		RBTNodeColor color() const
		{
			this->_ensureInCont();
			return _color;
		}

	private:
		RBTNodeColor _color = RBTNodeColor::BLACK;

		void _set_neighbors(nodeptr np)
		{
			_parent = _left = _right = np;
		}

		using _MyBase::_MyBase;

		RBTNode()
			: _MyBase(), _color(RBTNodeColor::BLACK)
		{
		}

		RBTNode(const RBTNode &rhs)
			: _MyBase(rhs), _color()
		{
		}
	};

	template<typename Valt, typename BaseNode = UniqueNode<Valt>>
	class RedBlackTree : public _CheckedNodeContainer<RBTNode<Valt, BaseNode>>
	{
	public:
		using value_type = Valt;
		using Node = RBTNode<Valt, BaseNode>;
		using _MyBase = _CheckedNodeContainer<Node>;
		using check_t = _CheckedNodeContainer<Node>;
		using nodeptr = typename check_t::nodeptr;

	public:
		RedBlackTree()
			: _MyBase(), _root(_NULL_NODE), _size()
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

		RedBlackTree(RedBlackTree &&rhs)
			: _MyBase(std::move(rhs)), _root(std::move(rhs._root)), _size(rhs._size)
		{
			rhs._root = _NULL_NODE;
			rhs._size = 0;
		}

		RedBlackTree &operator=(const RedBlackTree &rhs)
		{
			_MyBase::operator=(rhs);
			if (this != &rhs)
			{
				this->clear();
				this->_copy_subtree(*this, rhs, rhs._root);
			}
			return *this;
		}

		RedBlackTree &operator=(RedBlackTree &&rhs)
		{
			_MyBase::operator=(std::move(rhs));
			if (this != &rhs)
			{
				this->_destroy_subtree();
				_root = std::move(rhs._root);
				_size = rhs._size;
				rhs._root = _NULL_NODE;
				rhs._size = 0;
			}
			return *this;
		}

		~RedBlackTree()
		{
			this->_destroy_subtree();
		}

		// A copy of the subtree rooted at the given node
		RedBlackTree subtree(nodeptr np)
		{
			RedBlackTree ret;
			this->_copy_subtree(ret, *this, np);
			return ret;
		}

		size_t size() const
		{
			return _size;
		}

		bool empty() const
		{
			return size() == 0;
		}

		void insert(const value_type &value)
		{
			this->_insert_node(new Node(this, value));
		}

		void insert(value_type &&value)
		{
			this->_insert_node(new Node(this, std::move(value)));
		}

		template<typename... Types>
		void emplace(Types&&... args)
		{
			this->_insert_node(new Node(this, std::forward<Types>(args)...));
		}

		nodeptr root() const
		{
			return this->_conv_null_np(_root);
		}

		nodeptr search(const value_type &value) const
		{
			nodeptr np = _root;
			while (np != _NULL_NODE)
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
			if (!np || np == _NULL_NODE)
				np = _root;
			if (np != _NULL_NODE)
			{
				this->_ensureInTree(np);
				while (np->_right != _NULL_NODE)
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
			if (!np || np == _NULL_NODE)
				np = _root;
			if (np != _NULL_NODE)
			{
				this->_ensureInTree(np);
				while (np->_left != _NULL_NODE)
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
			if (np->_right != _NULL_NODE)
				return this->min_node(np->_right);
			nodeptr p;
			while ((p = np->_parent) != _NULL_NODE && p->_left != np)
				np = p;
			return this->_conv_null_np(p);
		}

		// The node with the largest key less than the key of the given node
		nodeptr predecessor(nodeptr np) const
		{
			this->_ensureInTree(np);
			if (np->_left != _NULL_NODE)
				return this->max_node(np->_left);
			nodeptr p;
			while ((p = np->_parent) != _NULL_NODE && p->_right != np)
				np = p;
			return this->_conv_null_np(p);
		}

		void remove(nodeptr np);

		// Remove the value, returns whether it's in the tree or not
		bool remove(const value_type &value)
		{
			nodeptr np = this->search(value);
			if (!np || np == _NULL_NODE)
				return false;
			this->remove(np);
			return true;
		}

		void clear()
		{
			_destroy_subtree();
		}

		// left-rotation, preserving the binary-search-tree property
		// if the right child of the given node is null, do nothing
		void left_rotate(nodeptr np)
		{
			this->_ensureInTree(np);
			nodeptr right = np->_right;
			if (right == _NULL_NODE)
				return;
			np->_right = right->_left;
			if (right->_left != _NULL_NODE)
				right->_left->_parent = np;
			if (np->_parent == _NULL_NODE)
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
		void right_rotate(nodeptr np)
		{
			this->_ensureInTree(np);
			nodeptr left = np->_left;
			if (left == _NULL_NODE)
				return;
			np->_left = left->_right;
			if (left->_right != _NULL_NODE)
				left->_right->_parent = np;
			if (np->_parent == _NULL_NODE)
				_root = left;
			else if (np == np->_parent->_right)
				np->_parent->_right = left;
			else
				np->_parent->_left = left;
			left->_parent = np->_parent;
			left->_right = np;
			np->_parent = left;
		}

	private:
		nodeptr _root = _NULL_NODE;
		size_t _size = 0;

		static nodeptr const _NULL_NODE;

		static void _set_null_neighbors(nodeptr np)
		{
			np->_set_neighbors(_NULL_NODE);
		}

		static nodeptr _conv_null_np(nodeptr np)
		{
			return np == _NULL_NODE ? nullptr : np;
		}

		void _ensureInTree(const nodeptr &np) const
		{
			np->_ensureInCont(this);
		}

		void _insert_node(Node *pNode);
	};

	template<typename Valt, typename BaseNode>
	typename RedBlackTree<Valt, BaseNode>::nodeptr const 
		RedBlackTree<Valt, BaseNode>::_NULL_NODE = typename RedBlackTree<Valt, BaseNode>::check_t::_new_node(
			new RedBlackTree<Valt, BaseNode>::Node);
}
