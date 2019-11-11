#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <glad/glad.h>
#include "le3d/core/vector2.hpp"
#include "le3d/log/log.hpp"
#include "le3d/context/context.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/input/input.hpp"
#include "le3d/env/env.hpp"

le::OnText::Token tOnText;
le::OnInput::Token tOnInput;
le::OnMouse::Token tOnMouse, tOnScroll;
const std::string_view resourcesPath = "../test/resources";
constexpr u16 WIDTH = 1280;
constexpr u16 HEIGHT = 720;

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

struct Verts
{
	u16 indices = 0;
	u32 vao = 0;
	u32 vbo = 0;
	u32 ebo = 0;

	virtual ~Verts();

	void draw();
	void release();
};

Verts::~Verts()
{
	release();
}

void Verts::draw()
{
	if (le::context::exists())
	{
		glBindVertexArray(vao);
		glChk();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glChk();
		glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, 0);
		glChk();
	}
}

void Verts::release()
{
	if (le::context::exists())
	{
		glDeleteVertexArrays(1, &vao);
		glChk();
		glDeleteBuffers(1, &vbo);
		glChk();
		glDeleteBuffers(1, &ebo);
		glChk();
	}
	vao = vbo = ebo = 0;
	indices = 0;
}

namespace le
{
Verts newQuad(Shader& shader, Vector2 size, Vector2 origin = Vector2::Zero)
{
	Vector2 hs = Fixed::OneHalf * size;
	hs.x /= WIDTH;
	hs.y /= HEIGHT;
	Vector2 nXY(origin.x / WIDTH, origin.y / HEIGHT);
	f32 verts[] = {nXY.x.toF32() + hs.x.toF32(), nXY.y.toF32() + hs.y.toF32(), 0.0f,

				   nXY.x.toF32() + hs.x.toF32(), nXY.y.toF32() - hs.y.toF32(), 0.0f,

				   nXY.x.toF32() - hs.x.toF32(), nXY.y.toF32() - hs.y.toF32(), 0.0f,

				   nXY.x.toF32() - hs.x.toF32(), nXY.y.toF32() + hs.y.toF32(), 0.0f};
	u32 indices[] = {0, 1, 3,

					 1, 2, 3};
	Verts v;
	v.indices = ARR_SIZE(indices);
	glGenVertexArrays(1, &v.vao);
	glChk();
	glGenBuffers(1, &v.vbo);
	glChk();
	glGenBuffers(1, &v.ebo);
	glChk();
	glBindVertexArray(v.vao);
	glChk();

	glBindBuffer(GL_ARRAY_BUFFER, v.vbo);
	glChk();
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glChk();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, v.ebo);
	glChk();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glChk();

	GLint position = glGetAttribLocation(shader.program(), "position");
	if (position >= 0)
	{
		auto glPos = toGLObj(position);
		glVertexAttribPointer(glPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(glPos);
		glChk();
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glChk();
	glBindVertexArray(0);
	glChk();
	return v;
}
} // namespace le

s32 run()
{
	if (!le::context::create(WIDTH, HEIGHT, "Test"))
	{
		return 1;
	}

	std::string vshFile(resourcesPath);
	std::string fshFile(resourcesPath);
	std::string fshFile2(resourcesPath);
	vshFile += "/shaders/default.vsh";
	fshFile += "/shaders/default.fsh";
	fshFile2 += "/shaders/test.fsh";
	auto vsh = readFile(vshFile);
	auto fsh = readFile(fshFile);
	auto fsh2 = readFile(fshFile2);
	le::Shader defaultShader;
	defaultShader.init("default", vsh, fsh);
	le::Shader testShader;
	testShader.init("test", vsh, fsh2);

	Verts v0 = le::newQuad(defaultShader, {500, 500}, {100, 100});
	Verts v1 = le::newQuad(testShader, {300, 300}, {-500, -350});
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
	tOnScroll = le::input::registerScroll([](f64 dx, f64 dy) { logD("Mouse scrolled: %.2f, %.2f", dx, dy); });
	auto test = le::input::registerFiledrop([](std::string_view path) { logD("File path: %s", path.data()); });
	glViewport(0, 0, WIDTH, HEIGHT);
	while (!le::context::isClosing())
	{
		auto padState = le::input::getGamepadState(GLFW_JOYSTICK_1);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glChk();
		glClear(GL_COLOR_BUFFER_BIT);
		glChk();
		glChk();
		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glChk();
		}
		defaultShader.setV4("colour", 1.0f, 0.0f, 0.0f);
		defaultShader.use();
		v0.draw();
		testShader.use();
		v1.draw();
		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glChk();
		}

		le::context::swapBuffers();
		le::context::pollEvents();
	}

	v0.release();
	le::context::destroy();
	return 0;
}

s32 main(s32 argc, char** argv)
{
	le::env::init(argc, argv);
	return run();
}
