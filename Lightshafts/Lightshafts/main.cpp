#include "Window.h"

int main(int argc, char** argv)
{
	Window window;

	window.Create("OpenGL Playground", 1920, 1080);
	window.Init();

	while (!window.is_closed)
	{
		window.Update();
	}

	return 0;
}