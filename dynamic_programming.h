#pragma once
#include "utils.h"


namespace lyf
{
	template<typename Values>
	auto memoized_cut_rod(const Values &values, size_t n)
	{
		using t = typename array_traits<Values>::value_type;
		t *r = new t[n + 1];
		for (auto i = 0; i != n; i++)
		{
			r[i] = std::numeric_limits<t>::lowest();
		}
		auto ret = memoized_cut_rod_aux(values, n, r);
		delete[] r;
		return ret;
	}

	template<typename Values, typename Val>
	Val memoized_cut_rod_aux(const Values &values, size_t n, Val *r)
	{
		if (r[n] < 0)
		{
			Val q = 0;
			if (n)
			{
				for (auto i = 1; i <= n; i++)
				{
					q = std::max(q, values[i] + memoized_cut_rod_aux(values, n - i, r));
				}
			}
			r[n] = q;
		}
		return r[n];
	}

	template<typename Values, typename Val = size_t>
	auto bottom_up_cut_rod(const Values &values, size_t n, Val c = 0U)
	{
		using t = typename array_traits<Values>::value_type;
		auto *r = new t[n + 1];
		auto *s = new size_t[n + 1];
		r[0] = 0;
		for (auto i = 1; i <= n; i++)
		{
			t q = std::numeric_limits<t>::lowest();
			for (auto j = i; j > 0; j--)
			{
				t nv = r[i - j] + values[j] - (j == i ? 0 : c);
				if (q < nv)
				{
					q = nv;
					s[i] = j;
				}
			}
			r[i] = q;
		}
		auto v = r[n];
		delete[] r;
		vector<size_t> l;
		while (n > 0)
		{
			l.push_back(s[n]);
			n -= s[n];
		}
		delete[] s;
		return std::make_pair(v, l);
	}

	template<typename Seq1, typename Seq2>
	auto longest_common_subsequence(const Seq1 &seq1, size_t len1, const Seq2 &seq2, size_t len2)
	{
		using value_type = typename array_traits<Seq1>::value_type;
		vector<value_type> ret;
		if (len1 == 0 || len2 == 0)
			return ret;
		size_t nrows = len1 + 1, ncols = len2 + 1;
		auto *c = new size_t[nrows*ncols];
		for (size_t i = 0; i != ncols; i++)
			c[i] = 0;
		for (size_t i = 0; i != nrows; i++)
			c[ncols*i] = 0;
		for (auto i = 0; i != len1; i++)
		{
			for (auto j = 0; j != len2; j++)
			{
				if (seq1[i] == seq2[j])
					c[(i + 1)*ncols + j + 1] = c[i*ncols + j] + 1;
				else if (c[i*ncols + j + 1] >= c[(i + 1)*ncols + j])
					c[(i + 1)*ncols + j + 1] = c[i*ncols + j + 1];
				else
					c[(i + 1)*ncols + j + 1] = c[(i + 1)*ncols + j];
			}
		}

		size_t i = len1, j = len2;
		while (i > 0 && j > 0)
		{
			auto v = c[i*ncols + j];
			if (v == c[(i - 1)*ncols + j])
				i--;
			else if (v == c[i*ncols + j - 1])
				j--;
			else
			{
				ret.push_back(seq1[i - 1]);
				i--; j--;
			}
		}
		//cout << "    ";
		//for (auto i = 0; i != len2; i++)
		//	cout << seq2[i] << " ";
		//cout << endl;
		//for (auto i = 0; i != nrows; i++)
		//{
		//	if (i == 0)
		//		cout << "  ";
		//	else
		//		cout << seq1[i - 1] << " ";
		//	for (auto j = 0; j != ncols; j++)
		//	{
		//		cout << c[i*ncols + j] << " ";
		//	}
		//	cout << endl;
		//}

		delete[] c;
		std::reverse(ret.begin(), ret.end());
		return ret;
	}

