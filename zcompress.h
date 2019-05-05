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

		virtual void getData(void *, size_t) = 0;
		virtual void generate() = 0;
		virtual BitSet getCode(Unit) const = 0;
		virtual void reset() = 0;
		virtual string getHeader() const = 0;
		virtual double CompressionRate() const = 0;
		virtual bool isGenerated() const = 0;
	};


	template<typename Unit>
	class HuffmanEncoder : public ZEncoderInterface<Unit>
	{
	private:
		struct _HuffmanNode
		{
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
			uint64_t freq = 1;
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

	public:
		HuffmanEncoder()
			: _pUnit2Node(new NMap()), _pUnit2BitSet(new BMap()), _FileTotalSize(0),
			  _isGenerated(false), _nDataFillBytes(0), _CompressionRate(-1), _pHeader(nullptr),
			  _pRoot(nullptr), _pBuffer(new Buff_t[_MAX_BUFF_SIZE]), _BuffSize(0)
		{
		}

		HuffmanEncoder(const HuffmanEncoder &) = delete;
		HuffmanEncoder &operator=(const HuffmanEncoder &) = delete;

		virtual void getData(void *pdata, size_t nBytes) override
		{
			this->_ensureNotGenerated();
			char *p = reinterpret_cast<char*>(pdata);
			while (nBytes)
			{
				size_t nw = std::min(nBytes, _MAX_BUFF_SIZE - _BuffSize);
				memcpy(_pBuffer.get() + _BuffSize, p, nw);
				p += nw;
				_BuffSize += nw;
				if (_BuffSize >= _MAX_BUFF_SIZE)
					_flush();
				nBytes -= nw;
			}
		}

		virtual void generate() override
		{
			this->_ensureNotGenerated();
			this->_flush();
			if (_pUnit2Node->size())
			{
				auto heap = lyf::newMinPriorityQueue<std::shared_ptr<Node>>([](auto e) { return e->freq; });
				for (auto it = _pUnit2Node->begin(); it != _pUnit2Node->end(); it++)
				{
					heap.insert(it->second);
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
					(*_pUnit2BitSet)[_pRoot->value()].reset(new BitSet{ 0 });;
				}
				else
				{
					_genBitSet(_pRoot, new BitSet());
				}
			}
			_isGenerated = true;
		}

		virtual BitSet getCode(Unit data) const override
		{
			this->_ensureGenerated();
			return *((*_pUnit2BitSet)[data]);
		}

		virtual string getHeader() const override
		{
			return const_cast<HuffmanEncoder*>(this)->getHeader();
		}

		string getHeader()
		{
			this->_ensureGenerated();
			if (!this->_pHeader)
			{
				this->_pHeader.reset(new string);
				this->_pHeader->push_back(_nDataFillBytes);
				char nDataUnitBytes = sizeof Unit;
				this->_pHeader->push_back(nDataUnitBytes);
				for (auto it = _pUnit2BitSet->begin(); it != _pUnit2BitSet->end(); it++)
				{
					size_t nCodeBits = it->second->size();
					for (char i = 0; i != nDataUnitBytes; i++)
					{
						char c = nCodeBits >> ((nDataUnitBytes - 1 - i) * 8);
						this->_pHeader->push_back(c);
					}
					char pos = 0, c = 0;
					for (auto i = it->second->begin(); i != it->second->end(); i++)
					{
						c += ((*i) << (7 - pos));
						if (++pos == 8)
						{
							pos = 0;
							this->_pHeader->push_back(c);
							c = 0;
						}
					}
					if (pos)
						this->_pHeader->push_back(c);
					auto cp = reinterpret_cast<const char*>(&(it->first));
					for (char i = 0; i != sizeof Unit; i++)
						this->_pHeader->push_back(cp[i]);
				}
			}
			return *(this->_pHeader);
		}

		virtual double CompressionRate() const override
		{
			return const_cast<HuffmanEncoder*>(this)->CompressionRate();
		}

		double CompressionRate()
		{
			this->_ensureGenerated();
			if (this->_CompressionRate < 0)
			{
				uint64_t compressedSize = 0;
				for (auto it = _pUnit2Node->begin(); it != _pUnit2Node->end(); it++)
				{
					compressedSize += (it->second->freq * (*_pUnit2BitSet)[it->first]->size());
				}
				this->_CompressionRate = static_cast<double>(compressedSize) / (this->_FileTotalSize * 8);
			}
			return this->_CompressionRate;
		}

		virtual bool isGenerated() const override
		{
			return this->_isGenerated;
		}

		virtual void reset() override
		{
			_pRoot = nullptr;
			_pUnit2Node->clear();
			_pUnit2BitSet->clear();
			_BuffSize = 0;
			_pHeader.reset();
			_nDataFillBytes = 0;
			_FileTotalSize = 0;
			_CompressionRate = -1;
			_isGenerated = false;
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

	private:
		inline static const size_t _MAX_BUFF_SIZE = 4096;
		std::unique_ptr<NMap> _pUnit2Node;
		std::unique_ptr<BMap> _pUnit2BitSet;
		std::shared_ptr<Node> _pRoot;
		std::unique_ptr<Buff_t[]> _pBuffer;
		size_t _BuffSize;
		std::unique_ptr<string> _pHeader;
		char _nDataFillBytes;
		uint64_t _FileTotalSize;
		double _CompressionRate;
		bool _isGenerated;

		void _ensureGenerated() const
		{
			if (!_isGenerated)
				throw std::runtime_error("Must call generate() first!");
		}

		void _ensureNotGenerated() const
		{
			if (_isGenerated)
				throw std::runtime_error("The encoder has been generated. Call reset() to reset it.");
		}

		void _flush()
		{
			auto size = _BuffSize / sizeof Unit;
			Unit *p = reinterpret_cast<Unit*>(_pBuffer.get());
			for (size_t i = 0; i != size; i++)
			{
				auto v = p[i];
				if (_pUnit2Node->find(v) == _pUnit2Node->end())
					(*_pUnit2Node)[v].reset(new Node(v));
				else
					(*_pUnit2Node)[v]->freq++;
			}
			auto rest = _BuffSize - size * sizeof Unit;
			if (rest)
			{
				char *cp = reinterpret_cast<char*>(p + size);
				Unit v = 0;
				for (size_t i = 0; i != rest; i++)
				{
					Unit d = cp[i];
					v += (d << ((sizeof Unit - 1 - i) * 8));
				}
				if (_pUnit2Node->find(v) == _pUnit2Node->end())
					(*_pUnit2Node)[v].reset(new Node(v));
				else
					(*_pUnit2Node)[v]->freq++;
				_nDataFillBytes = sizeof Unit - rest;
			}
			_FileTotalSize += _BuffSize;
			_BuffSize = 0;
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
		template<typename... EncoderArgs>
		ZCompresser(EncoderArgs&&... args)
			: _pEncoder(new Encoder(std::forward<EncoderArgs>(args)...))
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
				_pEncoder->getData(buffer.get(), bfsize);
			}
			size_t rest = fsize - cnt * bfsize;
			inf.read(buffer.get(), rest);
			_pEncoder->getData(buffer.get(), rest);
			inf.close();
			_pEncoder->generate();
			_pEncoder->showCodes(100);
			cout << _pEncoder->CompressionRate() << endl;
			
			string header = _pEncoder->getHeader();
			cout << header.size() << endl;

		}

		inline static const uint8_t MAX_COMPRESSION_LEVEL = 9;

	private:
		inline static const double _NoDeepFactor = 0.98;
		std::unique_ptr<Encoder> _pEncoder;
	};

	using HuffmanCompresser = ZCompresser<HuffmanEncoder<uint16_t>>;


	class ZDecompresser : public _BaseZCompresser
	{
		void decompress(string file);
	};
}
