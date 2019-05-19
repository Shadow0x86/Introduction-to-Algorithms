#pragma once
#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include "utils.h"


namespace lyf
{

	template<typename _KeyType>
	class BTree;

	template<typename _KeyType>
	class BTreeNode
	{
		friend class BTree<_KeyType>;

	public:
		using key_type = _KeyType;
		using key_cont = std::vector<key_type>;
		using id_type = lyf::uuid;
		using id_ptr_type = const id_type *;

	public:
		BTreeNode()
			: _pID(new id_type), _KeyCont(), _Loaded(false)
		{
		}

		BTreeNode(const id_type *id)
			: _pID(id), _KeyCont(), _Loaded(false)
		{
		}

		BTreeNode(const BTreeNode &) = delete;
		BTreeNode& operator=(const BTreeNode &) = delete;

		size_t KeySize() const
		{
			return _KeyCont.size();
		}

		const key_type &key(size_t i) const
		{
			return this->_KeyCont[i];
		}

		string id() const
		{
			return _pID->hex();
		}

		virtual size_t t() const = 0;

		virtual bool isLeaf() const = 0;

		bool isFull() const noexcept
		{
			return this->KeySize() == this->t() * 2 - 1;
		}

		virtual void save(const path &dir) const = 0;

		virtual void load(const path &dir) = 0;

		virtual void discard()
		{
			_KeyCont.clear();
			this->_Loaded = false;
		}

		std::unique_ptr<const id_type> const _pID;
		key_cont _KeyCont;
		bool _Loaded;
		inline static const size_t PAGE_SIZE = 4096;
	};


	template<typename _KeyType>
	class BTreeLeafNode : public BTreeNode<_KeyType>
	{
		friend class BTree<_KeyType>;

	public:
		using _MyBase = BTreeNode<_KeyType>;
		using key_type = typename _MyBase::key_type;
		using key_cont = typename _MyBase::key_cont;
		using id_type = typename _MyBase::id_type;
		using id_ptr_type = typename _MyBase::id_ptr_type;
		
		BTreeLeafNode()
			: _MyBase()
		{
			this->_static_assert();
		}

		bool isLeaf() const override
		{
			return true;
		}

		constexpr size_t t() const override
		{
			return ((_MyBase::PAGE_SIZE - Serializer<size_t>::SIZE) / Serializer<_KeyType>::SIZE + 1) / 2;
		}

	private:
		BTreeLeafNode(const id_type *id)
			: _MyBase(id)
		{
			this->_static_assert();
		}

		constexpr void _static_assert() const noexcept
		{
			static_assert(_MyBase::PAGE_SIZE > Serializer<size_t>::SIZE);
			static_assert((_MyBase::PAGE_SIZE - Serializer<size_t>::SIZE) / Serializer<_KeyType>::SIZE > 0);
		}

		void save(const path &dir) const override
		{
			size_t n = this->_KeyCont.size();
			std::ofstream outf(dir / this->_pID->hex(), std::ofstream::binary);
			Serializer<size_t>::serialize(outf, n);
			for (const auto &t : this->_KeyCont)
			{
				Serializer<key_type>::serialize(outf, t);
			}
			outf.close();
		}

		void load(const path &dir) override
		{
			if (this->_Loaded)
				return;
			std::ifstream inf(dir / this->_pID->hex(), std::ifstream::binary);
			if (inf)
			{
				size_t n;
				Serializer<size_t>::unserialize(inf, n);
				this->_KeyCont.resize(n);
				for (auto &t : this->_KeyCont)
				{
					Serializer<key_type>::unserialize(inf, t);
				}
			}
			inf.close();
			this->_Loaded = true;
		}
	};


	template<typename _KeyType>
	class BTreeInternalNode : public BTreeNode<_KeyType>
	{
		friend class BTree<_KeyType>;

