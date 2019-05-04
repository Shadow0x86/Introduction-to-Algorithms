#pragma once
#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "heap.h"


namespace lyf
{
	template<typename Unit>
	class ZEncoderInterface
	{
	public:
		using BitSet = std::vector<bool>;

		virtual void getData(void *pdata, size_t size) = 0;
		virtual void getData(std::function<size_t(void*, size_t)> read) = 0;
		virtual void generate() = 0;
		virtual const BitSet *getCode(Unit data) const = 0;
		virtual void updateLevel() = 0;
		virtual string getHeader() const = 0;
	};


	template<typename Unit>
	class HuffmanEncoder : public ZEncoderInterface<Unit>
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
		using Buff_t = char;

	private:
		struct Header
		{
			Header(uint8_t level, uint8_t nDataUnitBytes)
				: level(level), nDataUnitBytes(nDataUnitBytes)
			{
			}

			uint16_t nHeaderTotalBytes;
			uint8_t level;
			uint8_t nDataFillBytes;
			const uint8_t nDataUnitBytes;

			string getString(const BMap *bmap) const
			{
				// TOTO
				return "";
			}
		};

	public:
		HuffmanEncoder()
			: _Unit2Node(new NMap()), _Unit2BitSet(new BMap()),
			  _isGenerated(false), _header(Header(1, sizeof Unit)),
			  _root(nullptr), _buffer(new Buff_t[_MAX_BUFF_SIZE]), _bfsize(0)
		{
		}

		~HuffmanEncoder()
		{
			this->_reset();
			delete this->_Unit2BitSet;
			delete this->_Unit2Node;
			delete[] this->_buffer;
		}

		virtual void getData(void *pdata, size_t nBytes) override
		{
			this->_ensureNotGenerated();
			char *p = reinterpret_cast<char*>(pdata);
			while (nBytes)
			{
				size_t nw = std::min(nBytes, _MAX_BUFF_SIZE - _bfsize);
				memcpy(_buffer + _bfsize, p, nw);
				p += nw;
				_bfsize += nw;
				if (_bfsize >= _MAX_BUFF_SIZE)
					_flush();
				nBytes -= nw;
			}
		}

		virtual void getData(std::function<size_t(void*, size_t)> read) override
		{
			this->_ensureNotGenerated();
			size_t nRead = read(_buffer + _bfsize, _MAX_BUFF_SIZE - _bfsize);
			while (nRead)
			{
				_bfsize += nRead;
				_flush();
				nRead = read(_buffer, _MAX_BUFF_SIZE);
			}
		}

		virtual void generate() override
		{
			this->_ensureNotGenerated();
			this->_flush();
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
			_isGenerated = true;
		}

		virtual const BitSet *getCode(Unit data) const override
		{
			this->_ensureGenerated();
			auto &bs = *_Unit2BitSet;
			return bs.count(data) ? bs[data] : nullptr;
		}

		virtual void updateLevel() override
		{
			if (this->_header.level >= MAX_COMPRESSION_LEVEL)
				throw std::runtime_error("The compression level has reached the highest.");
			this->_reset();
			this->_header.level++;
		}

		virtual string getHeader() const override
		{
			this->_ensureGenerated();
			return this->_header.getString(this->_Unit2BitSet);
		}

		uint8_t level() const
		{
			return this->_header.level;
		}

		void showCode(size_t n = 0)
		{
			this->_ensureGenerated();
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

		inline static const uint8_t MAX_COMPRESSION_LEVEL = 9;

	private:
		inline static const size_t _MAX_BUFF_SIZE = 1024;
		NMap *_Unit2Node;
		BMap *_Unit2BitSet;
		Node *_root;
		Buff_t *_buffer;
		size_t _bfsize;
		Header _header;
		bool _isGenerated;

		void _ensureGenerated() const
		{
			if (!_isGenerated)
				throw std::runtime_error("Must call generate() first!");
		}

		void _ensureNotGenerated() const
		{
			if (_isGenerated)
				throw std::runtime_error("The code has been generated. Call updateLevel() to reset it.");
		}

		void _flush()
		{
			auto size = _bfsize / sizeof Unit;
			Unit *p = reinterpret_cast<Unit*>(_buffer);
			for (size_t i = 0; i != size; i++)
			{
				auto v = p[i];
				if (_Unit2Node->count(v))
					(*_Unit2Node)[v]->freq++;
				else
					(*_Unit2Node)[v] = new Node(v);
			}
			auto rest = _bfsize - size * sizeof Unit;
			if (rest)
			{
				auto v = p[size] & ~((~static_cast<Unit>(0)) >> rest);
				if (_Unit2Node->count(v))
					(*_Unit2Node)[v]->freq++;
				else
					(*_Unit2Node)[v] = new Node(v);
			}
			_bfsize = 0;
		}

		void _reset()
		{
			for (auto it = _Unit2BitSet->begin(); it != _Unit2BitSet->end(); it++)
				delete it->second;
			_deleteTree(_root);
			_root = nullptr;
			_Unit2Node->clear();
			_Unit2BitSet->clear();
			_bfsize = 0;
			_isGenerated = false;
		}

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
		
	private:
		static const char * const _FilenameSuffix;
	};

	const char * const _BaseZCompresser::_FilenameSuffix = ".zc";


	template<typename Encoder>
	class ZCompresser : public _BaseZCompresser
	{
	public:
		template<typename... Types>
		ZCompresser(Types&&... args)
			: _encoder(std::forward<Types>(args)...)
		{
		}

		void compress(string file, uint8_t level = 9)
		{
			std::ifstream inf(file, std::ifstream::binary);
			if (!inf)
				throw std::runtime_error("The file does not exist.");
			inf.seekg(0, std::ifstream::end);
			size_t rest = inf.tellg();
			inf.seekg(0);

			auto readFile = [&](void *bf, size_t nBytes) -> size_t
			{
				size_t nr = std::min(rest, nBytes);
				inf.read(reinterpret_cast<char*>(bf), nr);
				rest -= nr;
				return nr;
			};
			_encoder.getData(readFile);
			_encoder.generate();
			_encoder.showCode(100);

		}

	private:
		inline static const double _NoDeepFactor = 0.98;
		Encoder _encoder;
	};

	using HuffmanCompresser = ZCompresser<HuffmanEncoder<uint16_t>>;


	class ZDecompresser : public _BaseZCompresser
	{
		void decompress(string file);
	};
}
