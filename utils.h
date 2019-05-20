#pragma once
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <chrono>
#include <utility>
#include <random>
#include <filesystem>


#define INLINE inline
#define MOVE std::move
//#define LOG_CONST_ASSIGN


using namespace std::chrono;



namespace lyf
{
	using std::cout;
	using std::cin;
	using std::endl;
	using std::string;


	template<typename Iter>
	INLINE bool _isValidRange(Iter begin, Iter end)
	{
		return begin < end;
	}

	template<typename Iter>
	INLINE void _ensureValidRange(Iter begin, Iter end)
	{
#if _DEBUG
		assert(begin < end);
#endif
	}

	template<typename Key, typename T>
	struct _Predicate
	{
		INLINE static auto get(Key key, const T &v) -> decltype(key(v))
		{
			return key(v);
		}
	};

	template<typename T>
	struct _Predicate<void*, T>
	{
		INLINE static const T &get(void* key, const T &v)
		{
			return v;
		}
	};


	template<typename LIter, typename RIter>
	INLINE void swap_by_iter(LIter it1, RIter it2)
	{
		auto tmp = MOVE(*it1);
		*it1 = MOVE(*it2);
		*it2 = MOVE(tmp);
	}



	template<typename T>
	class ReversePtr
	{	// Reverse pointer class
		template<typename T>
		friend ReversePtr<T> rpbegin(T *begin, T *end);
		template<typename T>
		friend ReversePtr<T> rpend(T *begin, T *end);
		template<typename T>
		friend ReversePtr<const T> crpbegin(T *begin, T *end);
		template<typename T>
		friend ReversePtr<const T> crpend(T *begin, T *end);

	public:
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using reference_type = value_type & ;
		using const_reference_type = const value_type&;
		using const_iterator = ReversePtr<const value_type>;

		INLINE friend std::ostream &operator<<(std::ostream &out, const ReversePtr RPtr)
		{
			out << RPtr.current;
			return out;
		}
		INLINE friend difference_type operator-(const ReversePtr &lhs, const ReversePtr &rhs)
		{
			return rhs.current - lhs.current;
		}

		ReversePtr(const ReversePtr &rhs)
			:
#if _DEBUG
			_rbegin(rhs._rbegin), _rend(rhs._rend), 
#endif
			current(rhs.current)
		{

		}
		INLINE ReversePtr &operator=(const ReversePtr &rhs)
		{
#if _DEBUG
			_rbegin = rhs._rbegin;
			_rend = rhs._rend;
#endif
			current = rhs.current;
			return *this;
		}

		INLINE ReversePtr &operator++()
		{
			--current;
			_ensureValidCurrent();
			return *this;
		}
		INLINE ReversePtr operator++(int)
		{
			auto tmp = *this;
			++*this;
			return tmp;
		}
		INLINE ReversePtr &operator--()
		{
			++current;
			_ensureValidCurrent();
			return *this;
		}
		INLINE ReversePtr operator--(int)
		{
			auto tmp = *this;
			--*this;
			return tmp;
		}
		INLINE ReversePtr &operator+=(difference_type off)
		{
			current -= off;
			_ensureValidCurrent();
			return *this;
		}
		INLINE ReversePtr operator+(difference_type off) const
		{
			auto tmp = *this;
			return (tmp += off);
		}
		INLINE ReversePtr &operator-=(difference_type off)
		{
			current += off;
			_ensureValidCurrent();
			return *this;
		}
		INLINE ReversePtr operator-(difference_type off) const
		{
			auto tmp = *this;
			return (tmp -= off);
		}
		INLINE bool operator<(const ReversePtr &rhs) const
		{
			return rhs.current < current;
		}
		INLINE bool operator<=(const ReversePtr &rhs) const
		{
			return rhs.current <= current;
		}
		INLINE bool operator>(const ReversePtr &rhs) const
		{
			return !(*this <= rhs);
		}
		INLINE bool operator>=(const ReversePtr &rhs) const
		{
			return !(*this < rhs);
		}
		INLINE bool operator==(const ReversePtr &rhs) const
		{
			return current == rhs.current;
		}
		INLINE bool operator!=(const ReversePtr &rhs) const
		{
			return !(*this == rhs);
		}
		INLINE reference_type operator*() const
		{
			_ensureDerefCurrent();
			return *current;
		}
		INLINE value_type *operator->() const
		{
			_ensureDerefCurrent();
			return current;
		}
		INLINE reference_type operator[](size_t i) const
		{
#if _DEBUG
			assert(current - i <= _rbegin && current - i > _rend);
#endif
			return *(current - i);
		}
	protected:
		ReversePtr() {}
		ReversePtr(T *begin, T *end)
#if _DEBUG
			: _rbegin(end - 1), _rend(begin - 1)
#endif
		{
		}

