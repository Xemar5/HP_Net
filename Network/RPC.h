#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include <regex>

enum RPC_Receiver
{
	RPCr_All = -1,
	RPCr_Others = -2,
	RPCr_Server = -3,
	RPCr_Self = -4
};



//*** Functions responsible for RPC reflections
//*** Use with RPCreg / RPCregister and RPC

template<class...> struct types {};
template<class F, class Sig> struct stream_function_;
template<class Sig, class F> stream_function_<F, Sig> stream_function(F f) { return{ f }; }
template<class F, class R, class... Args> class stream_function_<F, R(Args...)>
{
public:
	stream_function_(F f) : _f(f) {}
	void operator()(std::istream& is) const { call(is, _f, types<Args...>{}); }
private:
	F _f;
	template<class Foo, class Head, class... Tail, class... Vs> void call(std::istream& is, Foo&& f, types<Head, Tail...>, Vs&&... vs) const
	{
		bool good = true;
		auto val = get<Head>(is, good);
		if (good) call(is, f, types<Tail...>{}, std::forward<Vs>(vs)..., val);
	}
	template<class Foo, class... Vs> void call(std::istream& is, Foo&& f, types<>, Vs&&... vs) const
	{
		f(std::forward<Vs>(vs)...);
	}
	template <class T>static T get(std::istream& args, bool& good)
	{
		T t;
		if (!(args >> t))
		{
			args.clear();
			std::cout << "ERR stream_function_::get : Supplied argument type doesn't match the function's arguments\n";
			good = false;
		}
		args.get();
		return t;
	}
	template <>static std::string get(std::istream& args, bool& good)
	{
		std::string t, h;
		while (true)
		{
			std::getline(args, h, '|');
			t += h;
			if (args.get() != '|') break;
		}
		return t;
	}
};

#define RPC_REG(NAME) RPC::Register(#NAME, stream_function<decltype(NAME)>(NAME))
#define RPC_SREG(NAME) RPC::RegisterStored(#NAME, stream_function<decltype(NAME)>(NAME))
#define RPC_GET(RECEIVER, NAME, ...) RPC::Set(RECEIVER, #NAME, __VA_ARGS__)

