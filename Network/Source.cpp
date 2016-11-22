#include "Network.h"
#include <SDL.h>

SDL_Event e;



int main(int argc, char** argv)
{
	std::cout << "starting..." + std::to_string(argc) + "\n";
	if (argc > 1) Network::Host(4);
	else Network::Join("127.0.0.1");
	bool q = false;
	while (!q)
	{
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