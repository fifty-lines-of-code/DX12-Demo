#include "EngineCore.h"

#include "Camera/Camera.h"
#include <DirectXMath.h>
#include "Scene Manager/Entities/Entity.h"
#include "../Game Timer/GameTimer.h"
#include "Scene Manager/Entities/Mesh/Mesh.h"
#include "Scene Manager/SceneManager.h"
#include "../Engine/Input System/XBox/XBoxInputSystem.h"

namespace Engine {

	EngineCore::EngineCore(HINSTANCE hInstance, std::wstring caption, int clientWidth, int clientHeight) :
		mhAppInst(hInstance),
		mMainWndCaption(caption),
		mClientWidth(clientWidth),
		mClientHeight(clientHeight),
		mAnimationSpeed(.375f), // todo: move this out to somewhere else
		mRenderer(DX12Renderer(clientWidth, clientHeight)),
		mSceneManager(SceneManager()),
		mCamera(Camera(clientWidth / (float)clientHeight)),
		mInputSystem(XboxInputSystem())
	{}

	EngineCore::~EngineCore() {}

	bool EngineCore::Initialize(HWND mainHwnd, const Engine::Vector3* playerPosition) {
		mhMainWnd = mainHwnd;

		if (!mRenderer.Initialize(mhMainWnd, EngineCore::NumberOfFrameResources)) { return false; }

		if (!mSceneManager.Initialize()) { return false; }

		if (!InitializeCamera(playerPosition)) { return false; }

		if (!mSceneManager.LoadScene()) { return false; }

		// set all entities to dity so they are updated
		mNumberOfDirtyFramesPerEntity.resize(mSceneManager.GetEntityCount());
		mNumberOfDirtyFramesPerEntity.assign(
			mSceneManager.GetEntityCount(),
			EngineCore::NumberOfFrameResources
		);

		if (!SetupPipeline()) { return false; }

		LoadGeometry();

		mRenderer.FinishInitialize();

		return true;
	}

	bool EngineCore::SetupPipeline() {
		return mRenderer.SetupPipeline(
			(uint32_t)mSceneManager.GetEntityCount(),
			mSceneManager.GetConstantBufferDataByteSizeOfEachEntity(),
			mSceneManager.GetConstantBufferDataByteSizeOfEachPerPassObject()
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

	void EngineCore::Update(float deltaTime, const Engine::Vector3* playerPosition) {
		// todo: 

		// update the camera with player pos
		mCamera.UpdateWithTarget(playerPosition);

		// update the scene manager
		mSceneManager.Update(
			&mInputSystem,
			deltaTime,
			mAnimationSpeed
		);

		// prepare the renderer for updates
		mRenderer.PrepareForUpdate();

		// Update per-pass constant buffers.

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
		for (auto& entity : *mSceneManager.GetEntities()) {
			uint32_t id = entity->GetID();

			if (entity->GetIsDirty()) {
				mNumberOfDirtyFramesPerEntity[id] = Engine::EngineCore::NumberOfFrameResources;
				entity->SetIsDirty(false);
			}

			if (mNumberOfDirtyFramesPerEntity[id] > 0) {
				Engine::Matrix4x4 transposedData;
				entity->CopyToDestinationConstantBufferDataTransposed(&transposedData);

				mRenderer.UpdatePerRenderItemCb(
					id,
					&transposedData,
					mSceneManager.GetConstantBufferDataByteSizeOfEachEntity()
				);
				mNumberOfDirtyFramesPerEntity[id]--;
			}
		}
	}

	const BasisVectors* EngineCore::GetCameraBasisVectors() const {
		return mCamera.GetBasisVectors();
	}

	void EngineCore::Draw() {
		mRenderer.BeginFrame();

		for (auto& entity : *mSceneManager.GetEntities()) {
			mRenderer.Draw(
				(uint32_t)entity->GetMesh()->GetMeshID(),
				(uint32_t)entity->GetMesh()->GetIndices().size(),
				entity->GetID(),
				mSceneManager.GetEntityCount()
			);
		}
		mRenderer.EndFrame();
	}

	const IInputSystem* const EngineCore::GetInputSystem() const {
		return &mInputSystem;
	}

	Entity* EngineCore::GetPlayerEntity() const {
		return mSceneManager.GetPlayerEntity();
	}

	void EngineCore::OnResize(UINT newClientWidth, UINT newClientHeight) {
		mRenderer.OnResize(newClientWidth, newClientHeight);
		mCamera.OnResize(newClientWidth, newClientHeight);
	}

	void EngineCore::SetFullscreen() {
		mRenderer.SetFullscreen();
		mClientWidth = mRenderer.GetClientWidth();
		mClientHeight = mRenderer.GetClientHeight();
		mCamera.OnResize(mClientWidth, mClientHeight);
	}

	void EngineCore::SetWindowed(UINT clientWidth, UINT clientHeight) {
		mClientWidth = clientWidth;
		mClientHeight = clientHeight;
		mRenderer.SetWindowed();
		mCamera.OnResize(clientWidth, clientHeight);

	}

	bool EngineCore::InitializeCamera(const Engine::Vector3* playerPosition) {
		mCamera.Initialize(playerPosition);
		return true;
	}

	void EngineCore::LoadGeometry() {
		for (auto& mesh : mSceneManager.GetMeshesToLoad()) {
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
}