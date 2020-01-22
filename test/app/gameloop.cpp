#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include "le3d/core/log.hpp"
#include "le3d/core/io.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/input.hpp"
#include "le3d/gfx/draw.hpp"
#include "le3d/gfx/model.hpp"
#include "le3d/gfx/primitives.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/gfx/vram.hpp"
#include "le3d/game/asyncLoaders.hpp"
#include "le3d/game/camera.hpp"
#include "le3d/game/entity.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/game/utils.hpp"

#include "ubotypes.hpp"
#include "gameloop.hpp"

namespace letest
{
using namespace le;

namespace
{
void runTest()
{
	stdfs::path const resources = stdfs::path(env::dirPath(env::Dir::Executable)).parent_path() / "test/resources";
	std::unique_ptr<IOReader> uReader;
	if (std::filesystem::is_regular_file(resources / "resources.zip"))
	{
		uReader = std::make_unique<ZIPReader>(resources / "resources.zip");
		LOG_I("[GameLoop] Using ZIP archive");
	}
	else
	{
		uReader = std::make_unique<FileReader>(resources);
		LOG_I("[GameLoop] Using Filesystem");
	}

	FreeCam camera;
	camera.setup("freecam");
	// input::setCursorMode(CursorMode::Disabled);
	camera.m_position = {0.0f, 0.0f, 3.0f};
	camera.m_flags.set((s32)FreeCam::Flag::FixedSpeed, false);

	AsyncTexturesLoader::Request texturesRequest = {
		"textures",
		"",
		{{"textures/container2.png"}, {"textures/container2_specular.png", TexType::Specular}, {"textures/awesomeface.png"}},
		uReader.get()};
	AsyncTexturesLoader loader(std::move(texturesRequest));
	loader.waitAll();
	loader.loadNext(3);

	FontAtlasData scpSheet;
	scpSheet.bytes = uReader->getBytes("fonts/scp_1024x512.png");
	scpSheet.deserialise(uReader->getString("fonts/scp_1024x512.json"));
	auto& hFont = resources::loadFont("default", std::move(scpSheet));

	auto& hMatricesUBO = resources::addUBO("Matrices", sizeof(uboData::Matrices), uboData::Matrices::s_bindingPoint, DrawType::Dynamic);
	auto& hLightsUBO = resources::addUBO("Lights", sizeof(uboData::Lights), uboData::Lights::s_bindingPoint, DrawType::Dynamic);
	auto samplerList = uReader->getString(resources / "samplers.json");
	resources::addSamplers(samplerList);
	resources::ShaderIDMap shaderIDMap = {
		{"unlit/tinted", {"shaders/default.vsh", "shaders/unlit/tinted.fsh"}},
		{"unlit/textured", {"shaders/default.vsh", "shaders/unlit/textured.fsh"}},
		{"lit/tinted", {"shaders/default.vsh", "shaders/lit/tinted.fsh"}},
		{"lit/textured", {"shaders/default.vsh", "shaders/lit/textured.fsh"}},
		{"ui/textured", {"shaders/ui.vsh", "shaders/unlit/textured.fsh"}},
		{"ui/tinted", {"shaders/ui.vsh", "shaders/unlit/tinted.fsh"}},
		{"unlit/skybox", {"shaders/skybox.vsh", "shaders/unlit/skyboxed.fsh"}},
		{"monolithic", {"shaders/monolithic.vsh", "shaders/monolithic.fsh"}},
	};
	resources::loadShaders(shaderIDMap, *uReader);
	auto& litTinted = resources::get<HShader>("lit/tinted");
	auto& litTextured = resources::get<HShader>("lit/textured");
	auto& uiTextured = resources::get<HShader>("ui/textured");
	auto& monolithic = resources::get<HShader>("monolithic");
	litTinted.setV4(env::g_config.uniforms.tint, Colour::Yellow);

#if defined(DEBUGGING)
	Entity::s_gizmoShader = monolithic;
#endif
	Skybox skybox;
	AsyncSkyboxLoader::Request skyboxRequest = {
		"skybox", "textures/skybox", {"right.jpg", "left.jpg", "up.jpg", "down.jpg", "front.jpg", "back.jpg"}, uReader.get()};
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
		gfx::draw(light);
	};

