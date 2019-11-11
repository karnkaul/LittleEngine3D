#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <glad/glad.h>
#include "le3d/core/vector2.hpp"
#include "le3d/core/time.hpp"
#include "le3d/log/log.hpp"
#include "le3d/context/context.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/shader.hpp"
#include "stb/stb_image.h"
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
GLObj newTexture(std::string_view id)
{
	std::string path(resourcesPath);
	path += "/";
	path += id;
	s32 w, h, ch;
	GLObj hTex = 0;
	stbi_set_flip_vertically_on_load(1);
	auto pData = stbi_load(path.data(), &w, &h, &ch, 0);
	if (pData)
	{
		glGenTextures(1, &hTex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, ch == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pData);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		logE("[%s] Failed to load texture!", id.data());
	}
	stbi_image_free(pData);
	return hTex;
}

void deleteTexture(GLObj& out_hTex)
{
	const GLuint glTex[] = {out_hTex};
	glDeleteTextures(1, glTex);
	out_hTex = 0;
}

Verts newQuad(Shader& shader, Colour c, Vector2 size, Vector2 origin = Vector2::Zero)
{
	Vector2 hs = Fixed::OneHalf * size;
	hs.x /= WIDTH;
	hs.y /= HEIGHT;
	Vector2 nXY(origin.x / WIDTH, origin.y / HEIGHT);
	const f32 verts[] = {
		nXY.x.toF32() + hs.x.toF32(), nXY.y.toF32() + hs.y.toF32(), 0.0f, c.r.toF32(), c.g.toF32(), c.b.toF32(), c.a.toF32(), 1.0f, 1.0f,

		nXY.x.toF32() + hs.x.toF32(), nXY.y.toF32() - hs.y.toF32(), 0.0f, c.r.toF32(), c.g.toF32(), c.b.toF32(), c.a.toF32(), 1.0f, 0.0f,

		nXY.x.toF32() - hs.x.toF32(), nXY.y.toF32() - hs.y.toF32(), 0.0f, c.r.toF32(), c.g.toF32(), c.b.toF32(), c.a.toF32(), 0.0f, 0.0f,

		nXY.x.toF32() - hs.x.toF32(), nXY.y.toF32() + hs.y.toF32(), 0.0f, c.r.toF32(), c.g.toF32(), c.b.toF32(), c.a.toF32(), 0.0f, 1.0f};
	const u32 indices[] = {0, 1, 3,

						   1, 2, 3};
	const auto stride = 9 * sizeof(float);
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

	GLint aPos = glGetAttribLocation(shader.program(), "aPos");
	if (aPos >= 0)
	{
		auto glPos = toGLObj(aPos);
		glVertexAttribPointer(glPos, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(glPos);
		glChk();
	}
	GLint aColour = glGetAttribLocation(shader.program(), "aColour");
	if (aColour >= 0)
	{
		auto glCol = toGLObj(aColour);
		glVertexAttribPointer(glCol, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(glCol);
		glChk();
	}
	GLint aTexCoord = glGetAttribLocation(shader.program(), "aTexCoord");
	if (aTexCoord >= 0)
	{
		auto glTexCoord = toGLObj(aTexCoord);
		glVertexAttribPointer(glTexCoord, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
		glEnableVertexAttribArray(glTexCoord);
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

	GLObj texture = le::newTexture("textures/container.jpg");
	GLObj t1 = le::newTexture("textures/awesomeface.png");

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

	Verts v0 = le::newQuad(defaultShader, le::Colour::White, {500, 500}, {100, 100});
	Verts v1 = le::newQuad(defaultShader, le::Colour::Yellow, {300, 300}, {-500, -350});
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
	le::Time::reset();
	le::Time dt;
	le::Time t = le::Time::now();
	while (!le::context::isClosing())
	{
		dt = le::Time::now() - t;
		t = le::Time::now();
		auto padState = le::input::getGamepadState(GLFW_JOYSTICK_1);
		logD("dt: %.2f", dt.assecs() * 1000);

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
		// defaultShader.setV4("tint", 1.0f, 0.0f, 0.0f);
		glActiveTexture(GL_TEXTURE0);
		defaultShader.use();
		defaultShader.setS32("uUseTexture", 1);
		defaultShader.setS32("uTexture", 0);
		glBindTexture(GL_TEXTURE_2D, texture);
		if (t1 > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, t1);
			defaultShader.setS32("uTexture1", 1);
			defaultShader.setS32("uMixTextures", 1);
		}
		v0.draw();
		glBindTexture(GL_TEXTURE_2D, 0);
		//testShader.use();
		defaultShader.setS32("uUseTexture", 0);
		defaultShader.setS32("uMixTextures", 0);
		v1.draw();
		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glChk();
		}

		le::context::swapBuffers();
		le::context::pollEvents();
	}

	le::deleteTexture(texture);
	v0.release();
	le::context::destroy();
	return 0;
}

s32 main(s32 argc, char** argv)
{
	le::env::init(argc, argv);
	return run();
}
