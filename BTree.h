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

	template<typename _Valt>
	inline void writeValueToFile(std::ofstream &outf, const _Valt &value)
	{
		outf.write(reinterpret_cast<const char*>(&value), sizeof(_Valt));
	}

	template<typename... Types>
	inline void writeTupleToFile(std::ofstream &outf, const std::tuple<Types...> &tuple)
	{
		traversalTuple(tuple, [&](const auto &e) { writeValueToFile(outf, e); });
	}

	template<typename _Valt>
	inline void readValueFromFile(std::ifstream &inf, _Valt &value)
	{
		inf.read(reinterpret_cast<char*>(&value), sizeof(_Valt));
	}

	template<typename... Types>
	inline void readTupleFromFile(std::ifstream &inf, std::tuple<Types...> &tuple)
	{
		traversalTuple(tuple, [&](auto &e) { readValueFromFile(inf, e); });
	}

	template<typename... Types>
	class BTreeNode
	{
	public:
		using key_type = std::tuple<std::remove_cv_t<std::remove_reference_t<Types>>...>;
		using key_cont = std::vector<key_type>;
		using id_type = lyf::uuid;
		using id_ptr_type = std::unique_ptr<const id_type>;
		const size_t _TypeSize = std::tuple_size_v<key_type>;

	public:
		BTreeNode()
			: _pID(new id_type), _KeyCont()
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
		}

	protected:

		BTreeNode(const id_type *id)
			: _pID(id), _KeyCont()
		{
		}

		id_ptr_type const _pID;
		key_cont _KeyCont;
	};


	template<typename... Types>
	class BTreeInternalNode : public BTreeNode<Types...>
	{
	public:
		using _MyBase = BTreeNode<Types...>;
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

		BTreeInternalNode(const id_type *id)
			: _MyBase(id), _ChildIdCont(), _ChildPtrCont()
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
			outf.write(reinterpret_cast<char*>(&n), sizeof(n));
			for (const auto &t : this->_KeyCont)
			{
				writeTupleToFile(outf, t);
			}
			for (const auto &idp : this->_ChildIdCont)
			{
				idp->toFile(outf);
			}
			outf.close();
		}

		void load(const string &dir) override
		{
			string path = dir + this->_pID->hex();
			std::ifstream inf(path, std::ifstream::binary);
			size_t n;
			inf.read(reinterpret_cast<char*>(&n), sizeof(n));
			this->_KeyCont.resize(n);
			for (auto &t : this->_KeyCont)
			{
				readTupleFromFile(inf, t);
			}
			this->_ChildIdCont.resize(n + 1);
			this->_ChildPtrCont.resize(n + 1);
			for (auto &idp : this->_ChildIdCont)
			{
				idp.reset(new id_type(inf));
			}
			inf.close();
		}

		void loadChild(const string &dir, size_t i)
		{
			// TODO
		}

		void discard() override
		{
			_MyBase::discard();
			_ChildIdCont.clear();
			_ChildPtrCont.clear();
		}

	private:
		child_id_cont _ChildIdCont;
		child_ptr_cont _ChildPtrCont;
	};


	template<typename... Types>
	class BTreeLeafNode : public BTreeNode<Types...>
	{
	public:

		bool isLeaf() const override
		{
			return true;
		}
	};


	template<typename... Types>
	class BTree
	{
	public:
		using Node = BTreeNode<Types...>;
		using NodePtr = std::shared_ptr<Node>;
		using key_type = typename Node::key_type;

	public:
		BTree();


	private:
		NodePtr _Root;
	};
}
