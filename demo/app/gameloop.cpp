#include "le3d/core/maths.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/io.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/input.hpp"
#include "le3d/engine/gfx/draw.hpp"
#include "le3d/engine/gfx/model.hpp"
#include "le3d/engine/gfx/primitives.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/engine/gfx/vram.hpp"
#include "le3d/game/asyncLoaders.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/game/ecs.hpp"

#include "ubotypes.hpp"
#include "gameloop.hpp"

namespace letest
{
using namespace le;

namespace
{
void runTest()
{
	stdfs::path const resources = stdfs::path(env::dirPath(env::Dir::Executable)).parent_path() / "demo/resources";
	stdfs::path const resourcesZip = resources.parent_path() / "resources.zip";
	std::unique_ptr<IOReader> uReader;
	if (std::filesystem::is_regular_file(resourcesZip))
	{
		uReader = std::make_unique<ZIPReader>(resourcesZip, "resources");
		LOG_I("[GameLoop] Using ZIP archive");
	}
	else
	{
		uReader = std::make_unique<FileReader>(resources);
		LOG_I("[GameLoop] Using Filesystem");
	}

	AsyncTexturesLoader::Request texturesRequest = {
		"textures",
		"",
		{{"textures/container2.png"}, {"textures/container2_specular.png", TexType::Specular}, {"textures/awesomeface.png"}},
		uReader.get()};
	AsyncTexturesLoader loader(std::move(texturesRequest));
	loader.waitAll();
	loader.loadNext(3);

	auto& hMatricesUBO = resources::addUBO("Matrices", sizeof(uboData::Matrices), uboData::Matrices::s_bindingPoint, DrawType::Dynamic);
	auto& hLightsUBO = resources::addUBO("Lights", sizeof(uboData::Lights), uboData::Lights::s_bindingPoint, DrawType::Dynamic);

	auto manifestJSON = GData(uReader->getString("manifest.json"));
	resources::addSamplers(manifestJSON);
	resources::loadShaders(manifestJSON, *uReader);
	resources::loadFonts(manifestJSON, *uReader);

	auto& litTinted = resources::get<HShader>("lit/tinted");
	auto& uiTextured = resources::get<HShader>("ui/textured");
	auto& monolithic = resources::get<HShader>("monolithic");
	litTinted.setV4(env::g_config.uniforms.tint, Colour::Yellow);
	auto& font = resources::get<BitmapFont>("default");

#if defined(DEBUGGING)
	CGizmo::s_gizmoShader = monolithic;
#endif
	AsyncSkyboxLoader::Request skyboxRequest = {
		"skybox", "textures/skybox", {"right.jpg", "left.jpg", "up.jpg", "down.jpg", "front.jpg", "back.jpg"}, uReader.get()};
	AsyncSkyboxLoader skyboxLoader(std::move(skyboxRequest));

	glm::vec3 pl0Pos(0.0f, 0.0f, 2.0f);
	glm::vec3 pl1Pos(-4.0f, 2.0f, 2.0f);
	uboData::Lights lights;
	lights.ptLights[0].position = glm::vec4(pl0Pos, 0.0f);
	lights.ptLights[1].position = glm::vec4(pl1Pos, 0.0f);
	lights.dirLights[0].direction = glm::vec4(-1.0f, -1.0f, 1.0f, 0.0f);
	lights.ptLights[0].clq = lights.ptLights[1].clq = glm::vec4(1.0f, 0.09f, 0.032f, 0.0f);
	lights.ptLights[0].ambient = lights.ptLights[1].ambient = lights.dirLights[0].ambient = glm::vec4(glm::vec3(0.2f), 1.0f);
	lights.ptLights[0].diffuse = lights.ptLights[1].diffuse = lights.dirLights[0].diffuse = glm::vec4(glm::vec3(0.5f), 1.0f);
	lights.ptLights[0].specular = lights.ptLights[1].specular = lights.dirLights[0].specular = glm::vec4(1.0f);

	auto drawLight = [](glm::vec3 const& pos, HVerts const& light, glm::vec4 const& diffuse) {
		ModelMats mats;
		mats.model = glm::translate(mats.model, pos);
		mats.oNormals = mats.model = glm::scale(mats.model, glm::vec3(0.2f));
		auto const& tinted = resources::get<HShader>("unlit/tinted");
		tinted.setV4(env::g_config.uniforms.tint, diffuse);
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
	blankCubeMesh.m_material.flags.set(Material::Flag::Textured, false);
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
			instanceMats.push_back(glm::translate(glm::mat4(1.0f), pos));
		}
	}
	descriptors::VBO vboDesc;
	vboDesc.attribCount = instanceCount;
	vboDesc.attribLocation = 5;
	vboDesc.vec4sPerAttrib = 4;
	HVBO instanceVBO = gfx::genVec4VBO(vboDesc, {instanceMesh.m_hVerts.hVAO});
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
	[[maybe_unused]] static bool bParented = true;
	[[maybe_unused]] static bool bPaused = false;

