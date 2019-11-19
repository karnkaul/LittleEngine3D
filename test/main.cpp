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
#include "le3d/game/resources.hpp"
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

std::string resourcePath(std::string_view id)
{
	std::string ret = resourcesPath;
	ret += "/";
	ret += id;
	return le::env::fullPath(ret);
}

s32 run()
{
	constexpr u16 WIDTH = 1280;
	constexpr u16 HEIGHT = 720;
	static glm::vec3 lightPos(0.0f, 0.0f, 2.0f);

	if (!le::context::glCreate(WIDTH, HEIGHT, "Test"))
	{
		return 1;
	}

	le::FreeCam camera;
	// le::input::setCursorMode(le::CursorMode::Disabled);
	// le::Camera camera;
	camera.m_position = {0.0f, 0.0f, 3.0f};
	camera.m_flags.set((s32)le::FreeCam::Flag::FixedSpeed, false);

	static const std::string DIFFUSE = "diffuse";
	static const std::string SPECULAR = "specular";

	le::resources::loadTexture("container2", DIFFUSE, readBytes(resourcePath("textures/container2.png")));
	le::resources::loadTexture("container2_specular", SPECULAR, readBytes(resourcePath("textures/container2_specular.png")));
	le::resources::loadTexture("awesomeface", DIFFUSE, readBytes(resourcePath("textures/awesomeface.png")));

	auto vsh = readFile(resourcePath("shaders/default.vsh"));
	auto pUnlitTinted = le::resources::loadShader("unlit/tinted", vsh, readFile(resourcePath("shaders/unlit/tinted.fsh")));
	auto pUnlitTextured = le::resources::loadShader("unlit/textured", vsh, readFile(resourcePath("shaders/unlit/textured.fsh")));
	auto pLitTinted = le::resources::loadShader("lit/tinted", vsh, readFile(resourcePath("shaders/lit/tinted.fsh")));
	le::resources::loadShader("lit/textured", vsh, readFile(resourcePath("shaders/lit/textured.fsh")));
	pLitTinted->m_flags.set((s32)le::Shader::Flag::Untextured, true);
	pLitTinted->setV4("tint", le::Colour::Yellow);
	pUnlitTinted->m_flags.set((s32)le::Shader::Flag::Unlit, true);
	pUnlitTinted->m_flags.set((s32)le::Shader::Flag::Untextured, true);
	pUnlitTextured->m_flags.set((s32)le::Shader::Flag::Unlit, true);

	le::DirLight dirLight;
	le::PtLight pointLight;
	pointLight.position = lightPos;
	
	le::Mesh mesh = le::Mesh::debugCube();
	le::Texture bad;
	mesh.m_textures = {le::resources::getTexture("container2"), le::resources::getTexture("container2_specular")};
	// mesh.m_textures = {bad};
	// lightingShader.setS32("mix_textures", 1);
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
	prop1.setShader(le::resources::findShader("lit/tinted"));

	le::HVerts light0 = le::gfx::tutorial::newLight(mesh.VAO());

	std::vector<le::Prop> props;
	for (s32 i = 0; i < 5; ++i)
	{
		le::Prop prop;
		prop.addFixture(mesh);
		prop.setShader(le::resources::findShader("lit/tinted"));
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
		le::context::glClearFlags(le::Colour(20, 50, 50));

		prop0.m_transform.setOrientation(
			glm::rotate(prop0.m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(1.0f, 0.3f, 0.5f)));
		prop1.m_transform.setOrientation(
			glm::rotate(prop1.m_transform.orientation(), glm::radians(dt.assecs() * 10), glm::vec3(1.0f, 0.3f, 0.5f)));
		le::RenderState state;
		state.view = camera.view();
		state.projection = camera.perspectiveProj(le::context::nativeAR());
		state.pShader = le::resources::findShader("lit/textured");
		state.pointLights.push_back(pointLight);
		state.dirLights.push_back(dirLight);
		prop0.render(state);
		prop1.render(state);
		for (auto& prop : props)
		{
			prop.setShader(le::resources::findShader("lit/textured"));
			prop.render(state);
		}

		glm::mat4 m(1.0f);
		m = glm::translate(m, lightPos);
		m = glm::scale(m, glm::vec3(0.1f));
		le::gfx::draw(light0, m, m, state, *pUnlitTinted);

		le::context::swapBuffers();
		le::context::pollEvents();
	}

	prop0.clearFixtures();
	le::context::glDestroy();
	return 0;
}

s32 main(s32 argc, char** argv)
{
	le::env::init(argc, argv);
	return run();
}