		INLINE void _ensureValidCurrent() const
		{
#if _DEBUG
			assert(_rbegin >= current && current >= _rend);
#endif
		}
		INLINE void _ensureDerefCurrent() const
		{
#if _DEBUG
			assert(_rbegin >= current && current > _rend);
#endif
		}

		value_type* current = nullptr;
#if _DEBUG
		value_type* _rbegin = nullptr;
		value_type* _rend = nullptr;
#endif
	};


	template<typename T>
	ReversePtr<T> rpbegin(T *begin, T *end)
	{
		_ensureValidRange(begin, end);
		auto ret = ReversePtr<T>(begin, end);
		ret.current = end - 1;
		return ret;
	}
	template<typename T>
	ReversePtr<T> rpend(T *begin, T *end)
	{
		_ensureValidRange(begin, end);
		auto ret = ReversePtr<T>(begin, end);
		ret.current = begin - 1;
		return ret;
	}
	template<typename T>
	ReversePtr<const T> crpbegin(T *begin, T *end)
	{
		return rpbegin<const T>(begin, end);
	}
	template<typename T>
	ReversePtr<const T> crpend(T *begin, T *end)
	{
		return rpend<const T>(begin, end);
	}

	template<typename T>
	INLINE ReversePtr<T> operator+(std::ptrdiff_t off, ReversePtr<T> RPtr)
	{
		return RPtr += off;
	}



	template<typename Iter>
	struct Iter_traits
	{
		using value_type = typename Iter::value_type;
		using difference_type = typename Iter::difference_type;
		using reference_type = value_type & ;
		using const_reference_type = const value_type &;
		//using const_iterator = typename Iter::const_iterator;
		using reverse_iterator = std::reverse_iterator<Iter>;
		//using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static reverse_iterator rbegin(Iter begin, Iter end)
		{
			return reverse_iterator(end);
		}
		static reverse_iterator rend(Iter begin, Iter end)
		{
			return reverse_iterator(begin);
		}
		//static const_reverse_iterator crbegin(Iter begin, Iter end)
		//{
		//	return const_reverse_iterator(end);
		//}
		//static const_reverse_iterator crend(Iter begin, Iter end)
		//{
		//	return const_reverse_iterator(begin);
		//}
	};


	template<typename T>
	struct Iter_traits<ReversePtr<T>>
	{
		using value_type = typename ReversePtr<T>::value_type;
		using difference_type = typename ReversePtr<T>::difference_type;
		using reference_type = value_type & ;
		using const_reference_type = const value_type &;
		using const_iterator = typename ReversePtr<T>::const_iterator;
	};

	template<typename T>
	struct Iter_traits<T*>
	{
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using reference_type = value_type & ;
		using const_reference_type = const value_type &;
		using const_iterator = const value_type*;
		using reverse_iterator = ReversePtr<value_type>;
		using const_reverse_iterator = ReversePtr<const value_type>;

		static reverse_iterator rbegin(T* begin, T* end)
		{
			return rpbegin(begin, end);
		}
		static reverse_iterator rend(T* begin, T* end)
		{
			return rpend(begin, end);
		}
		static const_reverse_iterator crbegin(T* begin, T* end)
		{
			return crpbegin(begin, end);
		}
		static const_reverse_iterator crend(T* begin, T* end)
		{
			return crpend(begin, end);
		}
	};

	template<typename Array_t>
	struct array_traits
	{
		using value_type = typename Array_t::value_type;
	};

	template<typename Valt>
	struct array_traits<Valt*>
	{
		using value_type = Valt;
	};

