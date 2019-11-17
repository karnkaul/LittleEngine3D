#include <array>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "le3d/core/assert.hpp"
#include "le3d/core/time.hpp"
#include "le3d/context/context.hpp"
#include "le3d/env/env.hpp"
#include "le3d/game/camera.hpp"
#include "le3d/game/entity.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/mesh.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/input/input.hpp"
#include "le3d/core/log.hpp"

le::OnInput::Token tOnInput;
const std::string resourcesPath = "../test/resources";

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

	if (!le::context::glCreate(WIDTH, HEIGHT, "Test"))
	{
		return 1;
	}

	le::FreeCam camera;
	// le::input::setCursorMode(le::CursorMode::Disabled);
	// le::Camera camera;
	camera.m_position = {0.0f, 0.0f, 3.0f};
	camera.m_flags.set((s32)le::FreeCam::Flag::FixedSpeed, false);

	auto path = le::env::fullPath(resourcesPath + "/textures/blank_1px.bmp");
	auto img = readBytes(path);
	le::Texture blankTex = le::gfx::gl::genTex("blank", "diffuse", std::move(img));
	path = (le::env::fullPath(resourcesPath + "/textures/container2.png"));
	img = readBytes(path);
	le::Texture t0 = le::gfx::gl::genTex("container2.png", "diffuse", std::move(img));
	path = le::env::fullPath(resourcesPath + "/textures/container2_specular.png");
	img = readBytes(path);
	le::Texture t0s = le::gfx::gl::genTex("container2_specular.png", "specular", std::move(img));
	path = le::env::fullPath(resourcesPath + "/textures/awesomeface.png");
	img = readBytes(path);
	le::Texture t1 = le::gfx::gl::genTex("awesomeface.png", "diffuse", std::move(img));

	std::string vshFile(le::env::fullPath(resourcesPath + "/shaders/default.vsh"));
	std::string fshFile(le::env::fullPath(resourcesPath + "/shaders/default.fsh"));
	std::string fshFile2(le::env::fullPath(resourcesPath + "/shaders/test.fsh"));
	std::string lightFSH(le::env::fullPath(resourcesPath + "/shaders/basic_light.fsh"));
	std::string litTexFSH(le::env::fullPath(resourcesPath + "/shaders/lit_texture.fsh"));
	auto vsh = readFile(vshFile);
	auto fsh = readFile(fshFile);
	auto fsh2 = readFile(fshFile2);
	auto fshLight = readFile(lightFSH);
	auto litTex = readFile(litTexFSH);
	le::Shader litShader;
	litShader.glSetup("default", vsh, fsh);
	litShader.setV3("light.ambient", glm::vec3(0.2f));
	litShader.setV3("light.diffuse", glm::vec3(0.5f));
	litShader.setV3("light.specular", glm::vec3(1.0f));
	litShader.setV3("material.ambient", {1.0f, 0.5f, 0.31f});
	litShader.setV3("material.diffuse", {1.0f, 0.5f, 0.31f});
	litShader.setV3("material.specular", glm::vec3(0.2f));
	litShader.setF32("material.shininess", 32.0f);
	litShader.m_flags.set((s32)le::Shader::Flag::Untextured, true);
	le::Shader lightShader;
	lightShader.glSetup("light", vsh, fshLight);
	lightShader.m_flags.set((s32)le::Shader::Flag::Unlit, true);
	lightShader.m_flags.set((s32)le::Shader::Flag::Untextured, true);
	le::Shader litTexShader;
	litTexShader.glSetup("litTexture", vsh, litTex);
	litTexShader.setV3("light.ambient", glm::vec3(0.2f));
	litTexShader.setV3("light.diffuse", glm::vec3(0.5f));
	litTexShader.setV3("light.specular", glm::vec3(1.0f));
	litTexShader.setF32("material.shininess", 32.0f);

	le::Mesh mesh = le::Mesh::debugCube();
	le::Texture bad;
	mesh.m_textures = {t0, t0s};
	//mesh.m_textures = {bad};
	//lightingShader.setS32("mix_textures", 1);
	static bool bWireframe = false;
	static bool bParented = true;

	le::Prop prop0;
	prop0.setup("awesome-container");
	prop0.addFixture(mesh);
	glm::mat4 offset(1.0f);
	offset = glm::translate(offset, glm::vec3(0.0f, 2.0f, 0.0f));
	prop0.addFixture(mesh, offset);
	prop0.m_transform.setPosition({2.0f, 2.5f, -2.0f});
	prop0.m_transform.setScale(2.0f);

	le::Prop prop1;
	prop1.addFixture(mesh);
	prop1.m_transform.setPosition({0.5f, -0.5f, -0.5f});
	prop1.m_transform.setScale(0.25f);
	prop0.m_transform.setParent(&prop1.m_transform);
	prop1.setShader(&litShader);

	le::HVerts light0 = le::gfx::tutorial::newLight(mesh.VAO());

	std::vector<le::Prop> props;
	for (s32 i = 0; i < 5; ++i)
	{
		le::Prop prop;
		prop.addFixture(mesh);
		//prop.setShader(&litShader);
		prop.setShader(&litShader);
		props.emplace_back(std::move(prop));
	}
	props[0].m_transform.setPosition({-0.5f, 0.5f, -4.0f});
	props[1].m_transform.setPosition({-0.5f, 1.5f, 4.0f});
	props[2].m_transform.setPosition({1.5f, -0.5f, 4.0f});
	props[3].m_transform.setPosition({-3.0f, -1.0f, 2.0f});
	props[4].m_transform.setPosition({-4.0f, 1.0f, -2.0f});

	tOnInput = le::input::registerInput([&](s32 key, s32 action, s32 mods) {
		if (action == GLFW_RELEASE)
		{
			if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
			{
				bWireframe = !bWireframe;
				prop0.m_flags.set((s32)le::Entity::Flag::Wireframe, bWireframe);
			}
			if (key == GLFW_KEY_P && mods & GLFW_MOD_CONTROL)
			{
				bParented = !bParented;
				prop0.m_transform.setParent(bParented ? &prop1.m_transform : nullptr);
			}
		}
		if (key == GLFW_MOUSE_BUTTON_1)
		{
			LOG_D("Mouse button 0 [pressed/released] [%d/%d]!", action == GLFW_PRESS, action == GLFW_RELEASE);
		}
	});
