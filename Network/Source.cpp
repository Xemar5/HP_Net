#include "Network.h"
#include "RPC.h"
#include <SDL.h>

SDL_Event e;

void Lookko(int i)
{
	std::cout << "In Lookko: " << i << std::endl;
}

int main(int argc, char** argv)
{
	RPC_SREG(Lookko);
	int ccwc = 0;
	int weqweq = 0;
	std::cout << "starting..." + std::to_string(argc) + "\n";
	if (argc > 1) Network::Host(4);
	else Network::Join("127.0.0.1");
	bool q = false;
	while (!q)
	{
		if(Network::isConnected())
			ccwc++;
		if (ccwc > 0)
		{
			ccwc = 0;
			Network::RemoteFunc(RPCr_Others, Lookko, weqweq++);
			//Network::Send(RPC_GET(RPCr_All, Lookko, weqweq++));
			//Network::Send(".123456789.123456789.123456789.123456789.123456789.123456789.123456789.123456789.123456789.123456789.123456789.123456789.123456789.123456789.123456789.123456789.123456789");
		}
		Network::Update();
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) q = true;
		}
		SDL_Delay(1);
	}
	std::cout << "exitting...\n";
	return 0;
}