	template<typename Valt, unsigned N>
	struct array_traits<Valt[N]>
	{
		using value_type = Valt;
	};

	
	template<typename Iter>
	auto rbegin(Iter begin, Iter end)
	{
		return Iter_traits<Iter>::rbegin(begin, end);
	}
	template<typename Iter>
	auto rend(Iter begin, Iter end)
	{
		return Iter_traits<Iter>::rend(begin, end);
	}
	template<typename Iter>
	auto crbegin(Iter begin, Iter end)
	{
		return Iter_traits<Iter>::crbegin(begin, end);
	}
	template<typename Iter>
	auto crend(Iter begin, Iter end)
	{
		return Iter_traits<Iter>::crend(begin, end);
	}
	template<typename Iter>
	auto riters(Iter begin, Iter end)
	{
		using T = Iter_traits<Iter>;
		return std::make_pair(T::rbegin(begin, end), T::rend(begin, end));
	}
	template<typename Iter>
	auto criters(Iter begin, Iter end)
	{
		using T = Iter_traits<Iter>;
		return std::make_pair(T::crbegin(begin, end), T::crend(begin, end));
	}


	template<typename Iter, typename Key = void*>
	INLINE bool iter_less(Iter left, Iter right, Key key = nullptr)
	{
		using K = _Predicate<Key, typename Iter_traits<Iter>::value_type>;
		return K::get(key, *left) < K::get(key, *right);
	}

	template<typename Iter, typename Key = void*>
	INLINE bool iter_greater(Iter left, Iter right, Key key = nullptr)
	{
		using K = _Predicate<Key, typename Iter_traits<Iter>::value_type>;
		return K::get(key, *right) < K::get(key, *left);
	}

	template<typename Iter, typename Key = void*>
	INLINE bool iter_equal(Iter left, Iter right, Key key = nullptr)
	{
		using K = _Predicate<Key, typename Iter_traits<Iter>::value_type>;
		return K::get(key, *left) == K::get(key, *right);
	}

	

	int randint(int max = INT_MAX)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<int> rnd;
		return rnd(gen) % max;
	}

	string randstring(size_t min_len = 1, size_t max_len = 40)
	{
		size_t len = randint(max_len - min_len + 1) + min_len;
		char *cp = new char[len + 1];
		for (size_t i = 0; i != len; i++)
			cp[i] = randint(127 - 33) + 33;
		cp[len] = 0;
		string s(cp);
		delete[] cp;
		return s;
	}

	

	class TestClass
	{
#ifdef LOG_CONST_ASSIGN
	private:
		static int cnt_copy_constructor;
		static int cnt_move_constructor;
		static int cnt_copy_assign;
		static int cnt_move_assign;

	public:
		static void reset_count()
		{
			cnt_copy_constructor = 0;
			cnt_move_constructor = 0;
			cnt_copy_assign = 0;
			cnt_move_assign = 0;
		}
		static void show_count()
		{
			cout << cnt_copy_constructor << endl;
			cout << cnt_move_constructor << endl;
			cout << cnt_copy_assign << endl;
			cout << cnt_move_assign << endl;
		}
#endif

	public:
		TestClass() : name(nullptr) {}
		explicit TestClass(const string &name)
			:name(new string(name))
		{
			//cout << "construct " << name << endl;
		}
		~TestClass()
		{
			//if (name)
			//	cout << "destroy " << *name << endl;
			delete name;
		}
		TestClass(const TestClass &rhs)
			: name(new string(*rhs.name))
		{
#ifdef LOG_CONST_ASSIGN
			cnt_copy_constructor++;
#endif
		}
		TestClass(TestClass &&rhs)
			: name(rhs.name)
		{
#ifdef LOG_CONST_ASSIGN
			cnt_move_constructor++;
#endif
			rhs.name = nullptr;
		}

		TestClass &operator=(const TestClass &rhs)
		{
#ifdef LOG_CONST_ASSIGN
			cnt_copy_assign++;
#endif
			if (&rhs != this)
			{
				delete name;
				name = new string(*rhs.name);
			}
			return *this;
		}
		TestClass &operator=(TestClass &&rhs)
		{
#ifdef LOG_CONST_ASSIGN
			cnt_move_assign++;
#endif
			if (&rhs != this)
			{
				delete name;
				name = rhs.name;
				rhs.name = nullptr;
			}
			return *this;
		}

		bool operator<(const TestClass &rhs) const
		{
			return (*name) < (*rhs.name);
		}
		bool operator<=(const TestClass &rhs) const
		{
			return (*name) <= (*rhs.name);
		}
		bool operator>(const TestClass &rhs) const
		{
			return rhs < *this;
		}
		bool operator>=(const TestClass &rhs) const
		{
			return rhs <= *this;
		}
		bool operator==(const TestClass &rhs) const
		{
			return *name == *rhs.name;
		}
		bool operator!=(const TestClass &rhs) const
		{
			return !(*this == rhs);
		}

		string getName() const
		{
			return *name;
		}

	private:
		string * name = nullptr;
	};

