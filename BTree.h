#pragma once
#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>


namespace lyf
{

	unsigned char random_char() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 255);
		return static_cast<unsigned char>(dis(gen));
	}

	std::string generate_hex(const unsigned int len) {
		std::stringstream ss;
		for (auto i = 0; i < len; i++) {
			auto rc = random_char();
			std::stringstream hexstream;
			hexstream << std::hex << int(rc);
			auto hex = hexstream.str();
			ss << (hex.length() < 2 ? '0' + hex : hex);
		}
		return ss.str();
	}

	inline std::string uuid()
	{
		return generate_hex(16);
	}

	template<typename... Types>
	class BTreeNode
	{
	public:
		using key_type = std::tuple<std::remove_cv_t<std::remove_reference_t<Types>>...>;
		using key_cont = std::vector<key_type>;
		using child_id_t = std::string;
		//using id_cont = std::vector<child_id_t>;
		using child_ptr_t = std::unique_ptr<BTreeNode>;
		using child_cont = std::vector<child_ptr_t>;
		constexpr size_t _TypeSize = std::tuple_size_v<key_type>;

	public:
		BTreeNode(bool isleaf = false)
			: _pKeyCont(new key_cont()), _pChildCont(nullptr), _MyId(uuid())
		{
			if (isleaf)
			{
				_pChildCont.reset(new child_cont);
			}
		}

		BTreeNode(bool isleaf, const std::string &id)
			: _pKeyCont(new key_cont()), _pChildCont(nullptr), _MyId(id)
		{
			if (isleaf)
			{
				_pChildCont.reset(new child_cont);
			}
		}

		BTreeNode(bool isleaf, const std::string &id, const std::string &dir)
			: BTreeNode(isleaf, id)
		{
			load(dir);
		}

		BTreeNode(const BTreeNode &) = delete;
		BTreeNode& operator=(const BTreeNode &) = delete;

		static std::unique_ptr<BTreeNode> getRoot(const std::string &dir, const std::string &id);

		static std::unique_ptr<BTreeNode> load(const std::string &dir, const std::string &id);

		size_t keySize() const
		{
			return _pKeyCont->size();
		}

		bool isLeaf() const
		{
			return _pChildCont == nullptr;
		}

		const child_id_t &id() const
		{
			return _MyId;
		}

		void save(const std::string &dir) const;

		void load(const std::string &dir);

		void discard()
		{
			_pKeyCont->clear();
			if (_pChildCont)
				_pChildCont->clear();
		}

	private:
		const child_id_t _MyId;
		std::unique_ptr<key_cont> _pKeyCont;
		std::unique_ptr<child_cont> _pChildCont;
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
