#include <memory>
#include "le3d/core/assert.hpp"
#include "le3d/engine/gfx/gfx_store.hpp"
#include "le3d/engine/gfx/primitives.hpp"
#include "le3d/engine/gfx/ubo_types.hpp"

namespace le::gfx
{
namespace
{
std::unique_ptr<GFXStore> g_store;

template <typename T>
void loadUbo(GFXStore* pStore)
{
	UniformBuffer::Descriptor uboDesc;
	uboDesc.id = T::s_name;
	uboDesc.bindingPoint = T::s_bindingPoint;
	uboDesc.drawType = T::s_drawType;
	uboDesc.size = sizeof(T);
	pStore->load(std::move(uboDesc));
}
} // namespace

GFXStore* GFXStore::instance()
{
	if (!g_store)
	{
		g_store = std::make_unique<GFXStore>();
	}
	return g_store.get();
}

bool GFXStore::destroyInstance()
{
	if (g_store)
	{
		g_store.reset();
		return true;
	}
	return false;
}

GFXStore::GFXStore()
{
	// Materials
	m_litTexturedMaterial.id = "materials/lit-textured";
	m_litTexturedMaterial.albedo.ambient = glm::vec3(0.3f);
	m_litTexturedMaterial.albedo.diffuse = glm::vec3(0.5f);
	m_litTexturedMaterial.albedo.specular = glm::vec3(1.0f);
	m_litTexturedMaterial.albedo.shininess = 32.0f;
	m_litTexturedMaterial.flags.set({Material::Flag::Lit, Material::Flag::Textured, Material::Flag::Specular}, true);

	// Samplers
	Sampler::Descriptor samplerDesc;
	samplerDesc.id = "samplers/default";
	load(std::move(samplerDesc));
	samplerDesc.id = "samplers/font";
	samplerDesc.wrap = TexWrap::ClampEdge;
	samplerDesc.minFilter = TexFilter::Linear;
	samplerDesc.anisotropy = 1;
	load(std::move(samplerDesc));

	// Primitives
	VertexArray::Descriptor primitiveDesc;
	primitiveDesc.id = "primitives/quad";
	primitiveDesc.drawType = DrawType::Dynamic;
	load(std::move(primitiveDesc), createQuad(1.0f, 1.0f));
	primitiveDesc.id = "primitives/cube";
	primitiveDesc.drawType = DrawType::Static;
	load(std::move(primitiveDesc), createCube(1.0f));
	primitiveDesc.id = "primitives/pyramid-4";
	load(std::move(primitiveDesc), create4Pyramid(1.0f));
	primitiveDesc.id = "primitives/tetrahedron";
	load(std::move(primitiveDesc), createTetrahedron(1.0f));

	primitiveDesc.id = "primitives/circle";
	load(std::move(primitiveDesc), createCircle(1.0f, 16));
	primitiveDesc.id = "primitives/cylinder";
	load(std::move(primitiveDesc), createCylinder(1.0f, 1.0f, 16));
	primitiveDesc.id = "primitives/cone";
	load(std::move(primitiveDesc), createCone(1.0f, 1.0f, 16));
	primitiveDesc.id = "primitives/sphere";
	load(std::move(primitiveDesc), createCubedSphere(1.0f, 8));

	// Uniform Buffers
	loadUbo<ubo::Matrices>(this);
	loadUbo<ubo::Lights>(this);

	// Blank Texture
	Texture::Descriptor textureDesc;
	textureDesc.id = "textures/blank";
	textureDesc.type = TexType::Diffuse;
	bytearray bytes = {(std::byte)0xff, (std::byte)0xff, (std::byte)0xff};
	m_pBlankTexture = load(std::move(textureDesc), std::move(bytes), 3, 1, 1);
	LOG_I("[%s] constructed", typeName<GFXStore>().data());
}

GFXStore::~GFXStore()
{
	LOG_I("[%s] destroyed", typeName<GFXStore>().data());
}

VertexArray* GFXStore::load(VertexArray::Descriptor descriptor, Geometry geometry)
{
	return load<VertexArray, Geometry>(std::move(descriptor), std::move(geometry));
}

UniformBuffer* GFXStore::load(UniformBuffer::Descriptor descriptor)
{
	return load<UniformBuffer>(std::move(descriptor));
}

Shader* GFXStore::load(Shader::Descriptor descriptor)
{
	return load<Shader>(std::move(descriptor));
}

Sampler* GFXStore::load(Sampler::Descriptor descriptor)
{
	return load<Sampler>(std::move(descriptor));
}

Texture* GFXStore::load(Texture::Descriptor descriptor, bytearray image)
{
	return load<Texture, bytearray>(std::move(descriptor), std::move(image));
}

Texture* GFXStore::load(Texture::Descriptor descriptor, bytearray bytes, u8 ch, u16 w, u16 h)
{
	return load<Texture, bytearray, u8, u16, u16>(std::move(descriptor), std::move(bytes), ch, w, h);
}

Font* GFXStore::load(Font::Descriptor descriptor, bytearray fontAtlasImage)
{
	return load<Font, bytearray>(std::move(descriptor), std::move(fontAtlasImage));
}

Cubemap* GFXStore::load(Cubemap::Descriptor descriptor, std::array<bytearray, 6> rludfb)
{
	return load<Cubemap, std::array<bytearray, 6>>(std::move(descriptor), std::move(rludfb));
}

Skybox* GFXStore::load(Skybox::Descriptor descriptor)
{
	return load<Skybox>(std::move(descriptor));
}

Mesh* GFXStore::load(Mesh::Descriptor descriptor)
{
	return load<Mesh>(std::move(descriptor));
}

Model* GFXStore::load(Model::Descriptor descriptor)
{
	return load<Model>(std::move(descriptor));
}

Text2D* GFXStore::load(Text2D::Descriptor descriptor)
{
	return load<Text2D>(std::move(descriptor));
}

bool GFXStore::isLoaded(std::string const& id) const
{
	return m_objects.isLoaded(id);
}

bool GFXStore::isReady(std::string const& id) const
{
	if (auto pObj = m_objects.get(id))
	{
		return pObj && (*pObj)->isReady();
	}
	return false;
}

bool GFXStore::preload(std::string const& id)
{
	ASSERT(!m_objects.isLoaded(id), "id already loaded!");
	if (m_objects.isLoaded(id))
	{
		LOG_E("[%s] Object ID already loaded: [%s]!", typeName<GFXStore>().data(), id.data());
		return false;
	}
	return true;
}
} // namespace le::gfx
