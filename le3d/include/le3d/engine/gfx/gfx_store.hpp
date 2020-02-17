#pragma once
#include <memory>
#include <string>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/map_store.hpp"
#include "gfx_objects.hpp"
#include "model.hpp"
#include "text2d.hpp"

namespace le::gfx
{
class GFXStore final
{
public:
	static GFXStore* instance();
	static bool destroyInstance();

	Texture* m_pBlankTexture;
	Material m_litTexturedMaterial;

private:
	TMapStore<std::unordered_map<std::string, std::unique_ptr<GFXObject>>> m_objects;

public:
	GFXStore();
	~GFXStore();

public:
	VertexArray* load(VertexArray::Descriptor descriptor, Geometry geometry);
	UniformBuffer* load(UniformBuffer::Descriptor descriptor);
	Shader* load(Shader::Descriptor descriptor);
	Sampler* load(Sampler::Descriptor descriptor);
	Texture* load(Texture::Descriptor descriptor, bytearray image);
	Texture* load(Texture::Descriptor descriptor, bytearray bytes, u8 ch, u16 w, u16 h);
	Font* load(Font::Descriptor descriptor, bytearray fontAtlasImage);
	Cubemap* load(Cubemap::Descriptor descriptor, std::array<bytearray, 6> rludfb);
	Skybox* load(Skybox::Descriptor descriptor);
	Mesh* load(Mesh::Descriptor descriptor);
	Model* load(Model::Descriptor descriptor);
	Text2D* load(Text2D::Descriptor descriptor);

	[[nodiscard]] bool isLoaded(std::string const& id) const;
	[[nodiscard]] bool isReady(std::string const& id) const;

	template <typename Type, typename... Args>
	Type* create(std::string const& id, Args... args);
	template <typename Type>
	Type const* get(std::string const& id) const;
	template <typename Type>
	Type* get(std::string const& id);
	template <typename Type>
	bool unload(std::string const& id);

private:
	bool preload(std::string const& id);

	template <typename Ret, typename... Args>
	Ret* load(typename Ret::Descriptor descriptor, Args... args);
};

template <typename Type, typename... Args>
Type* GFXStore::create(std::string const& id, Args... args)
{
	static_assert(std::is_base_of_v<GFXObject, Type>, "Type must derive from GFXObject!");
	if (!preload(id))
	{
		return nullptr;
	}
	auto uRet = std::make_unique<Type>(std::forward<Args>(args)...);
	uRet->m_id = id;
	m_objects.insert(id, std::move(uRet));
	return dynamic_cast<Type*>(m_objects.get(id)->get());
}

template <typename Type>
Type const* GFXStore::get(std::string const& id) const
{
	static_assert(std::is_base_of_v<GFXObject, Type>, "Type must derive from GFXObject!");
	if (m_objects.isLoaded(id))
	{
		return dynamic_cast<Type*>(m_objects.get(id)->get());
	}
	LOG_E("[%s] Object ID not loaded: [%s]!", typeName<GFXStore>().data(), id.data());
	ASSERT(false, "Object not loaded!");
	return nullptr;
}

template <typename Type>
Type* GFXStore::get(std::string const& id)
{
	static_assert(std::is_base_of_v<GFXObject, Type>, "Type must derive from GFXObject!");
	if (m_objects.isLoaded(id))
	{
		return dynamic_cast<Type*>(m_objects.get(id)->get());
	}
	LOG_E("[%s] Object ID not loaded: [%s]!", typeName<GFXStore>().data(), id.data());
	ASSERT(false, "Object not loaded!");
	return nullptr;
}

template <typename Type>
bool GFXStore::unload(std::string const& id)
{
	static_assert(std::is_base_of_v<GFXObject, Type>, "Type must derive from GFXObject!");
	if (m_objects.unload(id))
	{
		return true;
	}
	LOG_W("[%s] Object ID not loaded: [%s]!", typeName<GFXStore>().data(), id.data());
	return false;
}

template <typename Ret, typename... Args>
Ret* GFXStore::load(typename Ret::Descriptor descriptor, Args... args)
{
	if (!preload(descriptor.id.generic_string()))
	{
		return nullptr;
	}
	auto uRet = std::make_unique<Ret>();
	if (uRet->setup(std::move(descriptor), std::forward<Args>(args)...))
	{
		auto id = uRet->id().generic_string();
		m_objects.insert(id, std::move(uRet));
		return dynamic_cast<Ret*>(m_objects.get(id)->get());
	}
	return nullptr;
}
} // namespace le::gfx
