#include "Network.h"
#include <SDL.h>

SDL_Event e;

int main()
{
	std::cout << "starting...";
	bool q = false;
	while (!q)
	{
		Network::Update();
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) q = true;
		}
	}
	std::cout << "exitting...";
	return 0;
}