	ECSDB ecsdb;
	ecsdb.addSystem<FreeCamController, PropRenderer, SkyboxRenderer>();
#if defined(DEBUGGING)
	ecsdb.addSystem<GizmoSystem>();
#endif

	std::vector<ecs::SpawnID> entities;

	/*constexpr s32 stress = 1000;
	constexpr s32 noise = 500;
	for (s32 i = 0; i < stress; ++i)
	{
		auto id = ecsdb.spawnEntity<CTransform, CProp>("Stress_" + std::to_string(i));
		entities.push_back(id);
	}
	for (s32 i = 0; i < noise; ++i)
	{
		auto id = ecsdb.spawnEntity<CTransform>("Noise_" + std::to_string(i));
		entities.push_back(id);
	}*/

	auto eCamera = ecsdb.spawnEntity("camera");
	CFreeCam* pCamera = nullptr;
	if (auto pCam = ecsdb.getEntity(eCamera))
	{
		pCamera = pCam->addComponent<CFreeCam>();
		pCamera->m_position = {0.0f, 0.0f, 3.0f};
		pCamera->m_flags.set(CFreeCam::Flag::FixedSpeed, false);
		pCamera->m_flags.set(CFreeCam::Flag::Enabled, true);
		entities.push_back(eCamera);
	}

	auto eSkybox = ecsdb.spawnEntity<CSkybox>("skybox");
	if (auto pSkybox = ecsdb.getComponent<CSkybox>(eSkybox))
	{
		pSkybox->m_shader = resources::get<HShader>("unlit/skybox");
		entities.push_back(eSkybox);
	}

	ecs::SpawnID eProp1;
	if (auto pProp1 = spawnProp(ecsdb, "prop1", monolithic))
	{
		pProp1->m_models = {&blankCube};
		pProp1->getComponent<CTransform>()->m_transform.setPosition({0.5f, -0.5f, -0.5f}).setScale(0.25f);
		pProp1->m_oTint = Colour::Yellow;
		eProp1 = pProp1->getOwner()->spawnID();
		entities.push_back(eProp1);
	}

	ecs::SpawnID eProp0;
	if (auto pProp0 = spawnProp(ecsdb, "awesome-container", monolithic))
	{
		pProp0->m_models = {&cubeStack};
		if (auto pProp1 = ecsdb.getComponent<CTransform>(entities.back()))
		{
			pProp0->getComponent<CTransform>()->m_transform.setPosition({2.0f, 2.5f, -2.0f}).setScale(2.0f).setParent(&pProp1->m_transform);
		}
		eProp0 = pProp0->getOwner()->spawnID();
		entities.push_back(eProp0);
	}

	ecs::SpawnID eQuad;
	if (auto pQuad = spawnProp(ecsdb, "quad", monolithic))
	{
		pQuad->m_models = {&quad};
		eQuad = pQuad->getOwner()->spawnID();
		pQuad->getComponent<CTransform>()->m_transform.setPosition(glm::vec3(-2.0f, 2.0f, -2.0f));
		entities.push_back(eQuad);
	}

	HVerts lightVerts = gfx::tutorial::newLight(sphereMesh.m_hVerts);

	ecs::SpawnID eProps_0, eProps_3, eProps_4;
	std::vector<CProp*> props;
	for (s32 i = 0; i < 5; ++i)
	{
		if (auto pC = spawnProp(ecsdb, "prop_" + std::to_string(i), monolithic))
		{
			pC->m_models = {&cube};
			props.push_back(pC);
			entities.push_back(pC->getOwner()->spawnID());
			if (i == 0)
			{
				eProps_0 = pC->getOwner()->spawnID();
			}
			else if (i == 3)
			{
				eProps_3 = pC->getOwner()->spawnID();
			}
			else if (i == 4)
			{
				eProps_4 = pC->getOwner()->spawnID();
			}
		}
	}
	props[0]->getComponent<CTransform>()->m_transform.setPosition({-0.5f, 0.5f, -4.0f});
	props[1]->getComponent<CTransform>()->m_transform.setPosition({-0.5f, 1.5f, 4.0f});
	props[2]->getComponent<CTransform>()->m_transform.setPosition({1.5f, -0.5f, 4.0f});
	props[3]->getComponent<CTransform>()->m_transform.setPosition({-3.0f, -1.0f, 2.0f});
	props[4]->getComponent<CTransform>()->m_transform.setPosition({-4.0f, 1.0f, -2.0f});
	std::unordered_set<Key> held;

