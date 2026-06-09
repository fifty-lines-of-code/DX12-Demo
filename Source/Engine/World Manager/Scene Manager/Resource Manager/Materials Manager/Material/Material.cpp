#include "Material.h"

namespace Engine::EngineResources {
	Material::Material() :
		mType(MaterialType::Invalid),
		mIsDirty(false),
		mIsActive(false)
	{}

	Material::~Material() {}

	void Material::SetData(MaterialData& data) noexcept { mData = data; }

	void Material::SetType(MaterialType type) noexcept {
		mType = type;
	}

	void Material::SetIsDirty(bool isDirty) noexcept { mIsDirty = isDirty; }

	MaterialData& Material::GetData() noexcept { return mData; }

	MaterialType Material::GetType() const noexcept { return mType; }

	bool Material::GetIsDirty() const noexcept { return mIsDirty; }
}