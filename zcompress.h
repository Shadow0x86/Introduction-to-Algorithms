#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "heap.h"


namespace lyf
{

	uint64_t fileSize(string filename)
	{
		std::ifstream inf(filename, std::ifstream::binary);
		if (!inf)
			throw std::runtime_error("The file does not exist");
		inf.seekg(0, std::ifstream::end);
		uint64_t ret = inf.tellg();
		inf.close();
		return ret;
	}

	uint64_t fileSize(std::ifstream &f)
	{
		if (!f)
			throw std::runtime_error("The file does not exist");
		auto x = f.tellg();
		f.seekg(0, std::fstream::end);
		uint64_t ret = f.tellg();
		f.seekg(x);
		return ret;
	}

	uint64_t fileSize(std::ofstream &f)
	{
		if (!f)
			throw std::runtime_error("The file does not exist");
		auto x = f.tellp();
		f.seekp(0, std::fstream::end);
		uint64_t ret = f.tellp();
		f.seekp(x);
		return ret;
	}

	template<typename Unit>
	class HuffmanEncoder
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
		using BitSet = vector<bool>;
		using Node = _HuffmanNode;
		using NMap = std::unordered_map<Unit, std::shared_ptr<Node>>;
		using BMap = std::unordered_map<Unit, std::unique_ptr<BitSet>>;
		using Buff_t = char;

	public:
		HuffmanEncoder()
			: _pUnit2Node(new NMap()), _pUnit2BitSet(new BMap()), _nDataTotalBytes(0),
			  _nCodeFillBits(0), _pHeader(nullptr), _pRoot(nullptr), _c(0), _pos(0),
			  _pBuffer(new Buff_t[_MAX_BUFF_SIZE]), _BuffSize(0)
		{
		}

		HuffmanEncoder(const HuffmanEncoder &) = delete;
		HuffmanEncoder &operator=(const HuffmanEncoder &) = delete;

		void encode(const string &srcfile, const string &dstfile)
		{
			std::ifstream inf(srcfile, std::ifstream::binary);
			if (!inf)
				throw std::runtime_error("The src file does not exist.");
			std::ofstream outf(dstfile, std::ofstream::binary);
			if (!outf)
				throw std::runtime_error("Fail to create or open the dst file.");
			
			this->encode(inf, outf);

			inf.close();
			outf.close();
		}

		void encode(std::istream &in, std::ostream &out)
		{
			this->reset();
			uint64_t start = in.tellg();
			in.seekg(0, std::ifstream::end);
			_nDataTotalBytes = static_cast<uint64_t>(in.tellg()) - start;
			in.seekg(start);
			const size_t cnt = _nDataTotalBytes / _MAX_BUFF_SIZE;
			const size_t rest = _nDataTotalBytes % _MAX_BUFF_SIZE;
			this->_readStream(in, cnt, rest, [&]() { this->_flushToTree(); });
			this->_generate();
			in.seekg(start);
			this->_readStream(in, cnt, rest, [&]() { this->_flushToStream(out); });
			if (_pos)
			{
				out.write(&_c, 1);
				_nCodeFillBits = 8 - _pos;
				_c = 0;
				_pos = 0;
			}
		}

		BitSet getCode(Unit data) const
		{
			return *((*_pUnit2BitSet)[data]);
		}

		string getHeader() const
		{
			return const_cast<HuffmanEncoder*>(this)->getHeader();
		}

