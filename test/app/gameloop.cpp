#include "le3d/context/context.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/env/env.hpp"
#include "le3d/env/threads.hpp"
#include "le3d/game/camera.hpp"
#include "le3d/game/entity.hpp"
#include "le3d/game/fileLogger.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/game/scene.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/gfx/model.hpp"
#include "le3d/gfx/primitives.hpp"
#include "le3d/gfx/utils.hpp"
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

	Flags<HShader::MAX_FLAGS> noTex;
	noTex.set((s32)gfx::shading::Flag::Untextured, true);
	Flags<HShader::MAX_FLAGS> noLit;
	noLit.set((s32)gfx::shading::Flag::Unlit, true);
	Flags<HShader::MAX_FLAGS> noTexNoLit;
	noTexNoLit.set((s32)gfx::shading::Flag::Unlit, true);
	noTexNoLit.set((s32)gfx::shading::Flag::Untextured, true);

	auto def = readFile(resourcePath("shaders/default.vsh"));
	auto ui = readFile(resourcePath("shaders/ui.vsh"));
	/*auto& unlitTinted = */ resources::loadShader("unlit/tinted", def, readFile(resourcePath("shaders/unlit/tinted.fsh")), noTexNoLit);
	/*auto& unlitTextured = */ resources::loadShader("unlit/textured", def, readFile(resourcePath("shaders/unlit/textured.fsh")), noLit);
	auto& litTinted = resources::loadShader("lit/tinted", def, readFile(resourcePath("shaders/lit/tinted.fsh")), noTex);
	scene.mainShader = resources::loadShader("lit/textured", def, readFile(resourcePath("shaders/lit/textured.fsh")), {});
	/*auto& uiTextured = */ resources::loadShader("ui/textured", ui, readFile(resourcePath("shaders/unlit/textured.fsh")), noTexNoLit);
	/*auto& uiTinted = */ resources::loadShader("ui/tinted", ui, readFile(resourcePath("shaders/unlit/tinted.fsh")), noLit);
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

	HTexture bad;
	auto& cubeMesh = resources::debugCube();
	auto& quadMesh = resources::debugQuad();
	quadMesh.textures = {resources::getTexture("awesomeface")};
	// quad.m_textures = {bad};
	cubeMesh.textures = {resources::getTexture("container2"), resources::getTexture("container2_specular")};
	HMesh blankCubeMesh = cubeMesh;
	blankCubeMesh.textures.clear();
	Model cube;
	Model blankCube;
	Model cubeStack;
	cube.setupModel("cube");
	cubeStack.setupModel("cubeStack");
	blankCube.setupModel("blankCube");
	cube.addFixture(cubeMesh);
	cubeStack.addFixture(cubeMesh);
	blankCube.addFixture(blankCubeMesh);
	glm::mat4 offset(1.0f);
	offset = glm::translate(offset, glm::vec3(0.0f, 2.0f, 0.0f));
	cubeStack.addFixture(cubeMesh, offset);
	Model quad;
	quad.setupModel("quad");
	quad.addFixture(quadMesh);

	// mesh.m_textures = {bad};
	// lightingShader.setS32("mix_textures", 1);
	static bool bWireframe = false;
	static bool bParented = true;

	Prop prop0;
	prop0.setup("awesome-container");
	prop0.addModel(cubeStack);
	prop0.m_transform.setPosition({2.0f, 2.5f, -2.0f});
	prop0.m_transform.setScale(2.0f);
	prop0.setShader(resources::findShader("lit/textured"));

	Prop prop1;
	prop1.setup("prop1");
	prop1.addModel(cube);
	prop1.m_transform.setPosition({0.5f, -0.5f, -0.5f});
	prop1.m_transform.setScale(0.25f);
	prop0.m_transform.setParent(&prop1.m_transform);
	prop1.setShader(resources::findShader("lit/tinted"));
	prop1.m_oTintOverride = Colour::Yellow;

	Prop quadProp;
	quadProp.setup("quad");
	quadProp.addModel(quad);
	quadProp.setShader(resources::findShader("unlit/textured"));
	quadProp.m_transform.setPosition(glm::vec3(-2.0f, 2.0f, -2.0f));

	HVerts light0 = gfx::tutorial::newLight(cubeMesh.hVerts);
	HVerts light1 = gfx::tutorial::newLight(cubeMesh.hVerts);

	std::vector<Prop> props;
	for (s32 i = 0; i < 5; ++i)
	{
		Prop prop;
		prop.setup("prop_" + std::to_string(i));
		Model& m = i < 3 ? cube : blankCube;
		prop.addModel(m);
		std::string s = i < 3 ? "lit/textured" : "lit/tinted";
		prop.setShader(resources::findShader(s));
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
		context::clearFlags();

		prop0.m_transform.setOrientation(
			glm::rotate(prop0.m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(1.0f, 0.3f, 0.5f)));
		prop1.m_transform.setOrientation(
			glm::rotate(prop1.m_transform.orientation(), glm::radians(dt.assecs() * 10), glm::vec3(1.0f, 0.3f, 0.5f)));
		quadProp.m_transform.setOrientation(
			glm::rotate(prop1.m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(0.3f, 0.5f, 1.0f)));

		resources::shadeLights({dirLight}, scene.lighting.pointLights);
		// resources::shadeLights({}, {pl0});
		RenderState state = scene.perspective();
		prop0.render(state);
		prop1.render(state);
		quadProp.render(state);
		for (auto& prop : props)
		{
			// prop.setShader(resources::getShader("lit/textured"));
			prop.render(state);
		}

		Quad2D tl, tr, bl, br;
		// glm::vec2 uiSpace = {1920.0f, 1080.0f};
		glm::vec2 uiSpace = {1280.0f, 720.0f};
		tl.pTexture = tr.pTexture = bl.pTexture = br.pTexture = &resources::getTexture("awesomeface");
		tl.size = tr.size = bl.size = br.size = {200.0f, 200.0f};
		tl.space = tr.space = bl.space = br.space = uiSpace;
		tl.oTexCoords = tr.oTexCoords = bl.oTexCoords = br.oTexCoords = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);
		tr.pos = {uiSpace.x * 0.5f, uiSpace.y * 0.5f};
		tl.pos = {-tr.pos.x, tr.pos.y};
		bl.pos = {-tr.pos.x, -tr.pos.y};
		br.pos = {tr.pos.x, -tr.pos.y};
		draw2DQuads({tl, tr, bl, br});

		drawLight(light0Pos, light0, state);
		drawLight(light1Pos, light1, state);
		context::swapBuffers();
		context::pollEvents();
	}

	prop0.clearModels();
}
} // namespace

s32 gameloop::run(s32 argc, char** argv)
{
	le::env::init(argc, argv);
	FileLogger fileLogger(env::fullPath("debug.log"));

	constexpr u16 WIDTH = 1280;
	constexpr u16 HEIGHT = 720;

#if defined(DEBUGGING)
	context::g_bVSYNC = false;
#endif
	if (!context::create(WIDTH, HEIGHT, "Test"))
	{
		return 1;
	}
	runTest();
	context::destroy();
	return 0;
}
} // namespace letest
