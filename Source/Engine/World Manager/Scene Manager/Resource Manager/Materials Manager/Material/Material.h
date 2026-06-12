#pragma once

#include <cstdint>
#include "../../../../../Math/EngineMath.h"

namespace Engine::EngineResources {

	struct MaterialData {
		Vector4 DiffuseAlbedo;
		Vector3 FresnelR0;
		float Roughness = 0.25f;
		Matrix4x4 MatTransform;
		// remember wher you update this struct
		// also update 
		// 1. DX12FrameResource
		// 2. cbuffer in the shaders
	};

	enum class MaterialType: uint16_t {
		PLAYER,
		WALL,
		TERRAIN,
		COUNT,
		INVALID
	};

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