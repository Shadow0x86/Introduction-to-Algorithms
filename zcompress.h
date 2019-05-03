#pragma once
#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>
#include "heap.h"


namespace lyf
{
	template<typename Unit>
	class ZEncoderInterface
	{
	public:
		using BitSet = std::vector<bool>;

		virtual void getData(void *pdata, size_t size) = 0;
		virtual void generate() = 0;
		virtual const BitSet *getCode(Unit data) const = 0;
	};

	template<typename Unit>
	class HuffmanTree;

	template<typename Valt>
	class _HuffmanNode
	{
		friend class HuffmanTree<Valt>;

		using freq_t = uint64_t;

		_HuffmanNode()
			: vp(nullptr), freq(1)
		{
		}

		explicit _HuffmanNode(const Valt &value)
			: vp(new Valt(value)), freq(1)
		{
		}

		_HuffmanNode(const Valt &value, size_t freq)
			: vp(new Valt(value)), freq(freq)
		{
		}

		~_HuffmanNode()
		{
			delete vp;
		}

		void inc()
		{
			freq++;
		}

		void dec()
		{
			freq--;
		}

		bool hasValue() const
		{
			return vp != nullptr;
		}

		Valt value() const
		{
			return *vp;
		}

		const Valt * const vp;
		freq_t freq = 1;
		_HuffmanNode *left = nullptr;
		_HuffmanNode *right = nullptr;
	};

	template<typename Unit>
	class HuffmanTree : public ZEncoderInterface<Unit>
	{
	public:
		using _MyBase = ZEncoderInterface<Unit>;
		using BitSet = typename _MyBase::BitSet;
		using Node = _HuffmanNode;
		using NMap = std::unordered_map<Unit, Node*>;
		using BMap = std::unordered_map<Unit, BitSet*>;

		HuffmanTree()
			: _Unit2Node(NMap()), _Unit2BitSet(BMap()), _root(nullptr)
		{
		}

		~HuffmanTree()
		{
			for (auto it = _Unit2BitSet.begin(); it != _Unit2BitSet.end(); it++)
				delete it->second;
			this->_deleteTree(this->_root);
		}

		virtual void getData(void *pdata, size_t size) override
		{
			auto ulen = sizeof Unit;
		}

		virtual void generate() override
		{
			auto heap = lyf::newMinPriorityQueue<Node*>([](auto e) { return e->freq; });
			for (auto it = _Unit2Node.begin(); it != _Unit2Node.end(); it++)
			{
				heap.insert(it->second);
				this->_Unit2BitSet[it->first] = new BitSet();
			}
			size_t n = _Unit2Node.size() - 1;
			for (size_t i = 0; i < n; i++)
			{
				auto np = new Node();
				np->left = heap.pop();
				np->right = heap.pop();
				np->freq = np->left->freq + np->right->freq;
				heap.insert(np);
			}
			_root = heap.pop();
			if (_Unit2Node.size() == 1)
			{
				for (auto it = _Unit2Node.begin(); it != _Unit2Node.end(); it++)
				{
					_Unit2BitSet[it->first] = new BitSet{ 0 };
					break;
				}
			}
			else
			{
				_genBitSet(_root, new BitSet());
			}
		}

		virtual const BitSet *getCode(Unit data) const override
		{
			return this->_Unit2BitSet[data];
		}

	private:
		NMap _Unit2Node;
		BMap _Unit2BitSet;
		Node *_root;

		void _genBitSet(Node *np, BitSet *bp)
		{
			while (np)
			{
				if (np->hasValue())
				{
					_Unit2BitSet[np->value()] = bp;
					break;
				}
				else
				{
					auto newbp = new BitSet(bp->begin(), bp->end());
					newbp->push_back(0);
					_genBitSet(np->left, newbp);
					np = np->right;
					bp->push_back(1);
				}
			}
		}

		void _deleteTree(Node *np)
		{
			while (np)
			{
				_deleteTree(np->left);
				auto r = np->right;
				delete np;
				np = r;
			}
		}
	};


	class _BaseZCompresser
	{
	public:
		using BitSet = std::vector<bool>;
	private:
		static const char * const _FilenameSuffix;
	};

	const char * const _BaseZCompresser::_FilenameSuffix = ".zc";


	template<typename Encoder>
	class ZCompresser : public _BaseZCompresser
	{
	public:
		ZCompresser() {}

		void compress(string file, uint8_t level = 9);
		double compressRate();

	private:
		static const double _NoDeepFactor = 0.95;
	};

	using HuffmanCompresser = ZCompresser<HuffmanTree<uint16_t>>;


	class ZDecompresser : public _BaseZCompresser
	{
		void decompress(string file);
	};
}