class RPC
{
public:
	//*** Calls the function with arguments encoded in given string
	static void Call(std::string str);
	//*** Creates a string containing all the information needed to call given function
	template <typename ...T> static std::string Set(std::string foo, T&&... args);
	//*** Registers an RPC function
	template <typename F> static bool Register(std::string name, F foo);
	//*** Registers an RPC function
	//*** Each on-server call of the registered function will save it
	//***	for each client that enters after the call
	template <typename F> static bool RegisterStored(std::string name, F foo);

private:
	//*** Deleted default constructor
	RPC() = delete;
	//*** A Map of all RPC functions and their names
	static std::map<std::string, std::pair<std::function<void(std::istream&)>, bool>> _Map;
	//*** Contains all saved RPC which are called at client connection
	static std::vector<std::string> _Saved;
	//*** A function to help encode the RPC string parameter
	static std::string _stringInsert(std::string val)
	{
		for (auto it = val.begin(); it != val.end(); ++it)
			if (*it == '|') { it = val.insert(it, '|'); it += 1; }
		val.push_back('|');
		return val;
	}
	static std::string _RPC(std::string& s, decltype(_Map.end()) foo);
	template <typename K, typename ...T> static std::string _RPC(std::string& s, decltype(_Map.end()) foo, K&& val, T&&... args);
	template <typename ...T, unsigned N> static std::string _RPC(std::string& s, decltype(_Map.end()) foo, const char(&val)[N], T&&... args) { s += " " + _stringInsert(val); return _RPC(s, foo, args...); }
	template <typename ...T			   > static std::string _RPC(std::string& s, decltype(_Map.end()) foo, const char*& val, T&&... args)	{ s += " " + _stringInsert(val); return _RPC(s, foo, args...); }
	template <typename ...T			   > static std::string _RPC(std::string& s, decltype(_Map.end()) foo, const char& val, T&&... args)		{ s += " " + std::string(1, val); return _RPC(s, foo, args...); }
	template <typename ...T			   > static std::string _RPC(std::string& s, decltype(_Map.end()) foo, char& val, T&&... args)			{ s += " " + std::string(1, val); return _RPC(s, foo, args...); }
	template <typename ...T			   > static std::string _RPC(std::string& s, decltype(_Map.end()) foo, std::string& val, T&&... args)	{ s += " " + _stringInsert(val); return _RPC(s, foo, args...); }
	template <typename ...T			   > static std::string _RPC(std::string& s, decltype(_Map.end()) foo, int& val, T&&... args)			{ s += " " + std::to_string(val); return _RPC(s, foo, args...); }
	template <typename ...T			   > static std::string _RPC(std::string& s, decltype(_Map.end()) foo, double& val, T&&... args)			{ s += " " + std::to_string(val); return _RPC(s, foo, args...); }
	template <typename ...T			   > static std::string _RPC(std::string& s, decltype(_Map.end()) foo, float& val, T&&... args)			{ s += " " + std::to_string(val); return _RPC(s, foo, args...); }
	template <typename ...T			   > static std::string _RPC(std::string& s, decltype(_Map.end()) foo, unsigned& val, T&&... args)		{ s += " " + std::to_string(val); return _RPC(s, foo, args...); }
	//template <typename ...T, unsigned N> static std::string _RPC(std::string& s, int r, decltype(_Map.end()) foo, const char(&val)[N], T&&... args)	{ s += (s.size() ? " " : "") + _stringInsert(val); return _RPC(s, r, foo, args...); }
	//template <typename ...T			   > static std::string _RPC(std::string& s, int r, decltype(_Map.end()) foo, const char*& val, T&&... args)		{ s += (s.size() ? " " : "") + _stringInsert(val); return _RPC(s, r, foo, args...); }
	//template <typename ...T			   > static std::string _RPC(std::string& s, int r, decltype(_Map.end()) foo, const char& val, T&&... args)		{ s += (s.size() ? " " : "") + std::string(1, val); return _RPC(s, r, foo, args...); }
	//template <typename ...T			   > static std::string _RPC(std::string& s, int r, decltype(_Map.end()) foo, char& val, T&&... args)			{ s += (s.size() ? " " : "") + std::string(1, val); return _RPC(s, r, foo, args...); }
	//template <typename ...T			   > static std::string _RPC(std::string& s, int r, decltype(_Map.end()) foo, std::string& val, T&&... args)		{ s += (s.size() ? " " : "") + _stringInsert(val); return _RPC(s, r, foo, args...); }
	//template <typename ...T			   > static std::string _RPC(std::string& s, int r, decltype(_Map.end()) foo, int& val, T&&... args)				{ s += (s.size() ? " " : "") + std::to_string(val); return _RPC(s, r, foo, args...); }
	//template <typename ...T			   > static std::string _RPC(std::string& s, int r, decltype(_Map.end()) foo, double& val, T&&... args)			{ s += (s.size() ? " " : "") + std::to_string(val); return _RPC(s, r, foo, args...); }
	//template <typename ...T			   > static std::string _RPC(std::string& s, int r, decltype(_Map.end()) foo, float& val, T&&... args)			{ s += (s.size() ? " " : "") + std::to_string(val); return _RPC(s, r, foo, args...); }
	//template <typename ...T			   > static std::string _RPC(std::string& s, int r, decltype(_Map.end()) foo, unsigned& val, T&&... args)		{ s += (s.size() ? " " : "") + std::to_string(val); return _RPC(s, r, foo, args...); }

	friend class Network;
	friend class NetworkTCP;
};



template<typename ...T>
inline std::string RPC::Set(std::string foo, T && ...args)
{
	auto f = _Map.find(foo);
	if (f != _Map.end())
	{
		std::string s;
		return RPC::_RPC(s, f, args...);
	}
	std::cout << "ERR Network::RPC : No RPC function registered with given name\n";
	return "";
}
template<typename K, typename ...T>
inline std::string RPC::_RPC(std::string& s, decltype(_Map.end()) foo, K&& val, T&& ...args)
{
	std::cout << "ERR Network::RPC : unsupported value\n";
	return "";
}





template<typename F>
inline bool RPC::Register(std::string name, F foo)
{
	if (_Map.find(name) != _Map.end()) { std::cout << "ERR Network::RegisterRPC : Name already taken\n"; return false; }
	_Map[name] = std::pair<F, bool>(foo, false);
	return true;
}

template<typename F>
inline bool RPC::RegisterStored(std::string name, F foo)
{
	if (_Map.find(name) != _Map.end()) { std::cout << "ERR Network::RPC_RegisterStored : Name already taken\n"; return false; }
	_Map[name] = std::pair<F, bool>(foo, true);
	return true;
}


