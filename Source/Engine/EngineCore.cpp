#include "EngineCore.h"

#include "Camera/Camera.h"
#include <DirectXMath.h>
#include "World Manager/Scene Manager/Entity/Entity.h"
#include "../Game/Game Timer/GameTimer.h"
#include "World Manager/Scene Manager/Resource Manager/Materials Manager/Material/Material.h"
#include "World Manager/Scene Manager/Entity/Mesh/Mesh.h"
#include "../Engine/Input System/XBox/XBoxInputSystem.h"

namespace Engine {

	EngineCore::EngineCore(HINSTANCE hInstance, std::wstring caption) :
		mhAppInst(hInstance),
		mMainWndCaption(caption),
		mIsInitialized(false),
		mAnimationSpeed(.375f), // todo: move this out to somewhere else
		mRenderer(DX12Renderer()),
		mWorldManager(WorldManager()),
		mCamera(Camera()),
		mInputSystem(XboxInputSystem())
	{}

	EngineCore::~EngineCore() {}

	bool EngineCore::Initialize(HWND mainHwnd, UINT screenWidth, UINT screenHeight) {
		if (screenHeight == 0) { return false; }

		mhMainWnd = mainHwnd;

		if (!mRenderer.Initialize(mhMainWnd, NumberOfFrameResources, screenWidth, screenHeight)) { return false; }

		if (!mWorldManager.Initialize()) { return false; }

		if (!InitializeCamera(mWorldManager.GetPlayerCenter())) { return false; }

		// set all entities to dity so they are updated
		mNumberOfDirtyFramesPerEntity.resize(mWorldManager.GetEntityCount());
		mNumberOfDirtyFramesPerEntity.assign(
			mWorldManager.GetEntityCount(),
			NumberOfFrameResources
		);

		mNumberOfDirtyFramesPerMaterial.resize(mWorldManager.GetMaterialCount());
		mNumberOfDirtyFramesPerMaterial.assign(
			mWorldManager.GetMaterialCount(),
			NumberOfFrameResources
		);

		if (!SetupPipeline()) { return false; }

		LoadGeometry();

		LoadTextures();

		mRenderer.FinishInitialize();

		mIsInitialized = true;

		return true;
	}

	bool EngineCore::SetupPipeline() {
		return mRenderer.SetupPipeline(
			(uint32_t)mWorldManager.GetEntityCount(),
			mWorldManager.GetMaterialCount(),
			mWorldManager.GetConstantBufferDataByteSizeOfEachEntity(),
			mWorldManager.GetConstantBufferDataByteSizeOfEachPerPassObject(),
			mWorldManager.GetConstantBufferDataByteSizeOfEachMaterialObject()
		);
	}

	void EngineCore::UpdateInputSystemAndCamera(float deltaTime) {
		mInputSystem.Update();

		mCamera.UpdateWithInputSystem(
			deltaTime,
			mInputSystem.GetRightStickX(),
			mInputSystem.GetRightStickY()
		);
	}

	void EngineCore::Update(float deltaTime) {
		// todo: 

		// update the input system first
		UpdateInputSystemAndCamera(deltaTime);

		// update the world manager
		mWorldManager.Update(
			&mInputSystem,
			deltaTime,
			mAnimationSpeed,
			mCamera.GetBasisVectors()
		);

		// update the camera with updated player center
		mCamera.UpdateWithTarget(mWorldManager.GetPlayerCenter());

		// prepare the renderer for updates
		mRenderer.PrepareForUpdate();

		// Update constant buffers.
		UpdateConstantBuffers();
	}

	void EngineCore::Draw() {
		mRenderer.BeginFrame(mWorldManager.GetMaterialCount());

		for (auto& entity : mWorldManager.GetEntities()) {
			mRenderer.Draw(
				(uint32_t)entity.GetMesh()->GetMeshID(),
				(uint32_t)entity.GetMesh()->GetIndices().size(),
				entity.GetID(),
				mWorldManager.GetEntityCount()
			);
		}
		mRenderer.EndFrame();
	}

	void EngineCore::OnResize(UINT newClientWidth, UINT newClientHeight) {
		if (!mIsInitialized) { return; }

		mRenderer.OnResize(newClientWidth, newClientHeight);
		mCamera.OnResize(newClientWidth, newClientHeight);
	}

