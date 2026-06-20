#pragma once

#include "../../../Math/Geometry.h"
#include "../../../Math/BasisVectors.h"
#include "PerPassAndPerEntityConstantBufferData.h"
#include "../Resource Manager/Materials Manager/Material/Material.h"
#include "../../../Physics System/PhysicsBody.h"
#include "../Resource Manager/Texture Manager/TextureID.h"

namespace Engine {

	class Mesh;

	class Entity {
	public:
		Entity();
		Entity(
			uint32_t Id,
			Vector3 center,
			Vector3 scale,
			bool isStatic,
			bool isActive
		);
		~Entity();

		void SetID(uint32_t id);
		uint32_t GetID() const;

		void SetIsStatic(bool isStatic);
		bool GetIsStatic() const;

		void SetMesh(const Mesh* mesh);
		const Mesh* GetMesh() const;

		void Update(float stickX, float stickY, float deltaTime, float speed);

		void CopyToDestinationConstantBufferDataTransposed(EntityConstantBufferData& bufferData);
		EnginePhysics::PhysicsBody& GetPhysicsBody();
		const AABB& GetAABB() const;

		bool GetIsDirty() const;
		void SetIsDirty(bool dirty);

		bool GetIsActive() const noexcept;
		void SetIsActive(bool isActive) noexcept;

		void SetScale(Vector3 scale);

		void SetMaterialType(EngineResources::MaterialType type);

		EngineResources::TextureID GetTextureID() const;
		void SetTextureID(EngineResources::TextureID tID);

	private:
		EnginePhysics::PhysicsBody mPhysicsBody;
		const Mesh* mMesh;
		uint32_t mID;
		EngineResources::TextureID mTextureID;
		EngineResources::MaterialType mMaterialType;
		bool mIsStatic;
		bool mIsDirty;
		bool mIsActive;
	};
}