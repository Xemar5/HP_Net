#include "RPC.h"
#include "Network.h"

std::map<std::string, std::pair<std::function<void(std::istream&)>, bool>> RPC::_Map;
std::vector<std::string> RPC::_Saved;
//std::regex RPC::_Regex("([a-zA-Z_]+)(\\d*)\\.(\\d)\\.([a-zA-Z_<>:\\d]+)\\.(.*)");



void RPC::Call(std::string str)
{
}

std::string RPC::_RPC(std::string & s, decltype(_Map.end()) foo)
{
	if (foo == _Map.end()) { std::cout << "ERR Network::RPC : Given RPC function not supplied\n"; return ""; }
	return foo->first + s;
}