	public:
		using _MyBase = BTreeNode<_KeyType>;
		using key_type = typename _MyBase::key_type;
		using key_cont = typename _MyBase::key_cont;
		using id_type = typename _MyBase::id_type;
		using id_ptr_type = typename _MyBase::id_ptr_type;
		using child_id_cont = std::vector<id_ptr_type>;
		using child_ptr_t = std::shared_ptr<_MyBase>;
		using child_ptr_cont = std::vector<child_ptr_t>;

		BTreeInternalNode()
			: _MyBase(), _ChildIdCont(), _ChildPtrCont()
		{
			this->_static_assert();
		}

		bool isLeaf() const override
		{
			return false;
		}

		constexpr size_t t() const override
		{
			return ((_MyBase::PAGE_SIZE - Serializer<size_t>::SIZE - Serializer<id_type>::SIZE)
				/ (Serializer<_KeyType>::SIZE + Serializer<id_type>::SIZE) + 1) / 2;
		}

	private:
		BTreeInternalNode(const id_type *id)
			: _MyBase(id), _ChildIdCont(), _ChildPtrCont()
		{
			this->_static_assert();
		}

		constexpr void _static_assert() const noexcept
		{
			static_assert(_MyBase::PAGE_SIZE > Serializer<size_t>::SIZE);
			static_assert(_MyBase::PAGE_SIZE - Serializer<size_t>::SIZE > Serializer<id_type>::SIZE);
			static_assert((_MyBase::PAGE_SIZE - Serializer<size_t>::SIZE - Serializer<id_type>::SIZE)
				/ (Serializer<_KeyType>::SIZE + Serializer<id_type>::SIZE) > 0);
		}

		void save(const path &dir) const override
		{
			size_t n = this->_KeyCont.size();
			std::ofstream outf(dir / this->_pID->hex(), std::ofstream::binary);
			Serializer<size_t>::serialize(outf, n);
			for (const auto &t : this->_KeyCont)
			{
				Serializer<key_type>::serialize(outf, t);
			}
			for (auto idp : this->_ChildIdCont)
			{
				Serializer<id_type>::serialize(outf, *idp);
			}
			outf.close();
		}

		void load(const path &dir) override
		{
			if (this->_Loaded)
				return;
			std::ifstream inf(dir / this->_pID->hex(), std::ifstream::binary);
			if (inf)
			{
				size_t n;
				Serializer<size_t>::unserialize(inf, n);
				this->_KeyCont.resize(n);
				for (auto &t : this->_KeyCont)
				{
					Serializer<key_type>::unserialize(inf, t);
				}
				this->_ChildIdCont.resize(n + 1);
				this->_ChildPtrCont.resize(n + 1);
				for (auto &idp : this->_ChildIdCont)
				{
					idp = Serializer<id_type>::unserialize(inf).release();
				}
			}
			inf.close();
			this->_Loaded = true;
		}

		void loadChild(size_t i, const path &dir)
		{
			if (!this->_ChildPtrCont[i])
			{
				this->_ChildPtrCont[i].reset(new BTreeInternalNode(_ChildIdCont[i]));
			}
			this->_ChildPtrCont[i]->load(dir);
		}

		void splitChild(size_t i, const path &dir)
		{
			auto y = _ChildPtrCont[i];
			child_ptr_t z;
			if (y->isLeaf())
				z.reset(new BTreeLeafNode<key_type>);
			else
				z.reset(new BTreeInternalNode);
			auto t = y->t();
			size_t newkeysize = t - 1;
			z->_KeyCont.resize(newkeysize);
			for (size_t i = 0; i != newkeysize; i++)
				z->_KeyCont[i] = std::move(y->_KeyCont[i + t]);
			if (!y->isLeaf())
			{
				auto yp = dynamic_cast<BTreeInternalNode*>(y.get());
				auto zp = dynamic_cast<BTreeInternalNode*>(z.get());
				zp->_ChildIdCont.resize(t);
				zp->_ChildPtrCont.resize(t);
				for (size_t i = 0; i != t; i++)
				{
					zp->_ChildIdCont[i] = std::move(yp->_ChildIdCont[i + t]);
					zp->_ChildPtrCont[i] = std::move(yp->_ChildPtrCont[i + t]);
				}
			}
			this->_KeyCont.insert(_KeyCont.begin() + i, std::move(y->_KeyCont[newkeysize]));
			y->_KeyCont.resize(newkeysize);
			this->_ChildIdCont.insert(_ChildIdCont.begin() + i + 1, z->_pID.get());
			this->_ChildPtrCont.insert(_ChildPtrCont.begin() + i + 1, z);
			this->save(dir);
			y->save(dir);
			z->save(dir);
		}

