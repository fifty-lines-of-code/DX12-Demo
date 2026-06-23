#pragma once

#include "../../../Math/Geometry.h"
#include "../../../Math/BasisVectors.h"
#include "PerPassAndPerEntityConstantBufferData.h"
#include "EntityType.h"
#include "../Resource Manager/Materials Manager/Material/Material.h"
#include "../../../Physics System/PhysicsBody.h"
#include "../Resource Manager/Texture Manager/TextureID.h"

namespace Engine::EngineWorld {

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

		void SetMesh(Mesh* mesh);
		const Mesh* GetMesh() const;

		void Update(float stickX, float stickY, float deltaTime, float speed);

		void CopyToDestinationEntityConstantBufferDataTransposed(EntityConstantBufferData& bufferData);

		void CopyToDestinationSubMeshConstantBufferDataTransposed(
			uint8_t subMeshId,
			EntitySubMeshConstantBufferData& destinationBufferData
		);

		EnginePhysics::PhysicsBody& GetPhysicsBody();
		const AABB& GetAABB() const;

		bool GetIsDirty() const;
		void SetIsDirty(bool dirty);

		bool GetIsActive() const noexcept;
		void SetIsActive(bool isActive) noexcept;

		void SetScale(Vector3 scale);

		bool GetIsTerrainOrFloor() const noexcept;
		
		void SetEntityType(EntityType entityType) noexcept;
		EntityType GetEntityType() const noexcept;

		void SetSubMeshMaterialAndTexture(
			uint8_t subMeshIndex,
			EngineResources::MaterialType material,
			EngineResources::TextureID texture
		) noexcept;

	private:
		EnginePhysics::PhysicsBody mPhysicsBody;
		Mesh* mMesh;
		uint32_t mID;
		EntityType mEntityType;
		bool mIsStatic;
		bool mIsDirty;
		bool mIsActive;
	};
}