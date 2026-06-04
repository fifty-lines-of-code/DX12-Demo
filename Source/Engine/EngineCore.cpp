#include "EngineCore.h"

#include "Camera/Camera.h"
#include <DirectXMath.h>
#include "World Manager/Scene Manager/Entity/Entity.h"
#include "../Game/Game Timer/GameTimer.h"
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

		if (!mRenderer.Initialize(mhMainWnd, EngineCore::NumberOfFrameResources, screenWidth, screenHeight)) { return false; }

		if (!mWorldManager.Initialize()) { return false; }

		if (!InitializeCamera(mWorldManager.GetPlayerCenter())) { return false; }

		// set all entities to dity so they are updated
		mNumberOfDirtyFramesPerEntity.resize(mWorldManager.GetEntityCount());
		mNumberOfDirtyFramesPerEntity.assign(
			mWorldManager.GetEntityCount(),
			EngineCore::NumberOfFrameResources
		);

		if (!SetupPipeline()) { return false; }

		LoadGeometry();

		mRenderer.FinishInitialize();

		mIsInitialized = true;

		return true;
	}

	bool EngineCore::SetupPipeline() {
		return mRenderer.SetupPipeline(
			(uint32_t)mWorldManager.GetEntityCount(),
			mWorldManager.GetConstantBufferDataByteSizeOfEachEntity(),
			mWorldManager.GetConstantBufferDataByteSizeOfEachPerPassObject()
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
		mRenderer.BeginFrame();

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

	bool EngineCore::InitializeCamera(const Engine::Vector3& playerPosition) {
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

	void EngineCore::UpdateConstantBuffers() {
		// update per-pass constant buffers
		
		// DirectXMath uses row-major alignment in CPU memory, but 
		// HLSL defaults to column-major storage for matrix packing. 
		// We transpose here to prevent skewed vector transformations on the GPU.
		const Engine::Matrix4x4* viewProj = mCamera.GetViewProjection();
		DirectX::XMMATRIX viewProjTranspose = DirectX::XMMatrixTranspose(
			DirectX::XMLoadFloat4x4(&viewProj->AsXMFLOAT4X4())
		);
		DirectX::XMStoreFloat4x4(
			&mViewProjectionTranspose.AsXMFLOAT4X4(),
			viewProjTranspose
		);
		mRenderer.UpdatePerPassCb(
			&mViewProjectionTranspose,
			sizeof(Engine::Matrix4x4)
		);

		// update per entity cb
		for (auto& entity : mWorldManager.GetEntities()) {
			uint32_t id = entity.GetID();

			if (entity.GetIsDirty()) {
				mNumberOfDirtyFramesPerEntity[id] = Engine::EngineCore::NumberOfFrameResources;
				entity.SetIsDirty(false);
			}

			if (mNumberOfDirtyFramesPerEntity[id] > 0) {
				Engine::Matrix4x4 transposedData;
				entity.CopyToDestinationConstantBufferDataTransposed(&transposedData);

				mRenderer.UpdatePerRenderItemCb(
					id,
					&transposedData,
					mWorldManager.GetConstantBufferDataByteSizeOfEachEntity()
				);
				mNumberOfDirtyFramesPerEntity[id]--;
			}
		}
	}
}