#pragma once
#include <tuple>
#include <vector>
#include <memory>


template<typename... Types>
class BTreeNode
{
public:
	using Valt = std::tuple<Types>;
	using Cont = std::vector<Valt>;

public:
	BTreeNode();

	size_t size() const
	{
		return _Cont.size();
	}

	bool isLeaf() const;

	void save() const;

	void load() const;

private:
	Cont _Cont;
};


template<typename... Types>
class BTree
{
public:
	using Node = BTreeNode<Types...>;
	using NodePtr = std::shared_ptr<Node>;
	using Valt = typename Node::Valt;
	using Cont = typename Node::Cont;
public:
	BTree();


private:
	NodePtr _Root;
};
