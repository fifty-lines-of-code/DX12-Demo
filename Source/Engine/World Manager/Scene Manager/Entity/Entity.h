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
#include "../../Scene/SceneBlueprint.h"

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
		~Entity() = default;

		bool Initialize(
			const EntityBlueprint& entityBlueprint,
			uint32_t id,
			EngineResources::Mesh* mesh
		) noexcept;

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

		uint32_t GetID() const noexcept;

		bool GetIsStatic() const noexcept;

		Vector3 GetCenter() const noexcept;
		Vector3 GetScale() const noexcept;
		Vector3 GetSurfaceNormal() const noexcept;

		const EngineResources::Mesh* GetMesh() const noexcept;

		EnginePhysics::PhysicsBody& GetPhysicsBody() noexcept;
		const AABB& GetAABB() const noexcept;
		EntityTransformData& GetTransformData() noexcept;

		bool GetIsDirty() const;
		void SetIsDirty(bool dirty);

		bool GetIsActive() const noexcept;

		bool GetIsTerrainOrFloor() const noexcept;
		
		EntityType GetEntityType() const noexcept;

		bool IsSubMeshAtIndexRenderingAMirror(
			uint32_t subMeshIndex
		) const noexcept;

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

	private:
		void SetID(uint32_t id);
		void SetMesh(EngineResources::Mesh* mesh);
		void SetEntityType(EntityType entityType) noexcept;
		void SetIsStatic(bool isStatic);
		void SetIsActive(bool isActive) noexcept;
		void SetCenter(const Vector3& center) noexcept;
		void SetScale(const Vector3& scale) noexcept;
		void SetBasisVectors(
			const BasisVectors& basisVectors
		) noexcept;
		void SetSubMeshMaterialAndTexture(
			uint8_t subMeshIndex,
			EngineResources::MaterialType material,
			EngineResources::TextureID texture
		) noexcept;
		void SetSurfaceNormal(
			const Vector3& surfaceNormal
		) noexcept;
	};
}