	bool bModelsSwapped = false;
	stdfs::path const modelsRoot = "models";
	stdfs::path const model0Path = "test/fox";
	stdfs::path const model1Path = "plant";
	stdfs::path const model2Path = "";
	AsyncModelsLoader::Request modelsRequest = {"models", "models", {model0Path, model1Path}, uReader.get()};
	if (!model2Path.empty())
	{
		modelsRequest.resources.push_back(model2Path);
	}
	AsyncModelsLoader modelsLoader(std::move(modelsRequest));
	Mesh cubeMeshTexd = debug::Cube();
	Mesh quadMesh = debug::Quad();
	quadMesh.m_material.textures = {resources::get<HTexture>("textures/awesomeface.png")};
	/*HTexture bad;
	quadMesh.m_material.textures = {bad};*/
	cubeMeshTexd.m_material.textures = {resources::get<HTexture>("textures/container2.png"),
										resources::get<HTexture>("textures/container2_specular.png")};
	cubeMeshTexd.m_material.albedo = {glm::vec3(0.4f), glm::vec3(0.5f), glm::vec3(1.0f)};
	// cubeMesh.textures = {resources::getTexture("textures/container2.png")};
	Mesh blankCubeMesh = cubeMeshTexd;
	blankCubeMesh.m_material.textures.clear();
	blankCubeMesh.m_material.flags.set(s32(Material::Flag::Textured), false);
	Mesh sphereMesh = gfx::createCubedSphere(1.0f, "testSphere", 8, {});
	sphereMesh.m_material = cubeMeshTexd.m_material;

	Mesh& instanceMesh = sphereMesh;
	constexpr s32 instanceSide = 4;
	constexpr size_t instanceCount = size_t(instanceSide * instanceSide * 4);
	std::vector<glm::mat4> instanceMats;
	instanceMats.reserve(instanceCount);
	for (s32 row = -instanceSide; row < instanceSide; ++row)
	{
		for (s32 col = -instanceSide; col < instanceSide; ++col)
		{
			glm::vec3 pos{row * 2.0f, col * 2.0f, -2.0f};
			instanceMats.emplace_back(glm::translate(glm::mat4(1.0f), pos));
		}
	}
	gfx::VBODescriptor descriptor;
	descriptor.attribCount = instanceCount;
	descriptor.attribLocation = 5;
	descriptor.vec4sPerAttrib = 4;
	HVBO instanceVBO = gfx::genVec4VBO(descriptor, {instanceMesh.m_hVerts.hVAO});
	gfx::setVBO(instanceVBO, instanceMats.data());

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

	HVerts light0 = gfx::tutorial::newLight(cubeMeshTexd.m_hVerts);
	HVerts light1 = gfx::tutorial::newLight(cubeMeshTexd.m_hVerts);

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

	auto tOnInput = input::registerInput([&](s32 key, s32 action, s32 mods) {
		switch (action)
		{
		default:
			break;

		case GLFW_RELEASE:
		{
			if (key == GLFW_KEY_ESCAPE)
			{
				context::close();
				return;
			}
			break;
		}
		case GLFW_PRESS:
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
			break;
		}
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

	while (context::isAlive())
	{
		dt = Time::elapsed() - t;
		t = Time::elapsed();
		camera.tick(dt);
		// context::glClearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, Colour(50, 40, 10));
		context::clearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!bModelsSwapped)
		{
			if (modelsLoader.loadNext() == AsyncLoadState::Idle)
			{
				bModelsSwapped = true;
				prop0.clearModels();
				prop0.addModel(resources::get<Model>("models/" + model0Path.string()));
				props[4].clearModels();
				props[4].addModel(resources::get<Model>("models/" + model1Path.string()));
				if (resources::isLoaded<Model>("models/" + model2Path.string()))
				{
					auto& model2 = resources::get<Model>("models/" + model2Path.string());
					props[0].clearModels();
					props[0].addModel(model2);
				}
			}
		}
		if (skybox.hCube.byteCount == 0)
		{
			if (skyboxLoader.loadNext() == AsyncLoadState::Idle)
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
		// renderMeshes(instanceMesh, monolithic, instanceCount);
		if (bWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		drawLight(pl0Pos, light0);
		drawLight(pl1Pos, light1);
		quadProp.render();

		debug::Quad2D tl, tr, bl, br;
		HTexture& quadTex = resources::get<HTexture>("textures/awesomeface.png");
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
	gfx::releaseMesh(sphereMesh);
	modelsLoader.waitAll();
	resources::destroySkybox(skybox);
}
} // namespace

s32 gameloop::run(s32 argc, char const** argv)
{
	context::Settings settings;
	settings.window.title = "LE3D Test";
	settings.window.bVSYNC = false;
	// settings.window.width = 3000;
	settings.env.args = {argc, argv};
	settings.env.jobWorkerCount = 4;
	// settings.type = context::Type::BorderlessFullscreen;
	if (auto uContext = context::create(settings))
	{
		runTest();
		return 0;
	}
	return 1;
}
} // namespace letest