	template<typename Seq1, typename Seq2>
	auto longest_common_substring(const Seq1 &seq1, size_t len1, const Seq2 &seq2, size_t len2)
	{
		using value_type = typename array_traits<Seq1>::value_type;
		size_t nrows = len1 + 1, ncols = len2 + 1;
		auto *c = new size_t[nrows*ncols];
		for (size_t i = 0; i != ncols; i++)
			c[i] = 0;
		for (size_t i = 0; i != nrows; i++)
			c[ncols*i] = 0;
		size_t longest = 0;
		size_t index1 = 0;
		for (auto i = 0; i != len1; i++)
		{
			for (auto j = 0; j != len2; j++)
			{
				auto k = (i + 1)*ncols + j + 1;
				if (seq1[i] == seq2[j])
				{
					c[k] = c[i*ncols + j] + 1;
					if (c[k] > longest)
					{
						longest = c[k];
						index1 = i;
					}
				}
				else
					c[k] = 0;
			}
		}
		delete[] c;
		vector<value_type> ret(longest);
		size_t delta = index1 - longest + 1;
		for (size_t i = 0; i != longest; i++)
			ret[i] = seq1[i + delta];
		return ret;
	}

	template<typename Seq>
	auto longest_palindrome(const Seq &seq, size_t len)
	{
		using value_type = typename array_traits<Seq>::value_type;
		vector<value_type> rseq(len);
		for (size_t i = 0; i != len; i++)
			rseq[i] = seq[len - i - 1];
		return longest_common_subsequence(seq, len, rseq, len);
	}

	// get the longest increasing subsequence of the given sequence in O(n²) time.
	template<typename Seq>
	auto longest_increasing_subsequence_On2(const Seq &seq, size_t len)
	{
		using value_type = typename array_traits<Seq>::value_type;
		vector<value_type> ret;
		if (!len)
			return ret;
		size_t *c = new size_t[len];
		c[0] = 1;
		size_t last = 0;
		for (size_t i = 1; i != len; i++)
		{
			c[i] = 1;
			for (size_t j = 0; j != i; j++)
			{
				if (seq[j] <= seq[i])
				{
					c[i] = std::max(c[i], c[j] + 1);
				}
			}
			if (c[i] > c[last])
			{
				last = i;
			}
		}
		ret.resize(c[last]);
		size_t index = last;
		size_t i = index, j = c[last] - 1;
		ret[j] = seq[index];
		while (i-- > 0)
		{
			if ((c[i] == c[index] - 1) && (seq[i] <= seq[index]))
			{
				index = i;
				ret[--j] = seq[index];
			}
		}
		delete[] c;
		return ret;
	}

	// get the longest increasing subsequence of the given sequence in O(nlgn) time.
	template<typename Seq>
	auto longest_increasing_subsequence(const Seq &seq, size_t len)
	{
		using value_type = typename array_traits<Seq>::value_type;
		vector<value_type> ret;
		if (!len)
			return ret;
		size_t *c = new size_t[len];
		c[0] = 1;
		size_t last = 0;
		auto dp = new value_type[len];
		dp[0] = seq[0];
		size_t right = 1;
		for (size_t i = 0; i != len; i++)
		{
			size_t l = 0, r=right;
			while (l < r)
			{
				size_t mid = (l + r) / 2;
				if (seq[i] >= dp[mid])
					l = mid + 1;
				else
					r = mid;
			}
			right = std::max(l + 1, right);
			dp[l] = seq[i];
			c[i] = l + 1;
			if (c[i] > c[last])
			{
				last = i;
			}
		}
		delete[] dp;
		ret.resize(c[last]);
		size_t index = last;
		size_t i = index, j = c[last] - 1;
		ret[j] = seq[index];
		while (i-- > 0)
		{
			if ((c[i] == c[index] - 1) && (seq[i] <= seq[index]))
			{
				index = i;
				ret[--j] = seq[index];
			}
		}
		delete[] c;
		return ret;
	}
}
