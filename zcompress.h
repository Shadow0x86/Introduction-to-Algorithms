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
		virtual void generate() = 0;
		virtual const BitSet &getCode(Unit data) const = 0;
		virtual void updateLevel() = 0;
		virtual string getHeader() const = 0;
		virtual double compressionRatio() const = 0;
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

			bool hasValue() const
			{
				return vp != nullptr;
			}

			Unit value() const
			{
				return *vp;
			}

			std::unique_ptr<const Unit> const vp;
			freq_t freq = 1;
			std::shared_ptr<_HuffmanNode> left = nullptr;
			std::shared_ptr<_HuffmanNode> right = nullptr;
		};

	public:
		using _MyBase = ZEncoderInterface<Unit>;
		using BitSet = typename _MyBase::BitSet;
		using Node = _HuffmanNode;
		using NMap = std::unordered_map<Unit, std::shared_ptr<Node>>;
		using BMap = std::unordered_map<Unit, std::unique_ptr<BitSet>>;
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

			string getString(const BMap &bmap) const
			{
				// TOTO
				return "";
			}
		};

	public:
		HuffmanEncoder()
			: _pUnit2Node(new NMap()), _pUnit2BitSet(new BMap()),
			  _isGenerated(false), _header(Header(1, sizeof Unit)),
			  _pRoot(nullptr), _pBuffer(new Buff_t[_MAX_BUFF_SIZE]), _bfsize(0)
		{
		}

		virtual void getData(void *pdata, size_t nBytes) override
		{
			this->_ensureNotGenerated();
			char *p = reinterpret_cast<char*>(pdata);
			while (nBytes)
			{
				size_t nw = std::min(nBytes, _MAX_BUFF_SIZE - _bfsize);
				memcpy(_pBuffer.get() + _bfsize, p, nw);
				p += nw;
				_bfsize += nw;
				if (_bfsize >= _MAX_BUFF_SIZE)
					_flush();
				nBytes -= nw;
			}
		}

		virtual void generate() override
		{
			this->_ensureNotGenerated();
			this->_flush();
			auto heap = lyf::newMinPriorityQueue<std::shared_ptr<Node>>([](auto e) { return e->freq; });
			for (auto it = _pUnit2Node->begin(); it != _pUnit2Node->end(); it++)
			{
				heap.insert(it->second);
				(*_pUnit2BitSet)[it->first].reset(new BitSet());
			}
			size_t n = _pUnit2Node->size() - 1;
			for (size_t i = 0; i < n; i++)
			{
				std::shared_ptr<Node> np(new Node());
				np->left = heap.pop();
				np->right = heap.pop();
				np->freq = np->left->freq + np->right->freq;
				heap.insert(np);
			}
			_pRoot = heap.pop();
			if (_pUnit2Node->size() == 1)
			{
				for (auto it = _pUnit2Node->begin(); it != _pUnit2Node->end(); it++)
				{
					(*_pUnit2BitSet)[it->first].reset(new BitSet{ 0 });
					break;
				}
			}
			else
			{
				_genBitSet(_pRoot, new BitSet());
			}
			_isGenerated = true;
		}

		virtual const BitSet &getCode(Unit data) const override
		{
			this->_ensureGenerated();
			return *((*_pUnit2BitSet)[data]);
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
			return this->_header.getString(*_pUnit2BitSet);
		}

		virtual double compressionRatio() const
		{
			this->_ensureGenerated();
			// TODO
			return 1;
		}

		uint8_t level() const
		{
			return this->_header.level;
		}

		void showCodes(size_t n = 0)
		{
			this->_ensureGenerated();
			if (!n)
				n = _pUnit2BitSet->size();
			size_t k = 0;
			for (auto it = _pUnit2BitSet->begin(); it != _pUnit2BitSet->end() && k != n; it++, k++)
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
		inline static const size_t _MAX_BUFF_SIZE = 4096;
		std::unique_ptr<NMap> _pUnit2Node;
		std::unique_ptr<BMap> _pUnit2BitSet;
		std::shared_ptr<Node> _pRoot;
		std::unique_ptr<Buff_t[]> _pBuffer;
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
			Unit *p = reinterpret_cast<Unit*>(_pBuffer.get());
			for (size_t i = 0; i != size; i++)
			{
				auto v = p[i];
				if (_pUnit2Node->find(v) == _pUnit2Node->end())
					(*_pUnit2Node)[v].reset(new Node(v));
				else
					(*_pUnit2Node)[v]->freq++;
			}
			auto rest = _bfsize - size * sizeof Unit;
			if (rest)
			{
				auto v = p[size] & ~((~static_cast<Unit>(0)) >> rest);
				if (_pUnit2Node->find(v) == _pUnit2Node->end())
					(*_pUnit2Node)[v].reset(new Node(v));
				else
					(*_pUnit2Node)[v]->freq++;
			}
			_bfsize = 0;
		}

		void _reset()
		{
			_pRoot = nullptr;
			_pUnit2Node->clear();
			_pUnit2BitSet->clear();
			_bfsize = 0;
			_isGenerated = false;
		}

		void _genBitSet(std::shared_ptr<Node> np, BitSet *bp)
		{
			while (np)
			{
				if (np->hasValue())
				{
					(*_pUnit2BitSet)[np->value()].reset(bp);
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
			size_t fsize = inf.tellg();
			inf.seekg(0);
			const size_t bfsize = 4096;
			std::unique_ptr<char[]> buffer(new char[bfsize]);
			const size_t cnt = fsize / bfsize;
			for (size_t i = 0; i != cnt; i++)
			{
				inf.read(buffer.get(), bfsize);
				_encoder.getData(buffer.get(), bfsize);
			}
			size_t rest = fsize - cnt * bfsize;
			inf.read(buffer.get(), rest);
			_encoder.getData(buffer.get(), rest);
			inf.close();
			_encoder.generate();
			//_encoder.showCodes(100);

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