	bool EngineCore::InitializeCamera(const Vector3& playerPosition) {
		mCamera.Initialize(playerPosition);
		return true;
	}

	void EngineCore::LoadGeometry() {
		std::vector<const Mesh*> meshesToLoad;
		mWorldManager.GetMeshesToLoad(meshesToLoad);

		for (const Mesh* mesh : meshesToLoad) {
			mRenderer.LoadGeometry(
				(uint32_t)mesh->GetMeshID(),
				sizeof(Vertex),
				mesh->GetVbByteSize(),
				(void*)mesh->GetVertices().data(),
				mesh->GetIbByteSize(),
				(void*)mesh->GetIndices().data()
			);
		}
	}

	void EngineCore::LoadTextures() {
		for (EngineResources::TextureAsset& texture : mWorldManager.GetTextures()) {
			if (texture.isLoaded) { continue; }
			if (!texture.isReadyToLoad) { continue; }

			bool result = mRenderer.LoadTexture(
				texture.Name, 
				texture.FileName,
				(uint32_t)texture.id
			);

			if (result) {
				texture.isLoaded = true;
				texture.isReadyToLoad = false;
			}
		}
	}

	void EngineCore::UpdateConstantBuffers() {
		// update per-pass constant buffers
		UpdatePerPassConstantBuffers();

		// update per entity cb
		UpdatePerEntityConstantBuffers();
		
		// update per material cb
		UpdatePerMaterialConstantBuffers();
	}

	void EngineCore::UpdatePerPassConstantBuffers() const {
		PerPassConstantBufferData perPassCB;

		// DirectXMath uses row-major alignment in CPU memory, but 
		// HLSL defaults to column-major storage for matrix packing. 
		// We transpose here to prevent skewed vector transformations on the GPU.
		const Matrix4x4& viewProj = mCamera.GetViewProjection();
		DirectX::XMMATRIX viewProjTranspose = DirectX::XMMatrixTranspose(
			DirectX::XMLoadFloat4x4(&viewProj.AsXMFLOAT4X4())
		);
		DirectX::XMStoreFloat4x4(
			&perPassCB.ViewProjectionTranspose.AsXMFLOAT4X4(),
			viewProjTranspose
		);

		// set camera pos
		const Vector3& cameraPos = mCamera.GetPosition();
		perPassCB.EyePosW = cameraPos;

		// set ambient light
		perPassCB.AmbientLight = mWorldManager.GetAmbientLight();

		// set light data
		mWorldManager.GetLightsData(perPassCB.Lights);

		mRenderer.UpdatePerPassCb(
			&perPassCB,
			sizeof(PerPassConstantBufferData)
		);
	}

	void EngineCore::UpdatePerEntityConstantBuffers() {
		for (auto& entity : mWorldManager.GetEntities()) {
			uint32_t id = entity.GetID();

			if (entity.GetIsDirty()) {
				mNumberOfDirtyFramesPerEntity[id] = NumberOfFrameResources;
				entity.SetIsDirty(false);
			}

			if (mNumberOfDirtyFramesPerEntity[id] > 0) {
				EntityConstantBufferData bufferData;
				entity.CopyToDestinationConstantBufferDataTransposed(bufferData);

				mRenderer.UpdatePerRenderItemCb(
					id,
					&bufferData,
					mWorldManager.GetConstantBufferDataByteSizeOfEachEntity()
				);
				mNumberOfDirtyFramesPerEntity[id]--;
			}
		}
	}

	void EngineCore::UpdatePerMaterialConstantBuffers() {
		for (auto& material : mWorldManager.GetMaterials()) {
			uint16_t id = (uint16_t)material.GetType();

			if (material.GetIsDirty()) {
				mNumberOfDirtyFramesPerMaterial[id] = NumberOfFrameResources;
				material.SetIsDirty(false);
			}

			if (mNumberOfDirtyFramesPerMaterial[id] > 0) {
				mRenderer.UpdatePerMaterialCb(
					id,
					&material.GetData(),
					mWorldManager.GetConstantBufferDataByteSizeOfEachMaterialObject()
				);
				mNumberOfDirtyFramesPerMaterial[id]--;
			}
		}
	}
}