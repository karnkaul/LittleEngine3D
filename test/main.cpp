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

le::OnText::Token token;
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
	
	f32 vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};
	u32 vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	token = le::input::registerText(&onText);
	glViewport(0, 0, 1280, 720);
	while (!le::context::isClosing())
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader.program());
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		le::context::pollEvents();
		le::context::swapBuffers();
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	le::context::destroy();
	return 0;
}

s32 main(s32 argc, char** argv)
{
	le::env::init(argc, argv);
	return run();
}
