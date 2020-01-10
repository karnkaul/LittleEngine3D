#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include "le3d/engineVersion.hpp"
#include "le3d/context/context.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/gdata.hpp"
#include "le3d/core/jobs.hpp"
#include "le3d/core/maths.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/env/env.hpp"
#include "le3d/env/threads.hpp"
#include "le3d/game/asyncLoader.hpp"
#include "le3d/game/camera.hpp"
#include "le3d/game/entity.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/gfx/model.hpp"
#include "le3d/gfx/primitives.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/input/input.hpp"

#include "ubotypes.hpp"
#include "gameloop.hpp"

namespace letest
{
using namespace le;
using namespace le::utils;

namespace
{
OnInput::Token tOnInput;
stdfs::path const resourcesPath = "../test/resources";

stdfs::path resourcePath(stdfs::path const& id)
{
	return env::dirPath(env::Dir::Executable) / resourcesPath / id;
}

std::stringstream fileToStream(stdfs::path const& id)
{
	return utils::readFile(resourcePath(id));
}

bytestream fileToBytes(stdfs::path const& id)
{
	return utils::readBytes(resourcePath(id));
}

void runTest()
{
	FreeCam camera;
	camera.setup("freecam");
	// input::setCursorMode(CursorMode::Disabled);
	camera.m_position = {0.0f, 0.0f, 3.0f};
	camera.m_flags.set((s32)FreeCam::Flag::FixedSpeed, false);

	resources::loadTexture("container2", TexType::Diffuse, readBytes(resourcePath("textures/container2.png")), false);
	resources::loadTexture("container2_specular", TexType::Specular, readBytes(resourcePath("textures/container2_specular.png")), false);
	resources::loadTexture("awesomeface", TexType::Diffuse, readBytes(resourcePath("textures/awesomeface.png")), false);

	FontAtlasData scpSheet;
	scpSheet.bytes = readBytes(resourcePath("fonts/scp_1024x512.png"));
	scpSheet.deserialise(readFile(resourcePath("fonts/scp_1024x512.json")).str());
	auto& hFont = resources::loadFont("default", std::move(scpSheet));

	auto& hMatricesUBO = resources::addUBO("Matrices", sizeof(uboData::Matrices), uboData::Matrices::s_bindingPoint, gfx::Draw::Dynamic);
	auto& hLightsUBO = resources::addUBO("Lights", sizeof(uboData::Lights), uboData::Lights::s_bindingPoint, gfx::Draw::Dynamic);

	auto def = readFile(resourcePath("shaders/default.vsh")).str();
	auto ui = readFile(resourcePath("shaders/ui.vsh")).str();
	auto sb = readFile(resourcePath("shaders/skybox.vsh")).str();
	/*auto& unlitTinted = */ resources::loadShader("unlit/tinted", def, readFile(resourcePath("shaders/unlit/tinted.fsh")).str());
	resources::loadShader("unlit/textured", def, readFile(resourcePath("shaders/unlit/textured.fsh")).str());
	auto& litTinted = resources::loadShader("lit/tinted", def, readFile(resourcePath("shaders/lit/tinted.fsh")).str());
	auto& litTextured = resources::loadShader("lit/textured", def, readFile(resourcePath("shaders/lit/textured.fsh")).str());
	auto& uiTextured = resources::loadShader("ui/textured", ui, readFile(resourcePath("shaders/unlit/textured.fsh")).str());
	/*auto& uiTinted = */ resources::loadShader("ui/tinted", ui, readFile(resourcePath("shaders/unlit/tinted.fsh")).str());
	/*auto& skyboxShader = */ resources::loadShader("unlit/skybox", sb, readFile(resourcePath("shaders/unlit/skyboxed.fsh")).str());
	auto& monolithic = resources::loadShader("monolithic", readFile(resourcePath("shaders/monolithic.vsh")).str(),
											 readFile(resourcePath("shaders/monolithic.fsh")).str());
	litTinted.setV4(env::g_config.uniforms.tint, Colour::Yellow);

#if defined(DEBUGGING)
	Entity::s_gizmoShader = monolithic;
#endif
	Skybox skybox;
	ResourceLoadRequest skyboxRequest;
	skyboxRequest.getBytes = &fileToBytes;
	skyboxRequest.idPrefix = "textures/skybox";
	skyboxRequest.resourceIDs = {"left.jpg", "right.jpg", "up.jpg", "down.jpg", "front.jpg", "back.jpg"};
	AsyncSkyboxLoader skyboxLoader(std::move(skyboxRequest));
	/*skyboxLoader.waitAll();
	ASSERT(skyboxLoader.loadNext(), "Skybox not loaded!");
	skybox = std::move(skyboxLoader.m_skybox);*/

	glm::vec3 pl0Pos(0.0f, 0.0f, 2.0f);
	glm::vec3 pl1Pos(-4.0f, 2.0f, 2.0f);
	uboData::Lights lights;
	lights.ptLights[0].position = glm::vec4(pl0Pos, 0.0f);
	lights.ptLights[1].position = glm::vec4(pl1Pos, 0.0f);
	lights.dirLights[0].direction = glm::vec4(-1.0f, -1.0f, 1.0f, 0.0f);
	lights.ptLights[0].clq = lights.ptLights[1].clq = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);
	lights.ptLights[0].ambient = lights.ptLights[1].ambient = lights.dirLights[0].ambient = glm::vec4(0.2f);
	lights.ptLights[0].diffuse = lights.ptLights[1].diffuse = lights.dirLights[0].diffuse = glm::vec4(0.5f);
	lights.ptLights[0].specular = lights.ptLights[1].specular = lights.dirLights[0].specular = glm::vec4(1.0f);

