#include "le3d/core/maths.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/io.hpp"
#include "le3d/core/profiler.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/engine_loop.hpp"
#include "le3d/engine/input.hpp"
#include "le3d/env/engine_version.hpp"
#include "le3d/game/camera.hpp"
#include "le3d/game/ecs.hpp"
#include "le3d/game/utils.hpp"

#include "le3d/engine/gfx/gfx_objects.hpp"
#include "le3d/engine/gfx/gfx_store.hpp"
#include "le3d/engine/gfx/gfx_thread.hpp"
#include "le3d/engine/gfx/model.hpp"
#include "le3d/engine/gfx/text2d.hpp"
#include "le3d/engine/gfx/primitives.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/engine/gfx/ubo_types.hpp"
#include "le3d/engine/manifest_loader.hpp"

namespace le
{
namespace
{
gfx::Text2D* g_pFpsText = nullptr;
gfx::Text2D* g_pVersionText = nullptr;

void tickDebugTexts(Time dt)
{
	static s32 frameCount = 0;
	static s32 fps = 0;
	static Time elapsed;
	++frameCount;
	if (elapsed >= Time::from_s(1.0f))
	{
		elapsed = {};
		fps = frameCount;
		frameCount = 0;
		if (g_pFpsText)
		{
			g_pFpsText->updateText(std::to_string(fps) + " FPS");
		}
	}
	elapsed += dt;
	if (fps == 0 && g_pFpsText)
	{
		g_pFpsText->updateText(std::to_string(frameCount) + " FPS");
	}
	return;
}

void renderDebugTexts(f32 uiAspect)
{
	if (g_pFpsText)
	{
		g_pFpsText->render(uiAspect);
	}
	if (g_pVersionText)
	{
		g_pVersionText->render(uiAspect);
	}
	return;
}

void runTest()
{
	glm::vec3 uiSpace = {1920.0f, 1080.0f, 2.0f};
	ClearFlags clearFlags;
	clearFlags.set({ClearFlag::ColorBuffer, ClearFlag::DepthBuffer}, true);
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
	manifestLoader::Manifest manifest{"engine_manifest.json", uReader.get()};
	manifestLoader::Request manifestRequest;
	manifestRequest.manifest = manifest;
	manifestLoader::load(manifestRequest);

	std::string testStr = uReader->getString("engine_manifest.json");
	auto vec = utils::strings::tokenise(testStr, ',', {});

	static s32 const s_reloadCount = 0;
	for (s32 i = 1; i <= s_reloadCount; ++i)
	{
		manifestLoader::unload(manifest);
		manifestLoader::load(manifestRequest);
	}
	auto pGfxStore = gfx::GFXStore::instance();

	gfx::Text2D::Descriptor debugTextDesc;
	debugTextDesc.data.pos = {-uiSpace.x * 0.5f + 100.0f, uiSpace.y * 0.5f - 100.0f, 1.0f};
	debugTextDesc.data.scale = 0.2f;
	debugTextDesc.data.halign = gfx::Font::Text::HAlign::Left;
	debugTextDesc.data.colour = Colour(0xeeeeeeff);
	debugTextDesc.id = "fps";
	g_pFpsText = pGfxStore->load(std::move(debugTextDesc));
	debugTextDesc.data.pos.y = -debugTextDesc.data.pos.y;
	debugTextDesc.id = "version";
	debugTextDesc.data.text = env::engineVersion().toString(true);
	g_pVersionText = pGfxStore->load(std::move(debugTextDesc));
	auto pMonolithicShader = pGfxStore->get<gfx::Shader>("shaders/monolithic");
	gfx::Text2D text0;
	auto pText0 = &text0;
	gfx::Text2D::Descriptor textDesc;
	textDesc.id = "loading";
	textDesc.data.text = "Loading\n ";
	textDesc.data.scale = 1.0f;
	textDesc.data.pos = {0.0f, 0.0f, 1.0f};
	text0.setup(std::move(textDesc));
	auto pUbo0 = pGfxStore->get<gfx::UniformBuffer>("ubos/matrices");
	auto pUbo1 = pGfxStore->get<gfx::UniformBuffer>("ubos/lights");
	if (pUbo0)
	{
		gfx::ubo::Matrices tempMats;
		f32 const w = uiSpace.x * 0.5f;
		f32 const h = uiSpace.y * 0.5f;
		tempMats.uiProj = glm::ortho(-w, w, -h, h, -uiSpace.z, uiSpace.z);
		pUbo0->copyData(tempMats);
	}
	manifestRequest.manifest.id = "demo_manifest.json";
	manifestRequest.clearColour = Colour(0x0a000bff);
	manifestRequest.doFrame = [&](manifestLoader::Args args) {
		{
			std::string dots(size_t(args.progress * 20), '.');
			std::string loadText = "Loading\n";
			loadText += dots;
			text0.updateText(loadText);
			tickDebugTexts(args.dt);
			pText0->render(1920.0f / 1080.0f);
			renderDebugTexts(1920.0f / 1080.0f);
		}
	};
	manifestLoader::load(manifestRequest);

	if (!context::isAlive())
	{
		return;
	}

	gfx::Material defaultMat;
	gfx::Material litTexMat = gfx::GFXStore::instance()->m_litTexturedMaterial;
	gfx::Material litNoTexMat = litTexMat;
	litNoTexMat.flags.set(gfx::Material::Flag::Textured, false);
	gfx::Material litTexNoSpecMat = litTexMat;
	litTexNoSpecMat.flags.set(gfx::Material::Flag::Specular, false);

	ECSDB ecsdb;

	bool bTicking = true;
	bool bTickNext = false;
	Colour clearColour = colours::Black;
	auto pSh0 = pMonolithicShader;
	auto pSh1 = pGfxStore->get<gfx::Shader>("shaders/lit/textured");
	auto pShader = pSh0;

	Transform vao0Transform;
	vao0Transform.setPosition({2.0f, 0.0f, 0.0f});
	auto pVao0 = pGfxStore->get<gfx::VertexArray>("primitives/cube");

	FreeCam camera;
	auto pFreecam = &camera;
	pFreecam->m_position = {0.0f, 0.0f, 3.0f};

	ecsdb.addSystem<PropRenderer, debug::GizmoSystem>();

	gfx::Albedo lightsAlbedo;
	lightsAlbedo.ambient = glm::vec3(0.2f);
	lightsAlbedo.diffuse = glm::vec3(0.5f);
	lightsAlbedo.specular = glm::vec3(1.0f);
	gfx::ubo::Lights::Data pl0;
	gfx::ubo::Lights::Data pl1;
	gfx::ubo::Lights::Data dirL0;
	pl0.position = {-1.0f, 0.0f, 2.0f};
	pl1.position = {-4.0f, 2.0f, 2.0f};
	pl0.clq = pl1.clq = {1.0f, 0.09f, 0.032f};
	dirL0.albedo = pl0.albedo = pl1.albedo = lightsAlbedo;
	dirL0.direction = {-1.0f, -1.0f, 1.0f};
	dirL0.bDirLight = true;
	pl1.idx = 1;
	gfx::ubo::Lights uboLights;
	uboLights.setLights({pl0, pl1, dirL0});
	auto pSphereVerts = pGfxStore->get<gfx::VertexArray>("primitives/sphere");
	gfx::Material lightsMat;
	auto renderLights = [&]() {
		gfx::Shader::ModelMats mats;
		mats.model = glm::translate(pl0.position) * glm::scale(glm::vec3(0.2f));
		lightsMat.tint = Colour(lightsAlbedo.diffuse);
		pMonolithicShader->setModelMats(mats);
		pMonolithicShader->setMaterial(lightsMat);
		pSphereVerts->draw(*pMonolithicShader);
		mats.model = glm::translate(pl1.position) * glm::scale(glm::vec3(0.2f));
		pMonolithicShader->setModelMats(mats);
		pSphereVerts->draw(*pMonolithicShader);
	};

	gfx::Texture bad;
	gfx::Texture::Descriptor badDesc;
	badDesc.id = "textures/bad";
	bad.setup(badDesc, {});
	auto pTexture0 = pGfxStore->get<gfx::Texture>("textures/container.jpg");
	auto pTexture1 = pGfxStore->get<gfx::Texture>("textures/container2.png");
	auto pTexture1s = pGfxStore->get<gfx::Texture>("textures/container2_specular.png");
	auto pTextureAwesome = pGfxStore->get<gfx::Texture>("textures/awesomeface.png");
	gfx::Texture* pToBind = nullptr;

	textDesc.data.text = "Hello\nMulti-line World!";
	textDesc.data.scale = 0.5f;
	textDesc.data.halign = gfx::Font::Text::HAlign::Centre;
	textDesc.data.valign = gfx::Font::Text::VAlign::Middle;
	textDesc.data.pos = {0.0f, 00.0f, 1.0f};
	// textDesc.data.nYPad = 0.0f;
	pText0->update(std::move(textDesc.data));

	auto pSkybox = pGfxStore->get<gfx::Skybox>("skyboxes/default");

	gfx::Mesh::Descriptor meshDesc;
	meshDesc.id = "quad0";
	meshDesc.material.flags.set(gfx::Material::Flag::Textured, true);
	meshDesc.geometry = gfx::createQuad(1.0f, 1.0f);
	auto pQuad0 = pGfxStore->load(std::move(meshDesc));
	pQuad0->m_textures.push_back(pTextureAwesome);

	auto pModel0 = pGfxStore->get<gfx::Model>("models/plant");
	auto pModel1 = pGfxStore->get<gfx::Model>("models/fox");
	gfx::Model* pModel2 = nullptr;
	if (pGfxStore->isLoaded("models/nanosuit"))
	{
		pModel2 = pGfxStore->get<gfx::Model>("models/nanosuit");
	}
	meshDesc.id = "meshes/cube";
	meshDesc.geometry = gfx::createCube(1.0f);
	meshDesc.material.flags.set(gfx::Material::Flag::Lit, true);
	meshDesc.material.flags.set(gfx::Material::Flag::Textured, false);
	auto pCubeMesh = pGfxStore->load(meshDesc);
	pCubeMesh->m_material.tint = colours::Yellow;
	meshDesc.id = "meshes/sphere";
	meshDesc.geometry = gfx::createCubedSphere(1.0f, 8);
	meshDesc.material.flags.set(gfx::Material::Flag::Lit, true);
	meshDesc.material.flags.set(gfx::Material::Flag::Textured, true);
	auto pSphereMesh = pGfxStore->load(meshDesc);
	pSphereMesh->m_textures = {pTexture1, pTexture1s};

	std::vector<ecs::SpawnID> entities;
	auto eProp0 = ecsdb.spawnEntity<CTransform, debug::CGizmo>("prop0");
	if (auto pProp0 = ecsdb.addComponent<CProp>(eProp0))
	{
		pProp0->m_fixtures.push_back(pCubeMesh);
		pProp0->m_pShader = pGfxStore->get<gfx::Shader>("shaders/lit/tinted");
		auto pTransform0 = pProp0->getComponent<CTransform>();
		pTransform0->m_transform.setPosition({-1.0f, 0.0f, 0.0f});
		pTransform0->m_transform.setScale(glm::vec3(0.33f));
		entities.push_back(eProp0);
	}

	ecs::SpawnID eProp1;
	if (auto pProp1 = spawnProp(ecsdb, "prop1"))
	{
		pProp1->m_fixtures.push_back(pModel1);
		pProp1->m_pShader = pGfxStore->get<gfx::Shader>("shaders/lit/textured");
		auto cProp1Transform = pProp1->getComponent<CTransform>();
		cProp1Transform->m_transform.setPosition({-2.0f, 0.0f, -2.0f});
		eProp1 = pProp1->getOwner()->spawnID();
		if (auto pTransform0 = ecsdb.getComponent<CTransform>(eProp0))
		{
			cProp1Transform->m_transform.setParent(&pTransform0->m_transform);
		}
		entities.push_back(eProp1);
	}
	ecs::SpawnID eProp2;
	if (auto pProp2 = spawnProp(ecsdb, "prop2"))
	{
		pProp2->m_fixtures.push_back(pSphereMesh);
		auto cProp1Transform = pProp2->getComponent<CTransform>();
		cProp1Transform->m_transform.setPosition({-5.0f, 2.0f, -5.0f});
		eProp2 = pProp2->getOwner()->spawnID();
		entities.push_back(eProp2);
	}
	ecs::SpawnID eProp3;
	if (auto pProp3 = spawnProp(ecsdb, "prop3"))
	{
		pProp3->m_fixtures.push_back(pModel1);
		auto pTransform3 = pProp3->getComponent<CTransform>();
		pTransform3->m_transform.setPosition({5.0f, 1.0f, 5.0f});
		eProp3 = pProp3->getOwner()->spawnID();
		entities.push_back(eProp3);
	}
	ecs::SpawnID eProp4;
	if (auto pProp4 = spawnProp(ecsdb, "prop4"))
	{
		pProp4->m_fixtures.push_back(pCubeMesh);
		auto pTransform4 = pProp4->getComponent<CTransform>();
		pTransform4->m_transform.setPosition({5.0f, 0.0f, -6.0f});
		eProp4 = pProp4->getOwner()->spawnID();
		entities.push_back(eProp4);
	}
	ecs::SpawnID eProp5;
	if (auto pProp5 = spawnProp(ecsdb, "prop5"))
	{
		pProp5->m_fixtures.push_back(pModel0);
		auto pTransform5 = pProp5->getComponent<CTransform>();
		pTransform5->m_transform.setPosition({-3.0f, 0.0f, -4.0f});
		eProp5 = pProp5->getOwner()->spawnID();
		entities.push_back(eProp5);
	}
	ecs::SpawnID eProp6;
	if (auto pProp6 = spawnProp(ecsdb, "prop5"))
	{
		pProp6->m_fixtures.push_back(pModel0);
		auto pTransform6 = pProp6->getComponent<CTransform>();
		pTransform6->m_transform.setPosition({-4.0f, 1.0f, 4.0f});
		eProp6 = pProp6->getOwner()->spawnID();
		entities.push_back(eProp6);
	}

	ecs::SpawnID eQuad;
	if (auto pQuad = spawnProp(ecsdb, "quad"))
	{
		pQuad->m_fixtures = {pQuad0};
		pQuad->m_pShader = pGfxStore->get<gfx::Shader>("shaders/unlit/textured");
		eQuad = pQuad->getOwner()->spawnID();
		pQuad->getComponent<CTransform>()->m_transform.setPosition(glm::vec3(-2.0f, 2.0f, -2.0f));
		entities.push_back(eQuad);
		eQuad = pQuad->getOwner()->spawnID();
		entities.push_back(eQuad);
	}
	if (pModel2)
	{
		if (auto eProp2 = ecsdb.spawnEntity<CTransform, CProp, debug::CGizmo>("prop2"))
		{
			auto cProp2 = ecsdb.getComponent<CProp>(eProp2);
			cProp2->m_fixtures.push_back(pModel2);
			cProp2->m_pShader = pSh0;
			auto cProp2Transform = ecsdb.getComponent<CTransform>(eProp2);
			cProp2Transform->m_transform.setPosition({-0.0f, -2.0f, -3.0f});
			entities.push_back(eProp2);
		}
	}

	/*constexpr s32 stress = 1000;
	constexpr s32 noise = 500;
	for (s32 i = 0; i < stress; ++i)
	{
		auto id = ecsdb.spawnEntity<CTransform, CProp_OLD>("Stress_" + std::to_string(i));
		entities.push_back(id);
	}
	for (s32 i = 0; i < noise; ++i)
	{
		auto id = ecsdb.spawnEntity<CTransform>("Noise_" + std::to_string(i));
		entities.push_back(id);
	}*/

	auto const viewCrop = 1.0f;
	glm::vec2 viewCentre = {0.0f, 0.0f};
	auto onWindowSizeChange = input::registerResize([&](s32, s32) {
		auto view = Rect2(context::windowSize() * viewCrop, viewCentre);
		gfx::setView(view);
	});

	bool bWireframe = false;
	std::unordered_set<Key> held;
	auto tOnInput = input::registerInput([&](Key key, Action action, [[maybe_unused]] Mods mods) {
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
			if (!bTicking && key == Key::SPACE)
			{
				bTickNext = true;
			}
			if (key == Key::ENTER && mods & Mods::CONTROL)
			{
				auto mode = gfx::mode();
				switch (mode)
				{
				default:
					break;
				case GFXMode::BufferedThreaded:
					mode = GFXMode::BufferedMainThread;
					break;
				case GFXMode::BufferedMainThread:
					mode = GFXMode::ImmediateMainThread;
					break;
				case GFXMode::ImmediateMainThread:
					mode = GFXMode::BufferedThreaded;
					break;
				}
				gfx::setMode(mode);
			}
			if (key == Key::SPACE && mods & Mods::CONTROL)
			{
				bTicking = !bTicking;
				ecsdb.setAll(System::Flag::Ticking, bTicking);
			}
			if (key == Key::V && mods & Mods::CONTROL)
			{
				context::setSwapInterval(context::swapInterval() == 0 ? 1 : 0);
			}
			if (key == Key::C && mods & Mods::CONTROL)
			{
				clearColour = clearColour == colours::Black ? colours::Blue : colours::Black;
			}
			if (bTicking && key == Key::S && mods & Mods::CONTROL)
			{
				pShader = pShader == pSh0 ? pSh1 : pSh0;
			}
			if (bTicking && key == Key::W && mods & Mods::CONTROL)
			{
				bWireframe = !bWireframe;
			}
			if (bTicking && key == Key::P && mods & Mods::CONTROL)
			{
				static bool s_bParented = true;
				if (auto pTransform1 = ecsdb.getComponent<CTransform>(eProp1))
				{
					if (auto pTransform0 = ecsdb.getComponent<CTransform>(eProp0))
					{
						pTransform1->m_transform.setParent(s_bParented ? nullptr : &pTransform0->m_transform);
						s_bParented = !s_bParented;
					}
				}
			}
			if (bTicking && key == Key::I && mods && Mods::CONTROL)
			{
				static bool s_bInstancesSet = false;
				if (!s_bInstancesSet)
				{
					s_bInstancesSet = true;
					gfx::InstanceBuffer instances;
					instances.models.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(-4.5f, 1.5f, -4.0f)));
					instances.models.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, -3.0f)));
					instances.models.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 2.25f, -2.0f)));
					pSphereMesh->setInstances(std::move(instances));
				}
				else
				{
					s_bInstancesSet = false;
					pSphereMesh->setInstances({});
				}
			}
			if (bTicking && key == Key::T && mods & Mods::CONTROL)
			{
				if (pTexture0->isReady() && pTexture1->isReady() && pSh0->isReady())
				{
					if (!pToBind || pToBind == pTexture1)
					{
						pToBind = pTexture0;
					}
					else if (pToBind == &bad)
					{
						pToBind = pTexture1;
					}
					else if (pToBind == pTexture0)
					{
						pToBind = &bad;
					}
				}
			}
		}
		}
	});

	auto midTick = [eProp0, eProp1, eQuad](ECSDB& ecsdb, Time dt) {
		if (auto pProp0 = ecsdb.getComponent<CTransform>(eProp0))
		{
			pProp0->m_transform.setOrientation(
				glm::rotate(pProp0->m_transform.orientation(), glm::radians(dt.to_s() * 30), glm::vec3(1.0f, 0.3f, 0.5f)));
		}
		if (auto pProp1 = ecsdb.getComponent<CTransform>(eProp1))
		{
			pProp1->m_transform.setOrientation(
				glm::rotate(pProp1->m_transform.orientation(), glm::radians(dt.to_s() * 10), glm::vec3(1.0f, 0.3f, 0.5f)));
			if (auto pQuad = ecsdb.getComponent<CTransform>(eQuad))
			{
				pQuad->m_transform.setOrientation(
					glm::rotate(pProp1->m_transform.orientation(), glm::radians(dt.to_s() * 30), glm::vec3(0.3f, 0.5f, 1.0f)));
			}
		}
	};
	auto midTickHandle = ecsdb.addTickSlot(midTick, 10);

	Time dt;
	Time t = Time::elapsed();
	u32 tickFrame = 0;
	auto glID = pShader->gfxID();

	while (context::isAlive())
	{
		t = Time::elapsed();
		static Colour s_clearColour = clearColour;
		gfx::clearFlags(clearFlags, s_clearColour);

		if (bTicking || bTickNext)
		{
			++tickFrame;
			bTickNext = false;
			// Tick here
			s_clearColour = clearColour;
			glID = pShader->gfxID();
			vao0Transform.setOrientation(glm::rotate(vao0Transform.orientation(), glm::radians(dt.to_s() * 10), g_nUp));

			LOGIF_D(!bTicking, "Frame: Tick: %u, Swap: %u, Render: %u", tickFrame, context::framesTicked(), context::framesRendered());
		}
		pFreecam->tick(dt);
		ecsdb.tick(dt);
		tickDebugTexts(dt);

		auto v = pFreecam->view();
		auto p = pFreecam->perspectiveProj();
		gfx::ubo::Matrices uboMatrices{v, p, p * v, pFreecam->uiProj(uiSpace)};
		uboMatrices.setViewPos(pFreecam->m_position);
		pUbo0->copyData(uboMatrices);
		pUbo1->copyData(uboLights);

		// Render
		{
			if (pSkybox)
			{
				pSkybox->render();
			}

			auto const& u = env::g_config.uniforms;
			gfx::setPolygonMode(bWireframe ? PolygonMode::Line : PolygonMode::Fill);
			pShader->setMaterial(pToBind == pTexture0 ? litTexNoSpecMat : litTexMat);
			pShader->setMaterial(litTexMat);
			if (pToBind)
			{
				if (pToBind == pTexture1 && pTexture1s->isReady())
				{
					pSh0->bind({pTexture1, pTexture1s});
				}
				else
				{
					pSh0->bind({pToBind});
				}
			}
			else
			{
				pSh0->unbind({TexType::Diffuse});
			}
			pSh0->setBool(u.transform.isUI, false);
			pSh0->setBool(u.transform.isInstanced, false);
			gfx::Shader::ModelMats mats;
			mats.model = vao0Transform.model();
			mats.normals = vao0Transform.normalModel();
			pSh0->setModelMats(mats);
			pVao0->draw(*pSh0);

			ecsdb.render();
			renderLights();

			gfx::setPolygonMode(PolygonMode::Fill);
			pText0->render(uiSpace.x / uiSpace.y);
			renderDebugTexts(uiSpace.x / uiSpace.y);
		}
		context::swapAndPresent();
		context::pollEvents();
		dt = Time::elapsed() - t;
	}
	for (auto eID : entities)
	{
		ecsdb.destroyEntity(eID);
	}
}
} // namespace

s32 engineLoop::run(s32 argc, char const** argv)
{
#if defined(__arm__)
	env::g_config.shaderPrefix = "#version 300 es";
#else
	env::g_config.shaderPrefix = "#version 330 core";
#endif
	context::Settings settings;
	settings.window.title = "LE3D Test";
	settings.ctxt.bVSYNC = false;
	settings.ctxt.bThreaded = true;
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
} // namespace le
