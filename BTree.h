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
		using id_ptr_type = std::shared_ptr<const id_type>;

	public:
		BTreeNode(const path &dir)
			: _pID(new id_type), _KeyCont(), _Loaded(false), _FileModifier(dir / _pID->hex())
		{
		}

		BTreeNode(const path &dir, id_ptr_type id)
			: _pID(id), _KeyCont(), _Loaded(false), _FileModifier(dir / _pID->hex())
		{
		}

		BTreeNode(const BTreeNode &) = delete;
		BTreeNode& operator=(const BTreeNode &) = delete;

		size_t KeySize() const noexcept
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

		virtual size_t t() const noexcept = 0;

		virtual bool isLeaf() const noexcept = 0;

		bool isFull() const noexcept
		{
			return this->KeySize() == this->t() * 2 - 1;
		}

		virtual void save() = 0;

		virtual void load() = 0;

		void removeFile()
		{
			_FileModifier.close();
			std::filesystem::remove(_FileModifier.getPath());
		}

		virtual void discard()
		{
			_KeyCont.clear();
			this->_Loaded = false;
		}

		id_ptr_type const _pID;
		key_cont _KeyCont;
		bool _Loaded;
		lyf::FileModifier _FileModifier;
		inline static const size_t PAGE_SIZE = 4096;
	};


	template<typename _KeyType>
	class BTreeInternalNode;


	template<typename _KeyType>
	class BTreeLeafNode : public BTreeNode<_KeyType>
	{
		friend class BTree<_KeyType>;
		friend class BTreeInternalNode<_KeyType>;

	public:
		using _MyBase = BTreeNode<_KeyType>;
		using key_type = typename _MyBase::key_type;
		using key_cont = typename _MyBase::key_cont;
		using id_type = typename _MyBase::id_type;
		using id_ptr_type = typename _MyBase::id_ptr_type;

		bool isLeaf() const noexcept override
		{
			return true;
		}

		constexpr size_t t() const noexcept override
		{
			return ((_MyBase::PAGE_SIZE - Serializer<size_t>::SIZE) / Serializer<_KeyType>::SIZE + 1) / 2;
		}

	private:
		BTreeLeafNode(const path &dir)
			: _MyBase(dir)
		{
			this->_static_assert();
		}

		BTreeLeafNode(const path &dir, id_ptr_type id)
			: _MyBase(dir, id)
		{
			this->_static_assert();
		}

		constexpr void _static_assert() const noexcept
		{
			static_assert(_MyBase::PAGE_SIZE > Serializer<size_t>::SIZE);
			static_assert((_MyBase::PAGE_SIZE - Serializer<size_t>::SIZE) / Serializer<_KeyType>::SIZE > 0);
		}

		void insert(const key_type &key)
		{
			size_t i = 0;
			while (i != _KeyCont.size() && key > _KeyCont[i])
				i++;
			_KeyCont.insert(_KeyCont.begin() + i, key);
			size_t pos = lyf::Serializer<size_t>::SIZE + lyf::Serializer<key_type>::SIZE * i;
			auto s = lyf::Serializer<key_type>::serialize(key);
			size_t n = _KeyCont.size();
			this->_FileModifier.write(0, reinterpret_cast<char*>(&n), sizeof(n));
			this->_FileModifier.insert(pos, s.data(), s.size());
		}

		void save() override
		{
			size_t n = this->_KeyCont.size();
			this->_FileModifier.truncate();
			this->_FileModifier.append(reinterpret_cast<char*>(&n), sizeof(n));
			for (const auto &t : this->_KeyCont)
			{
				auto s = lyf::Serializer<key_type>::serialize(t);
				this->_FileModifier.write(s.data(), s.size());
			}
		}

		void load() override
		{
			if (this->_Loaded)
				return;
			if (this->_FileModifier.size())
			{
				size_t n;
				this->_FileModifier.read(0, reinterpret_cast<char*>(&n), sizeof(n));
				this->_KeyCont.resize(n);
				string s(Serializer<key_type>::SIZE, '\0');
				for (auto &t : this->_KeyCont)
				{
					this->_FileModifier.read(s.data(), Serializer<key_type>::SIZE);
					Serializer<key_type>::unserialize(s, t);
				}
			}
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

		bool isLeaf() const noexcept override
		{
			return false;
		}

		constexpr size_t t() const noexcept override
		{
			return ((_MyBase::PAGE_SIZE - Serializer<bool>::SIZE - Serializer<size_t>::SIZE - Serializer<id_type>::SIZE)
				/ (Serializer<_KeyType>::SIZE + Serializer<id_type>::SIZE) + 1) / 2;
		}

	private:
		BTreeInternalNode(const path &dir)
			: _MyBase(dir), _isChildLeaf(false), _ChildIdCont(), _ChildPtrCont()
		{
			this->_static_assert();
		}

		BTreeInternalNode(const path &dir, id_ptr_type id)
			: _MyBase(dir, id), _isChildLeaf(false), _ChildIdCont(), _ChildPtrCont()
		{
			this->_static_assert();
		}

		BTreeInternalNode(const path &dir, bool _isChildLeaf)
			: _MyBase(dir), _isChildLeaf(_isChildLeaf), _ChildIdCont(), _ChildPtrCont()
		{
			this->_static_assert();
		}

		constexpr void _static_assert() const noexcept
		{
			static_assert(_MyBase::PAGE_SIZE > Serializer<size_t>::SIZE + Serializer<bool>::SIZE);
			static_assert(_MyBase::PAGE_SIZE - Serializer<bool>::SIZE - Serializer<size_t>::SIZE > Serializer<id_type>::SIZE);
			static_assert((_MyBase::PAGE_SIZE - Serializer<bool>::SIZE - Serializer<size_t>::SIZE - Serializer<id_type>::SIZE)
				/ (Serializer<_KeyType>::SIZE + Serializer<id_type>::SIZE) > 0);
		}

		void save() override
		{
			size_t n = this->_KeyCont.size();
			this->_FileModifier.truncate();
			this->_FileModifier.append(reinterpret_cast<char*>(&_isChildLeaf), sizeof(_isChildLeaf));
			this->_FileModifier.write(reinterpret_cast<char*>(&n), sizeof(n));
			for (const auto &t : this->_KeyCont)
			{
				auto s = lyf::Serializer<key_type>::serialize(t);
				this->_FileModifier.write(s.data(), s.size());
			}
			for (auto idp : this->_ChildIdCont)
			{
				auto s = lyf::Serializer<id_type>::serialize(*idp);
				this->_FileModifier.write(s.data(), s.size());
			}
		}

		void load() override
		{
			if (this->_Loaded)
				return;
			if (this->_FileModifier.size())
			{
				this->_FileModifier.read(0, reinterpret_cast<char*>(&_isChildLeaf), sizeof(_isChildLeaf));
				size_t n;
				this->_FileModifier.read(reinterpret_cast<char*>(&n), sizeof(n));
				this->_KeyCont.resize(n);
				string s(Serializer<key_type>::SIZE, '\0');
				for (auto &t : this->_KeyCont)
				{
					this->_FileModifier.read(s.data(), Serializer<key_type>::SIZE);
					Serializer<key_type>::unserialize(s, t);
				}
				this->_ChildIdCont.resize(n + 1);
				this->_ChildPtrCont.resize(n + 1);
				s.resize(Serializer<id_type>::SIZE);
				for (auto &idp : this->_ChildIdCont)
				{
					this->_FileModifier.read(s.data(), Serializer<id_type>::SIZE);
					idp.reset(Serializer<id_type>::unserialize(s).release());
				}
			}
			this->_Loaded = true;
		}

		child_ptr_t loadChild(size_t i)
		{
			if (!this->_ChildPtrCont[i])
			{
				auto dir = this->_FileModifier.getPath().parent_path();
				_MyBase *p;
				if (this->_isChildLeaf)
					p = new BTreeLeafNode<key_type>(dir, _ChildIdCont[i]);
				else
					p = new BTreeInternalNode(dir, _ChildIdCont[i]);
				this->_ChildPtrCont[i].reset(p);
			}
			this->_ChildPtrCont[i]->load();
			return this->_ChildPtrCont[i];
		}

		void splitChild(size_t i)
		{
			auto y = loadChild(i);
			child_ptr_t z;
			auto dir = this->_FileModifier.getPath().parent_path();
			if (y->isLeaf())
				z.reset(new BTreeLeafNode<key_type>(dir));
			else
				z.reset(new BTreeInternalNode(dir, dynamic_cast<BTreeInternalNode*>(y.get())->_isChildLeaf));
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
				yp->_ChildIdCont.resize(t);
				yp->_ChildPtrCont.resize(t);
			}
			this->_KeyCont.insert(_KeyCont.begin() + i, std::move(y->_KeyCont[newkeysize]));
			y->_KeyCont.resize(newkeysize);
			this->_ChildIdCont.insert(_ChildIdCont.begin() + i + 1, z->_pID);
			this->_ChildPtrCont.insert(_ChildPtrCont.begin() + i + 1, z);
			this->save();
			y->save();
			z->save();
		}

		void mergeChild(size_t i)
		{
			auto pLeftChild = loadChild(i), pRightChild = loadChild(i + 1);
			pLeftChild->_KeyCont.push_back(std::move(_KeyCont[i]));
			for (auto &e : pRightChild->_KeyCont)
			{
				pLeftChild->_KeyCont.push_back(std::move(e));
			}
			if (!(pRightChild->isLeaf()))
			{
				auto lp = dynamic_cast<BTreeInternalNode<key_type>*>(pLeftChild.get());
				auto rp = dynamic_cast<BTreeInternalNode<key_type>*>(pRightChild.get());
				for (auto &e : rp->_ChildIdCont)
					lp->_ChildIdCont.push_back(std::move(e));
				for (auto &e : rp->_ChildPtrCont)
					lp->_ChildPtrCont.push_back(std::move(e));
			}
			pRightChild->removeFile();
			_KeyCont.erase(_KeyCont.begin() + i);
			_ChildIdCont.erase(_ChildIdCont.begin() + i + 1);
			_ChildPtrCont.erase(_ChildPtrCont.begin() + i + 1);
			this->save();
			pLeftChild->save();
		}

		void discard() override
		{
			_MyBase::discard();
			_ChildIdCont.clear();
			_ChildPtrCont.clear();
		}

		bool _isChildLeaf;
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
			this->_loadRoot();
		}

		BTree(const BTree &) = delete;
		BTree &operator=(const BTree &) = delete;

		std::pair<NodePtr, size_t> search(const key_type &key) const
		{
			auto np = _Root;
			while (true)
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
					np = p->loadChild(i);
				}
			}
		}

		void insert(const key_type &key)
		{
			auto np = _Root;
			if (np->isFull())
			{
				auto p = new BTreeInternalNode<key_type>(_Dir, np->isLeaf());
				_Root.reset(p);
				p->_ChildIdCont.push_back(np->_pID);
				p->_ChildPtrCont.push_back(np);
				p->splitChild(0);
				_saveRoot();
				_insertNonFull(_Root, key);
			}
			else
				_insertNonFull(np, key);
		}

		void remove(const key_type &key)
		{
			_recursiveRemove(_Root, key);
		}

		void discard()
		{
			_Root.reset();
			std::filesystem::remove_all(_Dir);
			this->_loadRoot();
		}

		inline static string const ROOT_FILE_NAME = "root";

	private:
		void _loadRoot()
		{
			std::filesystem::create_directory(_Dir);
			std::ifstream inf(_Dir / ROOT_FILE_NAME, std::ifstream::binary);
			if (inf.is_open())
			{
				bool isLeaf;
				Serializer<bool>::unserialize(inf, isLeaf);
				auto idp = Serializer<id_type>::unserialize(inf);
				if (isLeaf)
					_Root.reset(new BTreeLeafNode<key_type>(_Dir, typename Node::id_ptr_type(idp.release())));
				else
					_Root.reset(new BTreeInternalNode<key_type>(_Dir, typename Node::id_ptr_type(idp.release())));
			}
			else
			{
				_Root.reset(new BTreeLeafNode<key_type>(_Dir));
				_saveRoot();
			}
			_Root->load();
		}

		void _saveRoot() const
		{
			std::ofstream outf(_Dir / ROOT_FILE_NAME, std::ofstream::binary);
			if (!outf.is_open())
				throw std::runtime_error("Fail to create file " + ROOT_FILE_NAME);
			Serializer<bool>::serialize(outf, _Root->isLeaf());
			Serializer<id_type>::serialize(outf, *(_Root->_pID));
			outf.close();
		}

		void _insertNonFull(NodePtr np, const key_type &key)
		{
			while (true)
			{
				if (np->isLeaf())
				{
					dynamic_cast<BTreeLeafNode<key_type>*>(np.get())->insert(key);
					return;
				}
				else
				{
					size_t i = 0;
					while (i != np->KeySize() && key > np->key(i))
						i++;
					auto p = dynamic_cast<BTreeInternalNode<key_type>*>(np.get());
					np = p->loadChild(i);
					if (np->isFull())
					{
						p->splitChild(i);
						if (key > p->key(i))
							i++;
					}
				}
			}
		}

		bool _recursiveRemove(NodePtr np, const key_type &key)
		{
			np->load();
			size_t i = 0;
			while (i != np->KeySize() && key > np->key(i))
				i++;
			if (i != np->KeySize() && key == np->key(i))
			{
				if (np->isLeaf())
				{
					np->_KeyCont.erase(np->_KeyCont.begin() + i);
					np->save();
					return true;
				}
				else
				{
					auto p = dynamic_cast<BTreeInternalNode<key_type>*>(np.get());
					size_t sz;
					auto pLeftChild = p->loadChild(i);
					if ((sz = pLeftChild->KeySize()) >= pLeftChild->t())
					{
						p->_KeyCont[i] = pLeftChild->key(sz - 1);
						return _recursiveRemove(pLeftChild, p->_KeyCont[i]);
					}
					auto pRightChild = p->loadChild(i + 1);
					if ((sz = pRightChild->KeySize()) >= pRightChild->t())
					{
						p->_KeyCont[i] = pRightChild->key(0);
						return _recursiveRemove(pRightChild, p->_KeyCont[i]);
					}
					p->mergeChild(i);
					if (!p->KeySize())
					{
						p->removeFile();
						_Root = pLeftChild;
						_saveRoot();
					}
					return _recursiveRemove(pLeftChild, key);
				}
			}
			else if (!np->isLeaf())
			{
				auto p = dynamic_cast<BTreeInternalNode<key_type>*>(np.get());
				auto pChild = p->loadChild(i);
				if (pChild->KeySize() < pChild->t())
				{
					size_t sib_i = i + 1;
					auto sibling = p->loadChild(sib_i);
					if (sibling->KeySize() >= sibling->t())
					{
						pChild->_KeyCont.push_back(std::move(p->_KeyCont[i]));
						p->_KeyCont[i] = std::move(sibling->_KeyCont[0]);
						sibling->_KeyCont.erase(sibling->_KeyCont.begin());
						if (!pChild->isLeaf())
						{
							auto rawpc = dynamic_cast<BTreeInternalNode<key_type>*>(pChild.get());
							auto rawsib = dynamic_cast<BTreeInternalNode<key_type>*>(sibling.get());
							rawpc->_ChildIdCont.push_back(std::move(rawsib->_ChildIdCont[0]));
							rawpc->_ChildPtrCont.push_back(std::move(rawsib->_ChildPtrCont[0]));
							rawsib->_ChildIdCont.erase(rawsib->_ChildIdCont.begin());
							rawsib->_ChildPtrCont.erase(rawsib->_ChildPtrCont.begin());
						}
						goto no_merge;
					}
					if (i)
					{
						sib_i = i - 1;
						sibling = p->loadChild(sib_i);
						if (sibling->KeySize() >= sibling->t())
						{
							pChild->_KeyCont.insert(pChild->_KeyCont.begin(), std::move(p->_KeyCont[sib_i]));
							p->_KeyCont[sib_i] = std::move(sibling->_KeyCont.back());
							sibling->_KeyCont.pop_back();
							if (!pChild->isLeaf())
							{
								auto rawpc = dynamic_cast<BTreeInternalNode<key_type>*>(pChild.get());
								auto rawsib = dynamic_cast<BTreeInternalNode<key_type>*>(sibling.get());
								rawpc->_ChildIdCont.insert(rawpc->_ChildIdCont.begin(), std::move(rawsib->_ChildIdCont.back()));
								rawpc->_ChildPtrCont.insert(rawpc->_ChildPtrCont.begin(), std::move(rawsib->_ChildPtrCont.back()));
								rawsib->_ChildIdCont.pop_back();
								rawsib->_ChildPtrCont.pop_back();
							}
							goto no_merge;
						}
					}
					if (sib_i == i + 1)
						p->mergeChild(i);
					else
						p->mergeChild(i - 1);
					pChild = p->loadChild(i);
					if (!p->KeySize())
					{
						p->removeFile();
						_Root = pChild;
						_saveRoot();
					}
				}
				no_merge:
				return _recursiveRemove(pChild, key);
			}
			else
				return false;
		}

		path const _Dir;
		NodePtr _Root;
	};
}
