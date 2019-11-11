#include <array>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "le3d/core/vector2.hpp"
#include "le3d/core/rect2.hpp"
#include "le3d/core/time.hpp"
#include "le3d/context/context.hpp"
#include "le3d/env/env.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/factory.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/input/input.hpp"
#include "le3d/log/log.hpp"

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
	if (file.good())
	{
		buf << file.rdbuf();
	}
	return buf.str();
}

std::vector<u8> readBytes(std::string_view path) 
{
	std::string sPath(path);
	std::ifstream file(path.data(), std::ios::binary);
	std::vector<u8> buf;
	if (file.good())
	{
		buf = std::vector<u8>(std::istreambuf_iterator<char>(file), {});
	}
	return buf;
}

s32 run()
{
	constexpr u16 WIDTH = 1280;
	constexpr u16 HEIGHT = 720;

	if (!le::context::create(WIDTH, HEIGHT, "Test"))
	{
		return 1;
	}

	std::string path(resourcesPath);
	path += "/textures/container.jpg";
	auto img = readBytes(path);
	GLObj texture = le::gfx::genTex(std::move(img));
	path = resourcesPath;
	path += "/textures/awesomeface.png";
	img = readBytes(path);
	GLObj t1 = le::gfx::genTex(std::move(img));

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

	le::Primitive p0;
	p0.setShader(defaultShader);
	p0.provisionQuad(le::Rect2::sizeCentre({1, 1}), le::Rect2::UVs, le::Colour::White);
	p0.m_local = glm::rotate(p0.m_local, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	glm::mat4 proj = glm::mat4(1.0f);
	proj = glm::perspective(glm::radians(45.0f), (f32)WIDTH / HEIGHT, 0.1f, 100.0f);
	le::Primitive p1;
	p1.m_local = glm::translate(p1.m_local, glm::vec3(0.0f, 0.0f, 1.0f));
	p1.setShader(defaultShader);
	p1.provisionQuad(le::Rect2::sizeCentre({le::Fixed(0.5f), le::Fixed(0.5f)}, {le::Fixed(-0.5f), le::Fixed(-0.35f)}), le::Rect2::UVs, le::Colour::Yellow);
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
	le::Time::reset();
	le::Time dt;
	le::Time t = le::Time::now();
	while (!le::context::isClosing())
	{
		dt = le::Time::now() - t;
		t = le::Time::now();
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
		// defaultShader.setV4("tint", 1.0f, 0.0f, 0.0f);
		defaultShader.use();
		defaultShader.setS32("uUseTexture", 1);
		defaultShader.setS32("uTexture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		if (t1 > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, t1);
			defaultShader.setS32("uTexture1", 1);
			defaultShader.setS32("uMixTextures", 1);
		}
		//v0.draw();
		p0.draw(view, proj);
		glBindTexture(GL_TEXTURE_2D, 0);
		// testShader.use();
		defaultShader.setS32("uUseTexture", 0);
		defaultShader.setS32("uMixTextures", 0);
		//v1.draw();
		p1.draw(view, proj);
		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glChk();
		}

		le::context::swapBuffers();
		le::context::pollEvents();
	}

	le::gfx::releaseTex(texture);
	//v0.release();
	le::context::destroy();
	return 0;
}

s32 main(s32 argc, char** argv)
{
	le::env::init(argc, argv);
	return run();
}
