#pragma once
#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>


namespace lyf
{
	template<typename... Types>
	class BTreeNode
	{
	public:
		using key_type = std::tuple<std::remove_cv_t<std::remove_reference_t<Types>>...>;
		using key_cont = std::vector<key_type>;
		using child_type = size_t;
		using child_cont = std::vector<child_type>;
		constexpr size_t _TypeSize = std::tuple_size_v<key_type>;

	public:
		BTreeNode(bool isleaf)
			: _pKeyCont(new key_cont()), _pChildCont(nullptr)
		{
			if (isleaf)
			{
				_pChildCont.reset(new child_cont);
			}
		}

		BTreeNode(std::string filename, bool isleaf)
			: BTreeNode(isleaf), _Filename(filename)
		{
			load();
		}

		size_t size() const
		{
			return _pKeyCont->size();
		}

		bool isLeaf() const
		{
			return _pChildCont == nullptr;
		}

		void setFilename(std::string filename)
		{
			_Filename = filename;
		}

		void save() const;

		void load() const;

	private:
		std::unique_ptr<key_cont> _pKeyCont;
		std::unique_ptr<child_cont> _pChildCont;
		std::string _Filename;
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
