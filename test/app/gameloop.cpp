#include "le3d/context/context.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/env/env.hpp"
#include "le3d/game/camera.hpp"
#include "le3d/game/entity.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/game/scene.hpp"
#include "le3d/gfx/mesh.hpp"
#include "le3d/input/input.hpp"

#include "gameloop.hpp"

namespace letest
{
using namespace le;
using namespace le::utils;

namespace
{
OnInput::Token tOnInput;
const std::string resourcesPath = "../test/resources";

std::string resourcePath(std::string_view id)
{
	std::string ret = resourcesPath;
	ret += "/";
	ret += id;
	return env::fullPath(ret);
}

void runTest()
{
	Scene scene;
	scene.cameras.uMain = std::make_unique<FreeCam>();
	auto& camera = *dynamic_cast<FreeCam*>(scene.cameras.uMain.get());
	camera.setup("freecam");
	// input::setCursorMode(CursorMode::Disabled);
	camera.m_position = {0.0f, 0.0f, 3.0f};
	camera.m_flags.set((s32)FreeCam::Flag::FixedSpeed, false);

	static const std::string DIFFUSE = "diffuse";
	static const std::string SPECULAR = "specular";

	resources::loadTexture("container2", DIFFUSE, readBytes(resourcePath("textures/container2.png")));
	resources::loadTexture("container2_specular", SPECULAR, readBytes(resourcePath("textures/container2_specular.png")));
	resources::loadTexture("awesomeface", DIFFUSE, readBytes(resourcePath("textures/awesomeface.png")));

	Flags<Shader::MAX_FLAGS> noTex;
	noTex.set((s32)gfx::shading::Flag::Untextured, true);
	Flags<Shader::MAX_FLAGS> noLit;
	noLit.set((s32)gfx::shading::Flag::Unlit, true);
	Flags<Shader::MAX_FLAGS> noTexNoLit;
	noTexNoLit.set((s32)gfx::shading::Flag::Unlit, true);
	noTexNoLit.set((s32)gfx::shading::Flag::Untextured, true);

	auto vsh = readFile(resourcePath("shaders/default.vsh"));
	auto unlitTinted = resources::loadShader("unlit/tinted", vsh, readFile(resourcePath("shaders/unlit/tinted.fsh")), noTexNoLit);
	auto unlitTextured = resources::loadShader("unlit/textured", vsh, readFile(resourcePath("shaders/unlit/textured.fsh")), noLit);
	auto litTinted = resources::loadShader("lit/tinted", vsh, readFile(resourcePath("shaders/lit/tinted.fsh")), noTex);
	scene.mainShader = resources::loadShader("lit/textured", vsh, readFile(resourcePath("shaders/lit/textured.fsh")), {});
	gfx::shading::setV4(litTinted, "tint", Colour::Yellow);

	DirLight dirLight;
	PtLight pl0, pl1;
	glm::vec3 light0Pos(0.0f, 0.0f, 2.0f);
	glm::vec3 light1Pos(-4.0f, 2.0f, 2.0f);

	pl0.position = light0Pos;
	pl1.position = light1Pos;
	scene.lighting.dirLight = dirLight;
	scene.lighting.pointLights = {pl0, pl1};

	auto drawLight = [](const glm::vec3& pos, HVerts light, const RenderState& state) {
		glm::mat4 m(1.0f);
		m = glm::translate(m, pos);
		m = glm::scale(m, glm::vec3(0.1f));
		const auto& tinted = resources::findShader("unlit/tinted");
		gfx::shading::setV4(tinted, "tint", Colour::White);
		gfx::gl::draw(light, m, m, state, tinted);
	};

	Texture bad;
	auto& mesh = resources::debugMesh();
	auto& quad = resources::debugQuad();
	std::vector<Texture> textures = {resources::getTexture("awesomeface")};
	quad.m_textures = {resources::getTexture("awesomeface")};
	// quad.m_textures = {bad};
	mesh.m_textures = {resources::getTexture("container2"), resources::getTexture("container2_specular")};
	// mesh.m_textures = {bad};
	// lightingShader.setS32("mix_textures", 1);
	static bool bWireframe = false;
	static bool bParented = true;

	Prop prop0;
	prop0.setup("awesome-container");
	prop0.addFixture(mesh);
	glm::mat4 offset(1.0f);
	offset = glm::translate(offset, glm::vec3(0.0f, 2.0f, 0.0f));
	prop0.addFixture(mesh, offset);
	prop0.m_transform.setPosition({2.0f, 2.5f, -2.0f});
	prop0.m_transform.setScale(2.0f);
	prop0.setShader(resources::findShader("lit/textured"));

	Prop prop1;
	prop1.setup("prop1");
	prop1.addFixture(mesh);
	prop1.m_transform.setPosition({0.5f, -0.5f, -0.5f});
	prop1.m_transform.setScale(0.25f);
	prop0.m_transform.setParent(&prop1.m_transform);
	prop1.setShader(resources::findShader("lit/tinted"));

	Prop quadProp;
	quadProp.setup("quad");
	quadProp.addFixture(quad);
	quadProp.setShader(resources::findShader("unlit/textured"));
	quadProp.m_transform.setPosition(glm::vec3(-2.0f, 2.0f, -2.0f));

	HVerts light0 = gfx::tutorial::newLight(mesh.VAO());
	HVerts light1 = gfx::tutorial::newLight(mesh.VAO());

	std::vector<Prop> props;
	for (s32 i = 0; i < 5; ++i)
	{
		Prop prop;
		prop.setup("prop_" + std::to_string(i));
		prop.addFixture(mesh);
		prop.setShader(resources::findShader("lit/textured"));
		props.emplace_back(std::move(prop));
	}
	props[0].m_transform.setPosition({-0.5f, 0.5f, -4.0f});
	props[1].m_transform.setPosition({-0.5f, 1.5f, 4.0f});
	props[2].m_transform.setPosition({1.5f, -0.5f, 4.0f});
	props[3].m_transform.setPosition({-3.0f, -1.0f, 2.0f});
	props[4].m_transform.setPosition({-4.0f, 1.0f, -2.0f});

	tOnInput = input::registerInput([&](s32 key, s32 action, s32 mods) {
		if (action == GLFW_RELEASE)
		{
			if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
			{
				bWireframe = !bWireframe;
				prop0.m_flags.set((s32)Entity::Flag::Wireframe, bWireframe);
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
	auto test = input::registerFiledrop([](std::string_view path) { LOG_D("File path: %s", path.data()); });
#endif
	Time::reset();
	Time dt;
	Time t = Time::now();

	while (!context::isClosing())
	{
		dt = Time::now() - t;
		t = Time::now();
		camera.tick(dt);
		// context::glClearFlags(Colour(50, 40, 10));
		context::glClearFlags();

		prop0.m_transform.setOrientation(
			glm::rotate(prop0.m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(1.0f, 0.3f, 0.5f)));
		prop1.m_transform.setOrientation(
			glm::rotate(prop1.m_transform.orientation(), glm::radians(dt.assecs() * 10), glm::vec3(1.0f, 0.3f, 0.5f)));
		quadProp.m_transform.setOrientation(
			glm::rotate(prop1.m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(0.3f, 0.5f, 1.0f)));

		resources::shadeLights({dirLight}, scene.lighting.pointLights);
		RenderState state = scene.perspective(context::nativeAR());
		prop0.render(state);
		prop1.render(state);
		quadProp.render(state);
		for (auto& prop : props)
		{
			// prop.setShader(resources::getShader("lit/textured"));
			prop.render(state);
		}
		drawLight(light0Pos, light0, state);
		drawLight(light1Pos, light1, state);
		context::swapBuffers();
		context::pollEvents();
	}

	prop0.clearFixtures();
}
} // namespace

s32 gameloop::run(s32 argc, char** argv)
{
	le::env::init(argc, argv);

	constexpr u16 WIDTH = 1280;
	constexpr u16 HEIGHT = 720;

#if defined(DEBUGGING)
	context::g_bVSYNC = false;
#endif
	if (!context::glCreate(WIDTH, HEIGHT, "Test"))
	{
		return 1;
	}
	runTest();
	context::glDestroy();
	return 0;
}
} // namespace letest
