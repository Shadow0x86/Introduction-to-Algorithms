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
		virtual void reset() = 0;
	};


	template<typename Unit>
	class HuffmanTree : public ZEncoderInterface<Unit>
	{
	private:
		struct _HuffmanNode
		{
			using freq_t = uint64_t;

			_HuffmanNode()
				: vp(nullptr), freq(1)
			{
			}

			explicit _HuffmanNode(const Unit &value)
				: vp(new Unit(value)), freq(1)
			{
			}

			_HuffmanNode(const Unit &value, size_t freq)
				: vp(new Unit(value)), freq(freq)
			{
			}

			~_HuffmanNode()
			{
				delete vp;
			}

			bool hasValue() const
			{
				return vp != nullptr;
			}

			Unit value() const
			{
				return *vp;
			}

			const Unit * const vp;
			freq_t freq = 1;
			_HuffmanNode *left = nullptr;
			_HuffmanNode *right = nullptr;
		};

	public:
		using _MyBase = ZEncoderInterface<Unit>;
		using BitSet = typename _MyBase::BitSet;
		using Node = _HuffmanNode;
		using NMap = std::unordered_map<Unit, Node*>;
		using BMap = std::unordered_map<Unit, BitSet*>;

		HuffmanTree()
			: _Unit2Node(new NMap()), _Unit2BitSet(new BMap()), _root(nullptr)
		{
		}

		~HuffmanTree()
		{
			this->reset();
			delete this->_Unit2BitSet;
			delete this->_Unit2Node;
		}

		virtual void getData(void *pdata, size_t nBytes) override
		{
			auto size = nBytes / sizeof Unit;
			Unit *p = reinterpret_cast<Unit*>(pdata);
			for (size_t i = 0; i != size; i++)
			{
				auto v = p[i];
				if (_Unit2Node->count(v))
					(*_Unit2Node)[v]->freq++;
				else
					(*_Unit2Node)[v] = new Node(v);
			}
			auto rest = nBytes - size * sizeof Unit;
			if (rest)
			{
				auto v = p[size] & ~((~static_cast<Unit>(0))>>rest);
				if (_Unit2Node->count(v))
					(*_Unit2Node)[v]->freq++;
				else
					(*_Unit2Node)[v] = new Node(v);
			}
		}

		virtual void generate() override
		{
			auto heap = lyf::newMinPriorityQueue<Node*>([](auto e) { return e->freq; });
			for (auto it = _Unit2Node->begin(); it != _Unit2Node->end(); it++)
			{
				heap.insert(it->second);
				(*_Unit2BitSet)[it->first] = new BitSet();
			}
			size_t n = _Unit2Node->size() - 1;
			for (size_t i = 0; i < n; i++)
			{
				auto np = new Node();
				np->left = heap.pop();
				np->right = heap.pop();
				np->freq = np->left->freq + np->right->freq;
				heap.insert(np);
			}
			_root = heap.pop();
			if (_Unit2Node->size() == 1)
			{
				for (auto it = _Unit2Node->begin(); it != _Unit2Node->end(); it++)
				{
					(*_Unit2BitSet)[it->first] = new BitSet{ 0 };
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
			auto &bs = *_Unit2BitSet;
			return bs.count(data) ? bs[data] : nullptr;
		}

		virtual void reset() override
		{
			for (auto it = _Unit2BitSet->begin(); it != _Unit2BitSet->end(); it++)
				delete it->second;
			_deleteTree(_root);
			_root = nullptr;
			_Unit2Node->clear();
			_Unit2BitSet->clear();
		}

		void showCode(size_t n = 0)
		{
			if (!n)
				n = _Unit2BitSet->size();
			size_t k = 0;
			for (auto it = _Unit2BitSet->begin(); it != _Unit2BitSet->end() && k != n; it++, k++)
			{
				cout << it->first << ": ";
				for (auto i = it->second->begin(), end = it->second->end(); i != end; i++)
				{
					cout << *i;
				}
				cout << endl;
			}
		}

	private:
		NMap *_Unit2Node;
		BMap *_Unit2BitSet;
		Node *_root;

		void _genBitSet(Node *np, BitSet *bp)
		{
			while (np)
			{
				if (np->hasValue())
				{
					(*_Unit2BitSet)[np->value()] = bp;
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
		ZCompresser()
			: _encoder(Encoder())
		{
		}

		void compress(string file, uint8_t level = 9)
		{
			std::ifstream inf(file, std::ifstream::binary);
			inf.seekg(0, std::ifstream::end);
			size_t fsize = inf.tellg();
			inf.seekg(0);
			size_t bfsize = 1024;
			char *buffer = new char[bfsize];
			size_t cnt = fsize / bfsize;
			for (size_t i = 0; i != cnt; i++)
			{
				inf.read(buffer, bfsize);
				_encoder.getData(buffer, bfsize);
			}
			size_t rest = fsize - cnt * bfsize;
			inf.read(buffer, rest);
			_encoder.getData(buffer, rest);
			inf.close();
			_encoder.generate();
		}

	private:
		inline static const double _NoDeepFactor = 0.98;
		Encoder _encoder;
	};

	using HuffmanCompresser = ZCompresser<HuffmanTree<uint16_t>>;


	class ZDecompresser : public _BaseZCompresser
	{
		void decompress(string file);
	};
}
