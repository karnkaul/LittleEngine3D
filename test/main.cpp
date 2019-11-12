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
#include "le3d/game/entity.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/mesh.hpp"
#include "le3d/gfx/factory.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/input/input.hpp"
#include "le3d/log/log.hpp"

le::OnText::Token tOnText;
le::OnInput::Token tOnInput;
le::OnMouse::Token tOnMouse, tOnScroll;
const std::string resourcesPath = "../test/resources";

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

	// Fixed camera
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	glm::mat4 proj = glm::mat4(1.0f);
	proj = glm::perspective(glm::radians(45.0f), (f32)WIDTH / HEIGHT, 0.1f, 100.0f);

	std::string path(le::env::fullPath(resourcesPath + "/textures/container.jpg"));
	auto img = readBytes(path);
	le::Texture t0 = le::gfx::genTex("container.jpg", "diffuse", std::move(img));
	path = le::env::fullPath(resourcesPath + "/textures/awesomeface.png");
	img = readBytes(path);
	le::Texture t1 = le::gfx::genTex("awesomeface.png", "diffuse", std::move(img));

	std::string vshFile(le::env::fullPath(resourcesPath + "/shaders/default.vsh"));
	std::string fshFile(le::env::fullPath(resourcesPath + "/shaders/default.fsh"));
	std::string fshFile2(le::env::fullPath(resourcesPath + "/shaders/test.fsh"));
	auto vsh = readFile(vshFile);
	auto fsh = readFile(fshFile);
	auto fsh2 = readFile(fshFile2);
	le::Shader defaultShader;
	defaultShader.setup("default", vsh, fsh);
	le::Shader testShader;
	testShader.setup("test", vsh, fsh2);

	le::Mesh mesh = le::Mesh::debugCube();
	mesh.addTextures({t0, t1});
	defaultShader.setS32("mix_textures", 1);
	static bool bWireframe = false;
	static bool bParented = true;

	le::Prop prop0;
	prop0.setup("awesome-container");
	prop0.addMesh(mesh);
	prop0.m_transform.setPosition({1.0f, 1.5f, -2.0f});
	prop0.m_transform.setScale(2.0f);

	le::Prop prop1;
	prop1.addMesh(mesh);
	prop1.m_transform.setPosition({0.5f, -0.5f, -0.5f});
	prop1.m_transform.setScale(0.5f);
	prop0.m_transform.setParent(&prop1.m_transform);

	tOnText = le::input::registerText(&onText);
	tOnInput = le::input::registerInput([&](s32 key, s32 action, s32 mods) {
		if (action == GLFW_RELEASE)
		{
			if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
			{
				bWireframe = !bWireframe;
			}
			if (key == GLFW_KEY_P && mods & GLFW_MOD_CONTROL)
			{
				bParented = !bParented;
				prop0.m_transform.setParent(bParented ? &prop1.m_transform : nullptr);
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
		le::context::clearFlags(le::Colour(42, 75, 75, 255));

		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		prop0.m_transform.setOrientation(
			glm::rotate(prop0.m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(1.0f, 0.3f, 0.5f)));
		prop1.m_transform.setOrientation(
			glm::rotate(prop1.m_transform.orientation(), glm::radians(dt.assecs() * 10), glm::vec3(1.0f, 0.3f, 0.5f)));
		// mesh.draw(mesh.m_transform.model(), view, proj, defaultShader);
		le::RenderState state{view, proj, &defaultShader};
		defaultShader.setS32("mix_textures", 1);
		prop0.render(state);
		defaultShader.setS32("mix_textures", 0);
		prop1.render(state);

		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		le::context::swapBuffers();
		le::context::pollEvents();
	}

	prop0.clearFixtures();
	le::gfx::releaseTex(t0);
	le::context::destroy();
	return 0;
}

s32 main(s32 argc, char** argv)
{
	le::env::init(argc, argv);
	return run();
}
