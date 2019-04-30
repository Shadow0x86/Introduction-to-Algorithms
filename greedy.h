#pragma once
#include "utils.h"


namespace lyf
{
	template<typename Iter>
	auto greedy_activity_selector(const Iter begin, const Iter end, bool sort = false)
	{
		using value_type = typename Iter_traits<Iter>::value_type;
		vector<value_type> ret;
		if (begin >= end)
			return ret;
		if (sort)
			std::sort(begin, end, [](auto &a, auto &b) { return a.second < b.second; });
		auto it = begin, m = it;
		ret.push_back(*it);
		while (++it < end)
		{
			if (it->first >= m->second)
			{
				ret.push_back(*it);
				m = it;
			}
		}
		return ret;
	}
}
