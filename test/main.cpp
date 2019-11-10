#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <glad/glad.h>
#include "le3d/log/log.hpp"
#include "le3d/context/context.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/input/input.hpp"
#include "le3d/env/env.hpp"

le::OnText::Token tOnText;
le::OnInput::Token tOnInput;
le::OnMouse::Token tOnMouse, tOnScroll;
const std::string_view resourcesPath = "../test/resources";

void onText(char c)
{
	logD("%c pressed", c);
}

std::string readFile(std::string_view path)
{
	std::ifstream file(path.data());
	std::stringstream buf;
	buf << file.rdbuf();
	return buf.str();
}

s32 run()
{
	if (!le::context::create(720, 1280, "Test"))
	{
		return 1;
	}

	std::string vshFile(resourcesPath);
	std::string fshFile(resourcesPath);
	vshFile += "/shaders/default.vsh";
	fshFile += "/shaders/default.fsh";
	auto vsh = readFile(vshFile);
	auto fsh = readFile(fshFile);
	le::Shader shader;
	shader.init("default", vsh, fsh);

	f32 vertices[] = {
		0.5f,  0.5f,  0.0f,

		0.5f,  -0.5f, 0.0f,

		-0.5f, -0.5f, 0.0f,

		-0.5f, 0.5f,  0.0f,
	};
	u32 indices[] = {0, 1, 3,

					 1, 2, 3};
	u32 vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	static bool bWireframe = false;

	tOnText = le::input::registerText(&onText);
	tOnInput = le::input::registerInput([](s32 key, s32 action, s32 mods) {
		if (action == GLFW_RELEASE)
		{
			if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
			{
				bWireframe = !bWireframe;
			}
		}
		if (key == GLFW_MOUSE_BUTTON_1)
		{
			logD("Mouse button 0 [pressed/released] [%d/%d]!", action == GLFW_PRESS, action == GLFW_RELEASE);
		}
	});
	tOnMouse = le::input::registerMouse([](f64 x, f64 y) { logD("Mouse position: %.2f, %.2f", x, y); });
	tOnScroll = le::input::registerScroll([](f64 dx, f64 dy) { logD("Mouse scrolled: %.2f, %.2f", dx, dy); });
	auto test = le::input::registerFiledrop([](std::string_view path) { logD("File path: %s", path.data()); });
	glViewport(0, 0, 1280, 720);
	while (!le::context::isClosing())
	{
		auto padState = le::input::getGamepadState(GLFW_JOYSTICK_1);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shader.program());
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		// glDrawArrays(GL_TRIANGLES, 0, 3);
		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		glDrawElements(GL_TRIANGLES, ARR_SIZE(indices), GL_UNSIGNED_INT, 0);
		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		le::context::swapBuffers();
		le::context::pollEvents();
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	le::context::destroy();
	return 0;
}

s32 main(s32 argc, char** argv)
{
	le::env::init(argc, argv);
	return run();
}