	auto tOnInput = input::registerInput([&](Key key, Action action, Mods mods) {
		switch (action)
		{
		default:
			break;

		case Action::RELEASE:
		{
			held.erase(key);
			if (key == Key::ESCAPE)
			{
				context::close();
				return;
			}
			else if (key == Key::V && mods & Mods::CONTROL)
			{
				context::setSwapInterval(context::swapInterval() == 0 ? 1 : 0);
			}
			break;
		}
		case Action::PRESS:
		{
			held.insert(key);
			if (key == Key::W && mods & Mods::CONTROL)
			{
				bWireframe = !bWireframe;
				if (auto pProp0 = ecsdb.getComponent<CProp>(eProp0))
				{
					pProp0->m_flags.set(CProp::Flag::Wireframe, bWireframe);
				}
				if (auto pProps_3 = ecsdb.getComponent<CProp>(eProps_3))
				{
					pProps_3->m_flags.set(CProp::Flag::Wireframe, bWireframe);
				}
			}
			if (key == Key::P && mods & Mods::CONTROL)
			{
				/*bParented = !bParented;
				if (auto pProp0 = ecsdb.getComponent<CTransform>(eProp0))
				{
					if (auto pProp1 = ecsdb.getComponent<CTransform>(eProp1))
					{
						pProp0->m_transform.setParent(bParented ? &pProp1->m_transform : nullptr);
					}
				}*/
				bPaused = !bPaused;
				ecsdb.setAll(System::Flag::Ticking, !bPaused);
				auto uCamera = ecsdb.getSystem<FreeCamController>();
				uCamera->setFlag(System::Flag::Ticking, true);
			}
#if defined(DEBUGGING)
			if (key == Key::G && mods & Mods::CONTROL)
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
		if (key == Key::MOUSE_BUTTON_1)
		{
			LOG_D("Mouse button 0 [pressed/released] [%d/%d]!", action == Action::PRESS, action == Action::RELEASE);
		}
	});
#if defined(DEBUGGING)
	auto test = input::registerFiledrop([](stdfs::path const& path) { LOG_D("File path: %s", path.generic_string().data()); });
#endif
	Time dt;
	Time t = Time::elapsed();

	glm::vec3 uiSpace(1920.0f, 1080.0f, 2.0f);
	f32 uiAR = uiSpace.x / uiSpace.y;

	Rect2 view = Rect2::sizeCentre(context::windowSize() * 0.75f);
	//gfx::setView(view);
	auto onWindowSizeChange = input::registerResize([&](s32, s32) {
		view = Rect2::sizeCentre(context::windowSize() * 0.75f, view.centre());
		pCamera->m_aspectRatio = context::windowAspect();
		//gfx::setView(view);
	});

	auto midRender = [&](ECSDB const&) {
		ModelMats sphereMat;
		sphereMat.model = glm::translate(sphereMat.model, {2.0f, -0.5f, 0.0f});
		if (bWireframe)
		{
			context::setPolygonMode(context::PolygonMode::Line);
		}
		renderMeshes(sphereMesh, {sphereMat}, monolithic);
		// renderMeshes(instanceMesh, monolithic, instanceCount);
		context::setPolygonMode(context::PolygonMode::Fill);
	};
	auto midRenderHandle = ecsdb.addRenderSlot(midRender, 10);
	auto midTick = [eProp0, eProp1, eQuad](ECSDB& ecsdb, Time dt) {
		if (auto pProp0 = ecsdb.getComponent<CTransform>(eProp0))
		{
			pProp0->m_transform.setOrientation(
				glm::rotate(pProp0->m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(1.0f, 0.3f, 0.5f)));
		}
		if (auto pProp1 = ecsdb.getComponent<CTransform>(eProp1))
		{
			pProp1->m_transform.setOrientation(
				glm::rotate(pProp1->m_transform.orientation(), glm::radians(dt.assecs() * 10), glm::vec3(1.0f, 0.3f, 0.5f)));
			if (auto pQuad = ecsdb.getComponent<CTransform>(eQuad))
			{
				pQuad->m_transform.setOrientation(
					glm::rotate(pProp1->m_transform.orientation(), glm::radians(dt.assecs() * 30), glm::vec3(0.3f, 0.5f, 1.0f)));
			}
		}
	};
	auto midTickHandle = ecsdb.addTickSlot(midTick, 10);

	context::ClearFlags clearFlags;
	clearFlags.set({context::ClearFlag::ColorBuffer, context::ClearFlag::DepthBuffer}, true);
	while (context::isAlive())
	{
		dt = Time::elapsed() - t;
		t = Time::elapsed();
		context::clearFlags(clearFlags);

		/*bool bDraggingView = held.find(Key::LEFT_CONTROL) != held.end() && held.find(Key::MOUSE_BUTTON_1) != held.end();
		if (bDraggingView)
		{
			view = Rect2::sizeCentre(view.size(), input::cursorPos());
			gfx::setView(view);
		}*/

		if (!bModelsSwapped)
		{
			if (modelsLoader.loadNext() == AsyncLoadState::Idle)
			{
				bModelsSwapped = true;
				if (auto pProp0 = ecsdb.getComponent<CProp>(eProp0))
				{
					if (auto pProps_4 = ecsdb.getComponent<CProp>(eProps_4))
					{
						pProp0->m_models = {&resources::get<Model>("models/" + model0Path.string())};
						pProps_4->m_models = {&resources::get<Model>("models/" + model1Path.string())};
						auto pProp0 = ecsdb.getComponent<CProp>(eProps_0);
						if (resources::isLoaded<Model>("models/" + model2Path.string()) && pProp0)
						{
							auto& model2 = resources::get<Model>("models/" + model2Path.string());
							pProp0->m_models = {&model2};
						}
					}
				}
			}
		}
		if (auto pSkybox = ecsdb.getComponent<CSkybox>(eSkybox))
		{
			if (pSkybox->m_skybox.hCube.byteCount == 0)
			{
				if (skyboxLoader.loadNext() == AsyncLoadState::Idle)
				{
					pSkybox->m_skybox = std::move(skyboxLoader.m_skybox);
				}
			}
		}

		

		ecsdb.tick(dt);

		gfx::setUBO(hLightsUBO, lights);
		glm::mat4 v = pCamera->view();
		glm::mat4 p = pCamera->perspectiveProj();
		glm::vec4 c(pCamera->m_position, 0.0f);
		// uiSpace = glm::vec3(context::size(), 2.0f);
		uboData::Matrices mats{v, p, p * v, pCamera->uiProj(uiSpace), c};
		gfx::setUBO(hMatricesUBO, mats);

		ecsdb.render();

		drawLight(pl0Pos, lightVerts, lights.ptLights[0].diffuse);
		drawLight(pl1Pos, lightVerts, lights.ptLights[1].diffuse);
		// quadProp.render();

		debug::Quad2D tl, tr, bl, br;
		HTexture const& quadTex = resources::get<HTexture>("textures/awesomeface.png");
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
		debug::renderString(text, uiTextured, font, uiAR);

		debug::renderFPS(font, monolithic, uiAR);
		debug::renderVersion(font, uiTextured, uiAR);

		context::swapBuffers();
		context::pollEvents();
	}
	for (auto spawnID : entities)
	{
		ecsdb.destroyEntity(spawnID);
	}
	ecsdb.destroySystem<FreeCamController, PropRenderer, SkyboxRenderer, GizmoSystem>();
	gfx::releaseMesh(sphereMesh);
	modelsLoader.waitAll();
}
} // namespace

s32 gameloop::run(s32 argc, char const** argv)
{
#if defined(__arm__)
	env::g_config.shaderPrefix = "#version 300 es";
#else
	env::g_config.shaderPrefix = "#version 330 core";
#endif
	context::Settings settings;
	settings.window.title = "LE3D Test";
	settings.ctxt.bVSYNC = false;
	// settings.window.type = context::WindowType::BorderlessWindow;
	// settings.window.width = 3000;
	settings.env.args = {argc, argv};
	settings.env.jobWorkerCount = 4;
	if (auto uContext = context::create(settings))
	{
		runTest();
		return 0;
	}
	return 1;
}
} // namespace letest