		void discard() override
		{
			_MyBase::discard();
			_ChildIdCont.clear();
			_ChildPtrCont.clear();
		}

		child_id_cont _ChildIdCont;
		child_ptr_cont _ChildPtrCont;
	};


	template<typename _KeyType>
	class BTree
	{
	public:
		using Node = BTreeNode<_KeyType>;
		using NodePtr = std::shared_ptr<Node>;
		using key_type = typename Node::key_type;
		using id_type = typename Node::id_type;

	public:
		BTree(const path &dir)
			: _Dir(std::filesystem::absolute(dir)), _Root()
		{
			if (std::filesystem::is_regular_file(_Dir))
				throw std::invalid_argument("dir");
			std::filesystem::create_directory(_Dir);
			auto rootpath = _Dir / "root";
			std::ifstream inf(rootpath, std::ifstream::binary);
			if (inf)
			{
				auto idp = Serializer<id_type>::unserialize(inf);
				_Root.reset(new BTreeLeafNode<key_type>(idp.release()));
			}
			else
			{
				_Root.reset(new BTreeLeafNode<key_type>);
				std::ofstream outf(rootpath, std::ofstream::binary);
				Serializer<id_type>::serialize(outf, *(_Root->_pID));
				outf.close();
			}
			_Root->load(dir);
		}

		BTree(const BTree &) = delete;
		BTree &operator=(const BTree &) = delete;

		std::pair<NodePtr, size_t> search(const key_type &key) const
		{
			return _searchRecursive(key, _Root);
		}

		void insert(const key_type &key)
		{
			auto np = _Root;
			if (np->isFull())
			{
				auto p = new BTreeInternalNode<key_type>;
				_Root.reset(p);
				p->_ChildIdCont.push_back(np->_pID.get());
				p->_ChildPtrCont.push_back(np);
				p->splitChild(0, _Dir);
				_insertNonFull(_Root, key);
			}
			else
				_insertNonFull(np, key);
		}

		void remove(const key_type &key);

		void discard()
		{
			_Root = nullptr;
			//TODO:remove all files
		}

	private:
		std::pair<NodePtr, size_t> _searchRecursive(const key_type &key, NodePtr np) const
		{
			size_t i = 0;
			while (i != np->KeySize() && key > np->key(i))
				i++;
			if (i != np->KeySize() && key == np->key(i))
				return { np,i };
			else if (np->isLeaf())
				return { nullptr,0 };
			else
			{
				auto p = dynamic_cast<BTreeInternalNode<key_type>*>(np.get());
				p->loadChild(i, this->_Dir);
				return _searchRecursive(key, p->_ChildPtrCont[i]);
			}
		}

		void _insertNonFull(NodePtr np, const key_type &key)
		{
			size_t i = 0;
			if (np->isLeaf())
			{
				while (i != np->KeySize() && key > np->key(i))
					i++;
				np->_KeyCont.insert(np->_KeyCont.begin() + i, key);
				np->save(this->_Dir);
			}
			else
			{
				while (i != np->KeySize() && key > np->key(i))
					i++;
				auto p = dynamic_cast<BTreeInternalNode<key_type>*>(np.get());
				p->loadChild(i, this->_Dir);
				if (p->_ChildPtrCont[i]->isFull())
				{
					p->splitChild(i, this->_Dir);
					if (key > p->key(i))
						i++;
				}
				_insertNonFull(p->_ChildPtrCont[i], key);
			}
		}

		path const _Dir;
		NodePtr _Root;
	};
}
