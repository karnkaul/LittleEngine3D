#include <iostream>
#include <glad/glad.h>
#include "le3d/log/log.hpp"
#include "le3d/context/context.hpp"
#include "le3d/input/input.hpp"
#include "le3d/env/env.hpp"

le::OnText::Token token;

void onText(char c) 
{
	logD("%c pressed", c);
}

s32 run()
{
	if (!le::context::create(720, 1280, "Test"))
	{
		return 1;
	}

	token = le::input::registerText(&onText);
	glViewport(0, 0, 1280, 720);
	while (!le::context::isClosing())
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		le::context::pollEvents();
		le::context::swapBuffers();
	}
	le::context::destroy();
	return 0;
}

s32 main(s32 argc, char** argv)
{
	le::env::init(argc, argv);
	return run();
}
