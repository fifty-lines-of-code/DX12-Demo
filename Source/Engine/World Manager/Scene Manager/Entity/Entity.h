#pragma once

#include <array>
#include "../../../Math/BasisVectors.h"
#include "EntityRenderData.h"
#include "EntityTransformData.h"
#include "EntityType.h"
#include "../../../Math/Geometry.h"
#include "../Resource Manager/Materials Manager/Material/Material.h"
#include "../Resource Manager/Mesh Generator/Mesh/Mesh.h"
#include "PerPassAndPerEntityConstantBufferData.h"
#include "../../../Physics System/PhysicsBody.h"
#include "../Resource Manager/Texture Manager/TextureID.h"

namespace Engine::EngineWorld {

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

		void SetMesh(EngineResources::Mesh* mesh);
		const EngineResources::Mesh* GetMesh() const;

		void Update(
			float stickX,
			float stickY, 
			float deltaTime, 
			float speed
		);

		void CopyToDestinationEntityConstantBufferDataTransposed(EntityConstantBufferData& bufferData);

		void CopyToDestinationSubMeshConstantBufferData(
			uint8_t subMeshIndex,
			EntitySubMeshConstantBufferData& destinationBufferData
		);

		EnginePhysics::PhysicsBody& GetPhysicsBody() noexcept;
		const AABB& GetAABB() const noexcept;
		EntityTransformData& GetTransformData() noexcept;

		bool GetIsDirty() const;
		void SetIsDirty(bool dirty);

		bool GetIsActive() const noexcept;
		void SetIsActive(bool isActive) noexcept;

		void SetCenter(Vector3 center);
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
		EngineResources::Mesh* mMesh;
		EnginePhysics::PhysicsBody mPhysicsBody;
		EntityRenderData mRenderData;
		EntityTransformData mTransformData;
		uint32_t mID;
		EntityType mEntityType;
		bool mIsStatic;
		bool mIsDirty;
		bool mIsActive;
	};
}