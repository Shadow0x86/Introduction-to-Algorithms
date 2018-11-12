#pragma once
#include <memory>
#include "utils.h"



namespace lyf
{
	template<typename Node>
	class _CheckedBST;

	template<typename Valt, typename BaseNode>
	class BinarySearchTree;

	// Node class of binary search tree
	template<typename Valt, typename Base>
	class BSTNode : public Base
	{
		friend class _CheckedBST<BSTNode>;
		friend class BinarySearchTree<Valt, Base>;

	public:
		using _MyBase = Base;
		using nodeptr = typename _CheckedBST<BSTNode>::nodeptr;

		BSTNode &operator=(const BSTNode &rhs)
		{
			this->_ensureInCont();
			if (this != &rhs)
			{
				_MyBase::operator=(rhs);
			}
			return *this;
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

	private:
		nodeptr _parent = nullptr;
		nodeptr _left = nullptr;
		nodeptr _right = nullptr;
		unsigned char _same_flag = 0;

		void reverse_flag()
		{
			_same_flag = 1 - _same_flag;
		}

		void reset_neighbors()
		{
			_parent = _left = _right = nullptr;
		}

		using _MyBase::_MyBase;

		BSTNode() {}

		BSTNode(const BSTNode &rhs)
			: _MyBase(rhs), _parent(), _left(), _right(), _same_flag()
		{
		}
	};

	template<typename Node>
	class _CheckedBST
	{
	public:

#if DEBUG
		using nodeptr = std::shared_ptr<Node>;
#else
		using nodeptr = Node * ;
#endif

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

		static void _delete_node(nodeptr &np)
		{
#if DEBUG
			np->reset_neighbors();
			np->setCont();
			np.reset();
#else
			delete np;
			np = nullptr;
#endif
		}

		static void _set_out(nodeptr &np)
		{
			np->setCont();
		}
	};

	template<typename Valt, typename BaseNode>
	class BinarySearchTree : public _CheckedBST<BSTNode<Valt, BaseNode>>
	{
	public:
		using value_type = Valt;
		using Node = BSTNode<Valt, BaseNode>;
		using _MyBase = _CheckedBST<Node>;
		using check_t = _CheckedBST<Node>;
		using nodeptr = typename check_t::nodeptr;

	public:
		BinarySearchTree()
			: _MyBase(), _root(), _size()
		{
		}

		BinarySearchTree(std::initializer_list<value_type> list);

		template<typename Iter>
		BinarySearchTree(Iter begin, Iter end);

		BinarySearchTree(const BinarySearchTree &rhs);
		BinarySearchTree(BinarySearchTree &&rhs);
		BinarySearchTree &operator=(const BinarySearchTree &rhs);
		BinarySearchTree &operator=(BinarySearchTree &&rhs);

		~BinarySearchTree();

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

		nodeptr max_node(nodeptr np = nullptr) const
		{
			if (!np)
				np = _root;
			this->_ensureInTree(np);
			if (np)
				while (np->_right)
					np = np->_right;
			return np;
		}

		value_type max_value() const
		{
			return this->max_node()->value();
		}

		nodeptr min_node(nodeptr np = nullptr) const
		{
			if (!np)
				np = _root;
			this->_ensureInTree(np);
			if (np)
				while (np->_left)
					np = np->_left;
			return np;
		}

		value_type min_value() const
		{
			return this->min_node()->value();
		}

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

		void remove(nodeptr &np)
		{
			this->_ensureInTree(np);
			nodeptr new_np;
			if (!(np->_left && np->_right))
			{
				new_np = np->_left ? np->_left : np->_right;
			}
			else
			{
				new_np = this->successor(np);
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
			new_np->_parent = np->_parent;

			this->_delete_node(np);
		}

		bool remove(const value_type &value)
		{
			nodeptr np = this->search(value);
			if (!np)
				return false;
			this->remove(np);
			return true;
		}

		void clear();

	private:
		nodeptr _root = nullptr;
		size_t _size = 0;

		void _ensureInTree(const nodeptr &np) const
		{
			np->_ensureInCont(this);
		}

		template<typename Iter>
		void _build(Iter begin, Iter end);

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

		static void _copy_subtree(BinarySearchTree &dst, const BinarySearchTree &src, nodeptr root);
		void _destroy();
	};
}