	auto drawLight = [](glm::vec3 const& pos, HVerts light) {
		ModelMats mats;
		mats.model = glm::translate(mats.model, pos);
		mats.oNormals = mats.model = glm::scale(mats.model, glm::vec3(0.1f));
		auto const& tinted = resources::get<HShader>("unlit/tinted");
		tinted.setV4(env::g_config.uniforms.tint, Colour::White);
		tinted.setModelMats(mats);
		gfx::gl::draw(light);
	};

	bool bModelsSwapped = false;
	stdfs::path const modelsRoot = "models";
	stdfs::path const model0Path = "fox";
	stdfs::path const model1Path = "plant";
	stdfs::path const model2Path = "";
	ResourceLoadRequest rlRequest;
	rlRequest.idPrefix = "models";
	rlRequest.resourceIDs = {model0Path, model1Path};
	if (!model2Path.empty())
	{
		rlRequest.resourceIDs.push_back(model2Path);
	}
	rlRequest.getData = &fileToStream;
	rlRequest.getBytes = &fileToBytes;
	AsyncModelsLoader modelsLoader(std::move(rlRequest));
	HMesh cubeMeshTexd = debug::Cube();
	HMesh quadMesh = debug::Quad();
	quadMesh.material.textures = {resources::get<HTexture>("awesomeface")};
	// quadMesh.textures = {bad};
	cubeMeshTexd.material.textures = {resources::get<HTexture>("container2"), resources::get<HTexture>("container2_specular")};
	cubeMeshTexd.material.albedo = {glm::vec3(0.4f), glm::vec3(0.5f), glm::vec3(1.0f)};
	// cubeMesh.textures = {resources::getTexture("container2")};
	HMesh blankCubeMesh = cubeMeshTexd;
	blankCubeMesh.material.textures.clear();
	blankCubeMesh.material.flags.set(s32(Material::Flag::Textured), false);
	HMesh sphereMesh = gfx::createCubedSphere(1.0f, "testSphere", 8, {});
	sphereMesh.material = cubeMeshTexd.material;

	Model cube;
	Model blankCube;
	Model cubeStack;
	cube.setupModel("cube");
	cubeStack.setupModel("cubeStack");
	blankCube.setupModel("blankCube");
	cube.addFixture(cubeMeshTexd);
	cubeStack.addFixture(cubeMeshTexd);
	blankCube.addFixture(blankCubeMesh);
	glm::mat4 offset(1.0f);
	offset = glm::translate(offset, glm::vec3(0.0f, 2.0f, 0.0f));
	cubeStack.addFixture(cubeMeshTexd, offset);
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
	// prop0.setShader(resources::get<HShader>("lit/textured"));
	prop0.setShader(monolithic);

