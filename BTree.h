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
	public:
		using key_type = _KeyType;
		using key_cont = std::vector<key_type>;
		using id_type = lyf::uuid;
		using id_ptr_type = std::unique_ptr<const id_type>;

	public:
		BTreeNode()
			: _pID(new id_type), _KeyCont(), _Loaded(false)
		{
		}

		BTreeNode(const BTreeNode &) = delete;
		BTreeNode& operator=(const BTreeNode &) = delete;

		size_t keySize() const
		{
			return _KeyCont.size();
		}

		string id() const
		{
			return _pID->hex();
		}

		virtual bool isLeaf() const = 0;

		virtual void save(const string &dir) const = 0;

		virtual void load(const string &dir) = 0;

		virtual void discard()
		{
			_KeyCont.clear();
			this->_Loaded = false;
		}

	protected:
		BTreeNode(const id_type *id)
			: _pID(id), _KeyCont(), _Loaded(false)
		{
		}

		id_ptr_type const _pID;
		key_cont _KeyCont;
		bool _Loaded;
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
		using child_ptr_t = std::unique_ptr<BTreeNode>;
		using child_ptr_cont = std::vector<child_ptr_t>;

		BTreeInternalNode()
			: _MyBase(), _ChildIdCont(), _ChildPtrCont()
		{
		}

		bool isLeaf() const override
		{
			return false;
		}

		void save(const string &dir) const override
		{
			size_t n = this->_KeyCont.size();
			string path = dir + this->_pID->hex();
			std::ofstream outf(path, std::ofstream::binary);
			Serializer<size_t>::serialize(outf, n);
			for (const auto &t : this->_KeyCont)
			{
				Serializer<key_type>::serialize(outf, t);
			}
			for (const auto &idp : this->_ChildIdCont)
			{
				Serializer<id_type>::serialize(outf, *idp);
			}
			outf.close();
		}

		void load(const string &dir) override
		{
			if (this->_Loaded)
				return;
			string path = dir + this->_pID->hex();
			std::ifstream inf(path, std::ifstream::binary);
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
					idp = Serializer<id_type>::unserialize(inf);
				}
			}
			inf.close();
			this->_Loaded = true;
		}

		void loadChild(const string &dir, size_t i)
		{
			this->_ChildPtrCont[i]->load(dir);
		}

		void discard() override
		{
			_MyBase::discard();
			_ChildIdCont.clear();
			_ChildPtrCont.clear();
		}

	private:
		BTreeInternalNode(const id_type *id)
			: _MyBase(id), _ChildIdCont(), _ChildPtrCont()
		{
		}

		child_id_cont _ChildIdCont;
		child_ptr_cont _ChildPtrCont;
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
		}

		bool isLeaf() const override
		{
			return true;
		}

		void save(const string &dir) const override
		{
			size_t n = this->_KeyCont.size();
			string path = dir + this->_pID->hex();
			std::ofstream outf(path, std::ofstream::binary);
			Serializer<size_t>::serialize(outf, n);
			for (const auto &t : this->_KeyCont)
			{
				Serializer<key_type>::serialize(outf, t);
			}
			outf.close();
		}

		void load(const string &dir) override
		{
			if (this->_Loaded)
				return;
			string path = dir + this->_pID->hex();
			std::ifstream inf(path, std::ifstream::binary);
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

	private:
		BTreeLeafNode(const id_type *id)
			: _MyBase(id)
		{
		}
	};


	template<typename _KeyType>
	class BTree
	{
	public:
		using Node = BTreeNode<_KeyType>;
		using NodePtr = std::shared_ptr<Node>;
		using key_type = typename Node::key_type;

	public:
		BTree(const string &dir)
			: _Dir(dir), _Root()
		{
			std::ifstream inf(dir + "root", std::ifstream::binary);
			if (inf)
			{
				auto idp = Serializer<uuid>::unserialize(inf);
				_Root.reset(new BTreeLeafNode(idp.release()));
			}
			else
			{
				_Root.reset(new BTreeLeafNode);
			}
			_Root->load(dir);
		}

		std::pair<NodePtr, size_t> search(const key_type &key, NodePtr root = nullptr);

	private:
		string const _Dir;
		NodePtr _Root;
	};
}