		const string &getHeader()
		{
			if (!this->_pHeader)
			{
				this->_pHeader.reset(new string);
				auto cp = reinterpret_cast<const char*>(&_nDataTotalBytes);
				for (int i = 0; i != 8; i++)
					this->_pHeader->push_back(cp[i]);
				this->_pHeader->push_back(_nCodeFillBits);
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

		void reset()
		{
			_pRoot = nullptr;
			_pUnit2Node->clear();
			_pUnit2BitSet->clear();
			_BuffSize = 0;
			_pHeader.reset();
			_nDataTotalBytes = 0;
			_nCodeFillBits = 0;
		}

		void showCodes(size_t n = 0, std::ostream &out = cout)
		{
			if (!n)
				n = _pUnit2BitSet->size();
			size_t k = 0;
			for (auto it = _pUnit2BitSet->begin(); it != _pUnit2BitSet->end() && k != n; it++, k++)
			{
				out << it->first << ": ";
				for (auto i = it->second->begin(), end = it->second->end(); i != end; i++)
				{
					out << *i;
				}
				out << endl;
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
		uint64_t _nDataTotalBytes;
		char _nCodeFillBits;
		string _s;
		char _c;
		char _pos;

		template<typename Func>
		void _readStream(std::istream &in, size_t cnt, size_t rest, Func flush)
		{
			for (size_t i = 0; i != cnt; i++)
			{
				in.read(_pBuffer.get(), _MAX_BUFF_SIZE);
				_BuffSize = _MAX_BUFF_SIZE;
				flush();
			}
			if (rest)
			{
				in.read(_pBuffer.get(), rest);
				_BuffSize = rest;
				flush();
			}
		}

		void _generate()
		{
			this->_flushToTree();
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
		}

		void _flushToTree()
		{
			auto size = _BuffSize / sizeof Unit;
			auto rest = _BuffSize % sizeof Unit;
			if (rest)
			{
				char *cp = _pBuffer.get() + _BuffSize;
				for (size_t i = 0, len = sizeof Unit - rest; i != len; i++)
					cp[i] = 0;
				size++;
			}
			Unit *p = reinterpret_cast<Unit*>(_pBuffer.get());
			for (size_t i = 0; i != size; i++)
			{
				auto v = p[i];
				if (_pUnit2Node->find(v) == _pUnit2Node->end())
					(*_pUnit2Node)[v].reset(new Node(v));
				else
					(*_pUnit2Node)[v]->freq++;
			}
			_BuffSize = 0;
		}

		void _flushToStream(std::ostream &out)
		{
			auto size = _BuffSize / sizeof Unit;
			auto rest = _BuffSize % sizeof Unit;
			if (rest)
			{
				char *cp = _pBuffer.get() + _BuffSize;
				for (size_t i = 0, len = sizeof Unit - rest; i != len; i++)
					cp[i] = 0;
				size++;
			}
			Unit *p = reinterpret_cast<Unit*>(_pBuffer.get());
			for (size_t i = 0; i != size; i++)
			{
				const BitSet &bs = *((*_pUnit2BitSet)[p[i]]);
				for (auto it = bs.begin(); it != bs.end(); it++)
				{
					_c += ((*it) << (7 - _pos));
					if (++_pos == 8)
					{
						_pos = 0;
						_s.push_back(_c);
						_c = 0;
					}
				}
			}
			out.write(_s.data(), _s.size());
			_s.clear();
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


	class HuffmanDecoder
	{
	public:
		using BitSet = vector<bool>;
		using BMap = std::unordered_map<BitSet, string>;

		HuffmanDecoder()
			: _pBitSet2Data(nullptr), _nDataTotalBytes(0), _nRestBytes(0), _nCodeFillBits(0),
			  _nUnitBytes(0), _pBuffer(new char[_MAX_BUFF_SIZE]), _BuffSize(0)
		{
		}

		HuffmanDecoder(const string &header)
			: HuffmanDecoder()
		{
			this->setHeader(header);
		}

		void setHeader(const string &header)
		{
			_pBitSet2Data.reset(new BMap());
			_nDataTotalBytes = *reinterpret_cast<const uint64_t*>(header.data());
			_nCodeFillBits = header[8];
			_nUnitBytes = header[9];
			size_t pos = 10;
			while (pos != header.size())
			{
				size_t nCodeBits = 0;
				for (char j = 0; j != _nUnitBytes; j++)
				{
					auto c = static_cast<unsigned char>(header[pos + j]);
					nCodeBits += (c << ((_nUnitBytes - 1 - j) * 8));
				}
				pos += _nUnitBytes;
				BitSet bs;
				unsigned char flag = 128;
				for (size_t j = 0; j != nCodeBits; j++)
				{
					bs.push_back(header[pos] & flag);
					flag /= 2;
					if (flag == 0)
					{
						pos++;
						flag = 128;
					}
				}
				if (flag != 128)
					pos++;
				(*_pBitSet2Data)[bs] = header.substr(pos, _nUnitBytes);
				pos += _nUnitBytes;
			}
		}

		void decode(const string &src, const string &dst)
		{
			
			std::ifstream inf(src, std::ifstream::binary);
			if (!inf)
				throw std::runtime_error("The file does not exist");
			std::ofstream outf(dst, std::ofstream::binary);
			if (!outf)
				throw std::runtime_error("Fail to create or open the dst file");
			this->decode(inf, outf);
			inf.close();
			outf.close();
		}

		void decode(std::istream &in, std::ostream &out)
		{
			_nRestBytes = _nDataTotalBytes;
			uint64_t start = in.tellg();
			in.seekg(0, std::ifstream::end);
			uint64_t nSrcBytes = static_cast<uint64_t>(in.tellg()) - start;
			in.seekg(start);
			uint64_t nSrcBits = nSrcBytes * 8 - _nCodeFillBits;
			size_t cnt = nSrcBits / (_MAX_BUFF_SIZE * 8);
			for (size_t i = 0; i != cnt; i++)
			{
				in.read(_pBuffer.get(), _MAX_BUFF_SIZE);
				_BuffSize = _MAX_BUFF_SIZE;
				_flushToStream(out);
			}
			uint64_t restBits = nSrcBits % (_MAX_BUFF_SIZE * 8);
			if (restBits)
			{
				uint64_t restBytes = (restBits + 7) / 8;
				in.read(_pBuffer.get(), restBytes);
				_BuffSize = restBytes;
				_flushToStream(out, restBits);
			}
		}

		void showCodes(size_t n = 0, std::ostream &out = cout)
		{
			if (!n)
				n = _pBitSet2Data->size();
			size_t k = 0;
			for (auto it = _pBitSet2Data->begin(); it != _pBitSet2Data->end() && k != n; it++, k++)
			{
				for (auto i = it->first.begin(), end = it->first.end(); i != end; i++)
				{
					out << *i;
				}
				out << ": ";
				uint64_t v = 0;
				int j = 0;
				for (unsigned char s : it->second)
				{
					v += static_cast<uint64_t>(s) << (j * 8);
					j++;
				}
				out << v << endl;
			}
		}

	private:
		inline static size_t const _MAX_BUFF_SIZE = 4096;
		std::unique_ptr<BMap> _pBitSet2Data;
		uint64_t _nDataTotalBytes;
		uint64_t _nRestBytes;
		char _nCodeFillBits;
		char _nUnitBytes;
		std::unique_ptr<char[]> _pBuffer;
		size_t _BuffSize;
		BitSet _bs;

		void _flushToStream(std::ostream &out, uint64_t maxBits = 0)
		{
			if (!maxBits)
				maxBits = _BuffSize * 8;
			size_t i = 0;
			uint64_t bits = 0;
			unsigned char flag = 128;
			while (i != _BuffSize && bits != maxBits)
			{
				_bs.push_back(_pBuffer[i] & flag);
				if (_pBitSet2Data->find(_bs) != _pBitSet2Data->end())
				{
					const string &s = (*_pBitSet2Data)[_bs];
					uint64_t nw = std::min(_nRestBytes, static_cast<uint64_t>(_nUnitBytes));
					out.write(s.data(), nw);
					_nRestBytes -= nw;
					_bs.clear();
					if (!_nRestBytes)
						break;
				}
				bits++;
				flag /= 2;
				if (!flag)
				{
					flag = 128;
					i++;
				}
			}
			_BuffSize = 0;
		}
	};


	template<typename Encoder>
	class ZCompresser
	{
	public:
		template<typename... EncoderArgs>
		ZCompresser(EncoderArgs&&... args)
			: _pEncoder(new Encoder(std::forward<EncoderArgs>(args)...))
		{
		}

		void compress(const string &src, const string &dst = "", uint8_t level = 9)
		{
			if (level < 1 || level > 9)
				throw std::runtime_error("level must be between 1 and 9");
			string infilename = src, outfilename = ".encode1.tmp";
			std::ifstream in;
			std::ostringstream outs;
			std::ofstream outf;
			uint8_t lv = 1;
			uint64_t oldsize, newsize;
			vector<string> toremove;
			while (lv <= level)
			{
				in.open(infilename, std::ifstream::binary);
				oldsize = fileSize(in);
				outf.open(outfilename, std::ofstream::binary);
				_pEncoder->reset();
				_pEncoder->encode(in, outs);
				const string &header = _pEncoder->getHeader();
				outf.write(reinterpret_cast<char*>(&lv), 1);
				unsigned int hsize = header.size();
				outf.write(reinterpret_cast<char*>(&hsize), 4);
				outf.write(header.data(), header.size());
				outf.write(outs.str().data(), outs.str().size());
				newsize = fileSize(outf);
				if (newsize >= oldsize)
				{
					if (infilename != src)
					{
						toremove.push_back(outfilename);
						outfilename = infilename;
					}
					in.close();
					outf.close();
					break;
				}
				if (infilename != src)
					toremove.push_back(infilename);
				infilename = outfilename;
				outfilename.replace(7, 1, 1, ++lv + 48);
				in.clear();
				in.close();
				outf.clear();
				outf.close();
				outs.clear();
				outs.str("");
			}
			toremove.push_back(dst);
			for (auto &f : toremove)
				remove(f.c_str());
			rename(outfilename.c_str(), dst.c_str());
		}

		inline static const uint8_t MAX_COMPRESSION_LEVEL = 9;

	private:
		std::unique_ptr<Encoder> _pEncoder;
	};

	using HuffmanCompresser = ZCompresser<HuffmanEncoder<uint64_t>>;


	template<typename Decoder>
	class ZDecompresser
	{
	public:
		template<typename... DecoderArgs>
		ZDecompresser(DecoderArgs&&... args)
			: _pDecoder(new Decoder(std::forward<DecoderArgs>(args)...))
		{
		}

		void decompress(const string &src, const string &dst)
		{
			string infilename = src, outfilename = ".decode1.tmp";
			std::ifstream in;
			std::ofstream outf;
			vector<string> toremove;
			char lv, i = 1;
			unsigned int hsize;
			while (true)
			{
				in.open(infilename, std::ifstream::binary);
				outf.open(outfilename, std::ofstream::binary);
				in.read(&lv, 1);
				in.read(reinterpret_cast<char*>(&hsize), 4);
				string header;
				header.resize(hsize);
				in.read(header.data(), hsize);
				_pDecoder->setHeader(header);
				_pDecoder->decode(in, outf);
				in.clear();
				in.close();
				outf.clear();
				outf.close();
				if (infilename != src)
					toremove.push_back(infilename);
				if (lv == 1)
					break;
				infilename = outfilename;
				outfilename.replace(7, 1, 1, ++i + 48);
			}
			toremove.push_back(dst);
			for (auto &f : toremove)
				remove(f.c_str());
			rename(outfilename.c_str(), dst.c_str());
		}

	private:
		std::unique_ptr<Decoder> _pDecoder;
	};

	using HuffmanDecompresser = ZDecompresser<HuffmanDecoder>;
}