#if defined(DEBUGGING)
	auto test = le::input::registerFiledrop([](std::string_view path) { LOG_D("File path: %s", path.data()); });
#endif
	le::Time::reset();
	le::Time dt;
	le::Time t = le::Time::now();
	while (!le::context::isClosing())
	{
		dt = le::Time::now() - t;
		t = le::Time::now();
		camera.tick(dt);
		le::context::glClearFlags();

		prop0.m_transform.setOrientation(
			glm::rotate(prop0.m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(1.0f, 0.3f, 0.5f)));
		prop1.m_transform.setOrientation(
			glm::rotate(prop1.m_transform.orientation(), glm::radians(dt.assecs() * 10), glm::vec3(1.0f, 0.3f, 0.5f)));
		le::RenderState state;
		state.view = camera.view();
		state.projection = camera.perspectiveProj(le::context::nativeAR());
		state.pShader = &litShader;
		state.pShader = &litTexShader;
		//lightingShader.setS32("mix_textures", 1);
		// static glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
		static glm::vec3 lightPos(0.0f, 0.0f, 2.0f);
		litShader.setV3("light.position", lightPos);
		litTexShader.setV3("light.position", lightPos);
		//prop0.setShader(&litShader);
		prop0.render(state);
		prop1.render(state);
		for (auto& prop : props)
		{
			prop.setShader(&litTexShader);
			prop.render(state);
		}

		glm::mat4 m(1.0f);
		m = glm::translate(m, lightPos);
		m = glm::scale(m, glm::vec3(0.2f));
		le::gfx::draw(light0, m, m, state, lightShader);

		le::context::swapBuffers();
		le::context::pollEvents();
	}

	prop0.clearFixtures();
	le::gfx::gl::releaseTex(t0);
	le::context::glDestroy();
	return 0;
}

s32 main(s32 argc, char** argv)
{
	le::env::init(argc, argv);
	return run();
}