	Prop prop1;
	prop1.setup("prop1");
	prop1.addModel(blankCube);
	prop1.m_transform.setPosition({0.5f, -0.5f, -0.5f});
	prop1.m_transform.setScale(0.25f);
	prop0.m_transform.setParent(&prop1.m_transform);
	// prop1.setShader(resources::get<HShader>("lit/tinted"));
	prop1.setShader(monolithic);
	prop1.m_oTintOverride = Colour::Yellow;

	Prop quadProp;
	quadProp.setup("quad");
	quadProp.addModel(quad);
	// quadProp.setShader(resources::get<HShader>("unlit/textured"));
	quadProp.setShader(monolithic);
	quadProp.m_transform.setPosition(glm::vec3(-2.0f, 2.0f, -2.0f));

	HVerts light0 = gfx::tutorial::newLight(cubeMeshTexd.hVerts);
	HVerts light1 = gfx::tutorial::newLight(cubeMeshTexd.hVerts);

	std::vector<Prop> props;
	for (s32 i = 0; i < 5; ++i)
	{
		Prop prop;
		prop.setup("prop_" + std::to_string(i));
		Model& m = cube;
		prop.addModel(m);
		std::string shader = "monolithic";
		prop.setShader(resources::get<HShader>(shader));
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
				props[3].m_flags.set((s32)Entity::Flag::Wireframe, bWireframe);
			}
			if (key == GLFW_KEY_P && mods & GLFW_MOD_CONTROL)
			{
				bParented = !bParented;
				prop0.m_transform.setParent(bParented ? &prop1.m_transform : nullptr);
			}
#if defined(DEBUGGING)
			if (key == GLFW_KEY_G && mods & GLFW_MOD_CONTROL)
			{
				auto& ar = debug::Arrow();
				auto tip = debug::DArrow::Tip::Sphere;
				switch (ar.m_tip)
				{
				case debug::DArrow::Tip::Cone:
					tip = debug::DArrow::Tip::Sphere;
					break;

				case debug::DArrow::Tip::Cube:
					tip = debug::DArrow::Tip::Cone;
					break;

				case debug::DArrow::Tip::Sphere:
					tip = debug::DArrow::Tip::Cube;
					break;
				}
				ar.setTip(tip);
			}
#endif
		}
		if (key == GLFW_MOUSE_BUTTON_1)
		{
			LOG_D("Mouse button 0 [pressed/released] [%d/%d]!", action == GLFW_PRESS, action == GLFW_RELEASE);
		}
	});
#if defined(DEBUGGING)
	auto test = input::registerFiledrop([](stdfs::path const& path) { LOG_D("File path: %s", path.generic_string().data()); });
