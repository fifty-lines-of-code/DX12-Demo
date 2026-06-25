#pragma once

#include <cstdint>
#include "MaterialData.h"

namespace Engine::EngineResources {

	enum class MaterialType: uint16_t {
		PLAYER,
		WALL,
		TERRAIN,
		MIRROR,
		COUNT,
		INVALID
	};
	// remember to update the MATERIAL_COUNT inside 
	// opaque_vs_ps.hlsl when you update this
	// todo: find a better way to do this

	class Material {
	public:
		Material();
		~Material();

		void SetData(MaterialData& data) noexcept;
		void SetType(MaterialType type) noexcept;
		void SetIsDirty(bool isDirty) noexcept;
		MaterialData& GetData() noexcept;
		MaterialType GetType() const noexcept;
		bool GetIsDirty() const noexcept;

	private:
		MaterialData mData;
		MaterialType mType;
		bool mIsDirty;
		bool mIsActive;
	};
}