#ifdef LOG_CONST_ASSIGN
	int TestClass::cnt_copy_constructor = 0;
	int TestClass::cnt_move_constructor = 0;
	int TestClass::cnt_copy_assign = 0;
	int TestClass::cnt_move_assign = 0;
#endif

	std::ostream &operator<<(std::ostream &out, const TestClass &p)
	{
		out << "TestClass(" << p.getName() << ")";
		return out;
	}


	template<typename T>
	struct TestType;

	template<>
	struct TestType<int>
	{
		static int newObj() { return randint(); }
	};

	template<>
	struct TestType<long long>
	{
		static long long newObj() { return randint(); }
	};

	template<>
	struct TestType<TestClass>
	{
		static TestClass newObj() { return TestClass(randstring()); }
	};


	class _CheckedNode
	{
	protected:
		_CheckedNode() {}

		_CheckedNode(void *pCont)
#if _DEBUG
			:_pCont(pCont)
#endif
		{
		}

		_CheckedNode(const _CheckedNode &rhs)
#if _DEBUG
			:_pCont()
#endif
		{
		}

		_CheckedNode &operator=(const _CheckedNode &rhs)
		{
			return *this;
		}

		void _setCont(void *pCont = nullptr)
		{
#if _DEBUG
			_pCont = pCont;
#endif
		}
		void _ensureInCont() const
		{
#if _DEBUG
			if (!_pCont)
				throw std::runtime_error("The node is outside range!");
#endif
		}
		void _ensureInCont(const void *pCont) const
		{
#if _DEBUG
			if (_pCont != pCont)
				throw std::runtime_error("The node is outside range!");
#endif
		}

	private:
#if _DEBUG
		void *_pCont = nullptr;
#endif

	};


	template<typename Node>
	class _CheckedNodeContainer
	{
	public:
#if _DEBUG
		using nodeptr = std::shared_ptr<Node>;
#else
		using nodeptr = Node * ;
#endif

		static nodeptr _new_node(Node *pNode)
		{
#if _DEBUG
			return nodeptr(pNode);
#else
			return pNode;
#endif
		}

		static void _assign_node(nodeptr &ln, Node *pNode)
		{
#if _DEBUG
			ln.reset(pNode);
#else
			ln = pNode;
#endif
		}

		static void _delete_node(nodeptr np)
		{
#if _DEBUG
			np->_reset_neighbors();
			np->_setCont();
			np._reset();
#else
			delete np;
#endif
		}

		static void _set_out(nodeptr np)
		{
			np->_setCont();
		}
	};


	template<typename Valt>
	class SharedNode : public _CheckedNode
	{
	public:
		using _MyBase = _CheckedNode;
		using value_type = Valt;

		value_type &value()
		{
			_ensureInCont();
			return *_pVal;
		}

	protected:
		SharedNode() {}

		SharedNode(const SharedNode &rhs)
			:_MyBase(rhs), _pVal(rhs._pVal)
		{
		}

		SharedNode &operator=(const SharedNode &rhs)
		{
			if (this != &rhs)
			{
				_MyBase::operator=(rhs);
				_pVal = rhs._pVal;
			}
			return *this;
		}

		SharedNode(void *pCont, const Valt &value)
			:_MyBase(pCont), _pVal(new Valt(value))
		{
		}

		SharedNode(void *pCont, Valt &&value)
			:_MyBase(pCont), _pVal(new Valt(std::move(value)))
		{
		}

		template<typename... Types>
		SharedNode(void *pCont, Types&&... args)
			:_MyBase(pCont), _pVal(new Valt(std::forward<Types>(args)...))
		{
		}

		std::shared_ptr<Valt> _pVal = nullptr;

	};

	template<typename Valt>
	class UniqueNode : public _CheckedNode
	{
	public:
		using _MyBase = _CheckedNode;
		using value_type = Valt;

		value_type &value()
		{
			_ensureInCont();
			return *_pVal;
		}

	protected:
		UniqueNode() {}

		UniqueNode(const UniqueNode &rhs)
			:_MyBase(rhs), _pVal(new Valt(*(rhs._pVal)))
		{
		}

		UniqueNode &operator=(const UniqueNode &rhs)
		{
			if (this != &rhs)
			{
				_MyBase::operator=(rhs);
				_pVal.reset(new Valt(*(rhs._pVal)));
			}
			return *this;
		}

		UniqueNode(void *pCont, const Valt &value)
			:_MyBase(pCont), _pVal(new Valt(value))
		{
		}

		UniqueNode(void *pCont, Valt &&value)
			:_MyBase(pCont), _pVal(new Valt(std::move(value)))
		{
		}

		template<typename... Types>
		UniqueNode(void *pCont, Types&&... args)
			: _MyBase(pCont), _pVal(new Valt(std::forward<Types>(args)...))
		{
		}

		std::unique_ptr<Valt> _pVal = nullptr;

	};
	
	template<typename T>
	INLINE size_t hash(const T &v, std::decay_t<decltype(std::hash<T>())>* = nullptr)
	{
		static std::hash<T> h;
		return h(v);
	}

	template<typename T>
	INLINE size_t hash(const T &v, std::decay_t<decltype(v.hash())>* = nullptr)
	{
		return v.hash();
	}

	INLINE size_t hash(const char *vp)
	{
		static std::hash<std::string> h;
		return h(vp);
	}
	
	template<typename _Func>
	void _traversalTuple_Recursive(const std::tuple<> &tuple, _Func func)
	{
	}

	template<typename _Func, typename _This, typename... _Rest>
	void _traversalTuple_Recursive(const std::tuple<_This, _Rest...> &tuple, _Func func)
	{
		func(std::get<0>(tuple));
		_traversalTuple_Recursive(reinterpret_cast<const std::tuple<_Rest...>&>(tuple), func);
	}

	template<typename _Func, typename... _Types>
	void traversalTuple(const std::tuple<_Types...> &tuple, _Func func)
	{
		_traversalTuple_Recursive(tuple, func);
	}

	template<typename _Func>
	void _traversalTuple_Recursive(std::tuple<> &tuple, _Func func)
	{
	}

	template<typename _Func, typename _This, typename... _Rest>
	void _traversalTuple_Recursive(std::tuple<_This, _Rest...> &tuple, _Func func)
	{
		func(std::get<0>(tuple));
		_traversalTuple_Recursive(reinterpret_cast<std::tuple<_Rest...>&>(tuple), func);
	}

	template<typename _Func, typename... _Types>
	void traversalTuple(std::tuple<_Types...> &tuple, _Func func)
	{
		_traversalTuple_Recursive(tuple, func);
	}


	template<typename... _Types>
	struct SerialSize;

	template<typename _This, typename... _Rest>
	struct SerialSize<_This, _Rest...>
	{
		static constexpr size_t value = sizeof(_This) + SerialSize<_Rest...>::value;
	};

	template<>
	struct SerialSize<>
	{
		static constexpr size_t value = 0;
	};
	
	template<typename... _Types>
	inline constexpr size_t SerialSize_v = SerialSize<_Types...>::value;


	class uuid
	{
	public:
		uuid()
		{
			for (size_t i = 0; i != SIZE; i++)
			{
				_Data[i] = static_cast<char>(randint(256));
			}
		}

		uuid(std::ifstream &inf)
		{
			inf.read(_Data, SIZE);
		}

		uuid(const string &raw)
		{
			memcpy(_Data, raw.data(), SIZE);
		}

		uuid(const uuid &) = delete;
		uuid &operator=(const uuid &) = delete;

		inline string toString() const
		{
			return hex();
		}

		string hex() const
		{
			std::stringstream ss;
			for (size_t i = 0; i != SIZE; i++)
			{
				ss << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(static_cast<unsigned char>(_Data[i]));
			}
			return ss.str();
		}

		inline static string new_hex()
		{
			return uuid().hex();
		}

		inline static uuid fromFile(std::ifstream &inf)
		{
			return uuid(inf);
		}

		inline void toFile(std::ofstream &outf, bool flush = false) const
		{
			outf.write(_Data, SIZE);
			if (flush)
				outf.flush();
		}

		inline string toRawString() const
		{
			std::stringstream ss;
			ss.write(_Data, SIZE);
			return ss.str();
		}

		inline static size_t const SIZE = 16;

	private:
		char _Data[SIZE];
	};

	std::ostream &operator<<(std::ostream &out, const uuid &value)
	{
		out << value.toString();
		return out;
	}

	template<typename _Valt>
	inline void writeValueToFile(std::ofstream &outf, const _Valt &value)
	{
		outf.write(reinterpret_cast<const char*>(&value), sizeof(_Valt));
	}

	template<typename... Types>
	inline void writeTupleToFile(std::ofstream &outf, const std::tuple<Types...> &tuple)
	{
		traversalTuple(tuple, [&](const auto &e) { writeValueToFile(outf, e); });
	}

	template<typename _Valt>
	inline void readValueFromFile(std::ifstream &inf, _Valt &value)
	{
		inf.read(reinterpret_cast<char*>(&value), sizeof(_Valt));
	}

	template<typename... Types>
	inline void readTupleFromFile(std::ifstream &inf, std::tuple<Types...> &tuple)
	{
		traversalTuple(tuple, [&](auto &e) { readValueFromFile(inf, e); });
	}

	template<typename _Valt>
	struct Serializer
	{
		using Valt = _Valt;

		inline static size_t const SIZE = sizeof(Valt);

		inline static void serialize(std::ofstream &outf, const Valt &value)
		{
			outf.write(reinterpret_cast<const char*>(&value), sizeof(Valt));
		}

		inline static string serialize(const Valt &value)
		{
			std::stringstream ss;
			ss.write(reinterpret_cast<const char*>(&value), sizeof(Valt));
			return ss.str();
		}

		inline static void unserialize(std::ifstream &inf, Valt &value)
		{
			inf.read(reinterpret_cast<char*>(&value), sizeof(Valt));
		}

		inline static void unserialize(const string &raw, Valt &value)
		{
			memcpy(&value, raw.data(), sizeof(Valt));
		}

		inline static std::unique_ptr<Valt> unserialize(std::ifstream &inf)
		{
			std::unique_ptr<Valt> ret(new Valt);
			inf.read(reinterpret_cast<char*>(ret.get()), sizeof(Valt));
			return ret;
		}

		inline static std::unique_ptr<Valt> unserialize(const string &raw)
		{
			auto p = malloc(sizeof(Valt));
			memcpy(p, raw.data(), sizeof(Valt));
			return std::unique_ptr<Valt>(reinterpret_cast<Valt*>(p));
		}
	};

	template<typename... Types>
	struct Serializer<std::tuple<Types...>>
	{
		using Valt = std::tuple<Types...>;

		inline static size_t const SIZE = SerialSize_v<Types...>;

		inline static void serialize(std::ofstream &outf, const Valt &value)
		{
			writeTupleToFile(outf, value);
		}

		inline static string serialize(const Valt &value)
		{
			std::stringstream ss;
			traversalTuple(value, [&](const auto &e) { ss.write(reinterpret_cast<const char*>(&e), sizeof(e)); });
			return ss.str();
		}

		inline static void unserialize(std::ifstream &inf, Valt &value)
		{
			readTupleFromFile(inf, value);
		}

		inline static void unserialize(const string &raw, Valt &value)
		{
			std::stringstream ss(raw);
			traversalTuple(value, [&](auto &e) { ss.read(reinterpret_cast<char*>(&e), sizeof(e)); });
		}

		inline static std::unique_ptr<Valt> unserialize(std::ifstream &inf)
		{
			std::unique_ptr<Valt> ret(new Valt);
			readTupleFromFile(inf, *ret);
			return ret;
		}

		inline static std::unique_ptr<Valt> unserialize(const string &raw)
		{
			auto tp = reinterpret_cast<Valt*>(malloc(sizeof(Valt)));
			std::stringstream ss(raw);
			traversalTuple(*tp, [&](auto &e) { ss.read(reinterpret_cast<char*>(&e), sizeof(e)); });
			return std::unique_ptr<Valt>(tp);
		}
	};

	template<>
	struct Serializer<uuid>
	{
		using Valt = uuid;

		inline static size_t const SIZE = uuid::SIZE;

		inline static void serialize(std::ofstream &outf, const Valt &value)
		{
			value.toFile(outf);
		}

		inline static string serialize(const Valt &value)
		{
			return value.toRawString();
		}

		inline static std::unique_ptr<Valt> unserialize(std::ifstream &inf)
		{
			return std::make_unique<Valt>(inf);
		}

		inline static std::unique_ptr<Valt> unserialize(const string &raw)
		{
			return std::make_unique<Valt>(raw);
		}
	};


	using std::filesystem::path;


	class FileModifier
	{
	public:
		FileModifier(const path &_path)
			: _Path(_path), _f()
		{
			if (!std::filesystem::exists(_Path))
			{
				_f.open(_Path, std::fstream::out | std::fstream::trunc);
				_f.close();
			}
			_open();
		}

		~FileModifier()
		{
			_f.close();
		}

		path getPath() const
		{
			return _Path;
		}

		size_t size()
		{
			_f.seekg(0, std::fstream::end);
			return _f.tellg();
		}

		void append(const char *data, size_t size)
		{
			_f.seekp(0, std::fstream::end);
			_f.write(data, size);
		}

		void insert(size_t pos, const char *data, size_t size)
		{
			auto total = this->size();
			pos = std::min(total, pos);
			size_t sz = total - pos;
			std::unique_ptr<char[]> p(new char[sz]);
			read(pos, p.get(), sz);
			_f.seekp(pos);
			_f.write(data, size);
			_f.write(p.get(), sz);
		}

		void seekg(size_t pos)
		{
			_f.seekg(pos);
		}

		void read(size_t pos, char *data, size_t size)
		{
			_f.seekg(pos);
			_f.read(data, size);
		}

		void read(char *data, size_t size)
		{
			_f.read(data, size);
		}

		void seekp(size_t pos)
		{
			_f.seekp(pos);
		}

		void write(size_t pos, const char *data, size_t size)
		{
			_f.seekp(pos);
			_f.write(data, size);
		}

		void write(const char *data, size_t size)
		{
			_f.write(data, size);
		}

		void copy_append(size_t pos, size_t size, FileModifier &dst)
		{
			std::unique_ptr<char[]> p(new char[size]);
			read(pos, p.get(), size);
			dst.append(p.get(), size);
		}

		void copy_insert(size_t pos, size_t size, FileModifier &dst, size_t dst_pos)
		{
			std::unique_ptr<char[]> p(new char[size]);
			read(pos, p.get(), size);
			dst.insert(dst_pos, p.get(), size);
		}

		void copy_rewrite(size_t pos, size_t size, FileModifier &dst, size_t dst_pos)
		{
			std::unique_ptr<char[]> p(new char[size]);
			read(pos, p.get(), size);
			dst.write(dst_pos, p.get(), size);
		}

		void resize(size_t size)
		{
			_f.flush();
			std::filesystem::resize_file(_Path, size);
		}

		void truncate()
		{
			resize(0);
		}

	private:
		path const _Path;
		std::fstream _f;

		void _open()
		{
			_f.open(_Path, std::fstream::in | std::fstream::out | std::fstream::binary);
			if (!_f.is_open())
				throw std::runtime_error("Fail to open file");
		}
	};

}

namespace std
{
	template<typename _First, typename... _Rest>
	std::ostream &operator<<(std::ostream &out, const std::tuple<_First, _Rest...> &tuple)
	{
		out << "(" << std::get<0>(tuple);
		auto f = [&](const auto &e) { out << ", " << e; };
		lyf::traversalTuple(reinterpret_cast<const std::tuple<_Rest...>&>(tuple), f);
		out << ")";
		return out;
	}
}