#endif
	Time dt;
	Time t = Time::elapsed();

	glm::vec3 uiSpace(1920.0f, 1080.0f, 2.0f);
	f32 uiAR = uiSpace.x / uiSpace.y;

	while (!context::isClosing())
	{
		dt = Time::elapsed() - t;
		t = Time::elapsed();
		camera.tick(dt);
		// context::glClearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, Colour(50, 40, 10));
		context::clearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!bModelsSwapped)
		{
			if (modelsLoader.loadNext())
			{
				bModelsSwapped = true;
				prop0.clearModels();
				prop0.addModel(resources::get<Model>("models/" + model0Path.string()));
				props[4].clearModels();
				props[4].addModel(resources::get<Model>("models/" + model1Path.string()));
				if (resources::isLoaded<Model>("models/" + model2Path.string()))
				{
					props[0].clearModels();
					props[0].addModel(resources::get<Model>("models/" + model2Path.string()));
				}
			}
		}
		if (skybox.cubemap.byteCount == 0)
		{
			if (skyboxLoader.loadNext())
			{
				skybox = std::move(skyboxLoader.m_skybox);
			}
		}

		prop0.m_transform.setOrientation(
			glm::rotate(prop0.m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(1.0f, 0.3f, 0.5f)));
		prop1.m_transform.setOrientation(
			glm::rotate(prop1.m_transform.orientation(), glm::radians(dt.assecs() * 10), glm::vec3(1.0f, 0.3f, 0.5f)));
		quadProp.m_transform.setOrientation(
			glm::rotate(prop1.m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(0.3f, 0.5f, 1.0f)));

		gfx::setUBO(hLightsUBO, lights);
		glm::mat4 v = camera.view();
		glm::mat4 p = camera.perspectiveProj();
		glm::vec4 c(camera.m_position, 0.0f);
		// uiSpace = glm::vec3(context::size(), 2.0f);
		uboData::Matrices mats{v, p, p * v, camera.uiProj(uiSpace), c};
		gfx::setUBO(hMatricesUBO, mats);

		renderSkybox(skybox, resources::get<HShader>("unlit/skybox"));

		prop0.render();
		prop1.render();
		std::vector<ModelMats> m(3, ModelMats());
		for (size_t i = 0; i < 3; ++i)
		{
			auto& prop = props[i];
			// prop.setShader(resources::get<HShader>("lit/textured"));
			m[i].model = prop.m_transform.model();
			m[i].oNormals = prop.m_transform.normalModel();
			prop.render();
		}
		litTextured.setV4(env::g_config.uniforms.tint, Colour::White);
		// auto const& cube = debug::debugCube();
		// renderMeshes(cube, m, litTextured);
		m = std::vector<ModelMats>(props.size() - 3, ModelMats());
		for (size_t i = 3; i < props.size(); ++i)
		{
			auto& prop = props[i];
			m[i - 3].model = prop.m_transform.model();
			m[i - 3].oNormals = prop.m_transform.normalModel();
			prop.render();
		}
		// renderMeshes(cube, m, litTinted);
		ModelMats sphereMat;
		sphereMat.model = glm::translate(sphereMat.model, {2.0f, -0.5f, 0.0f});
		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		renderMeshes(sphereMesh, {sphereMat}, monolithic);
		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		drawLight(pl0Pos, light0);
		drawLight(pl1Pos, light1);
		quadProp.render();

		debug::Quad2D tl, tr, bl, br;
		HTexture& quadTex = resources::get<HTexture>("awesomeface");
		tl.size = tr.size = bl.size = br.size = {200.0f, 200.0f};
		tr.pos = {uiSpace.x * 0.5f, uiSpace.y * 0.5f, 0.0f};
		tl.pos = {-tr.pos.x, tr.pos.y, 0.0f};
		bl.pos = {-tr.pos.x, -tr.pos.y, 0.0f};
		br.pos = {tr.pos.x, -tr.pos.y, 0.0f};
		debug::draw2DQuads({tl, tr, bl, br}, quadTex, monolithic, uiAR);

		debug::Text2D text;
		text.text = "Hello World!";
		text.align = debug::Text2D::Align::Centre;
		text.height = 100.0f;
		text.pos = glm::vec3(0.0f, 300.0f, 0.0f);
		debug::renderString(text, uiTextured, hFont, uiAR);

		debug::renderFPS(hFont, monolithic, uiAR);
		debug::renderVersion(hFont, uiTextured, uiAR);

		context::swapBuffers();
		context::pollEvents();
	}
	gfx::releaseMeshes({&sphereMesh});
	modelsLoader.waitAll();
	resources::destroySkybox(skybox);
}
} // namespace

s32 gameloop::run(s32 argc, char const** argv)
{
	context::Settings settings;
	settings.title = "LE3D Test";
	settings.bVSYNC = false;
	settings.env.args = {argc, argv};
	settings.env.jobWorkerCount = 4;
	// settings.type = context::Type::BorderlessFullscreen;
	if (auto wContext = context::create(settings))
	{
		runTest();
		return 0;
	}
	return 1;
}
} // namespace letest
