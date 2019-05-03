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
		void getData(void *pdata, size_t size) = 0;
		void generate() = 0;
		std::pair<void*, size_t> getCode(Unit data) = 0;
	};

	template<typename Unit>
	class HuffmanTree : public ZEncoderInterface<Unit>
	{
	public:
		using BitSet = std::vector<bool>;

		HuffmanTree();

		void getData(void *pdata, size_t size) override
		{

		}

		void generate() override
		{

		}

		std::pair<void*, size_t> getCode(Unit data) override
		{

		}

	private:
		std::unordered_map<Unit, BitSet> _map;
	};


	class _BaseCompresser
	{
	public:
		using BitSet = std::vector<bool>;
	private:
		static const char * const _FilenameSuffix;
	};

	const char * const _BaseCompresser::_FilenameSuffix = ".zc";


	template<typename Encoder>
	class ZCompresser : public _BaseCompresser
	{
	public:
		ZCompresser() {}

		void compress(string file, uint8_t level = 9);
		double compressRate();

	private:
		static const double _NoDeepFactor = 0.95;
	};

	using HuffmanCompresser = ZCompresser<HuffmanTree<uint16_t>>;


	class ZDecompresser : public _BaseCompresser
	{
		void decompress(string file);
	};
}
