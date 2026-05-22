#include "Engine.h"

#include "Camera/Camera.h"
#include "../Renderer/DX12Renderer/DX12Renderer.h"
#include "Scene Manager/Entities/Entity.h"
#include "../Game Timer/GameTimer.h"
#include "Scene Manager/Entities/Mesh/Mesh.h"
#include "Scene Manager/SceneManager.h"
#include "../Engine/Input System/XBox/XBoxInputSystem.h"

using namespace DirectX;

Engine::Engine(HINSTANCE hInstance, std::wstring caption, int clientWidth, int clientHeight) :
	mhAppInst(hInstance), 
	mMainWndCaption(caption),
	mClientWidth(clientWidth), 
	mClientHeight(clientHeight),
	mAnimationSpeed(.375f), // todo: move this out to somewhere else
	mRenderer(std::make_unique<DX12Renderer>(clientWidth, clientHeight)),
	mSceneManager(std::make_unique<SceneManager>()),
	mCamera(std::make_unique<Camera>(clientWidth / (float) clientHeight)),
	mInputSystem(std::make_unique<XboxInputSystem>())
{}

Engine::~Engine() {}

bool Engine::Initialize(HWND mainHwnd, DirectX::XMFLOAT4 playerPosition) {
	mhMainWnd = mainHwnd;

	if (!mRenderer->Initialize(mhMainWnd, Engine::NumberOfFrameResources)) { return false; }

	if (!mSceneManager->Initialize()) { return false; }

	if (!InitializeCamera(playerPosition)) { return false; }

	if (!mSceneManager->LoadScene()) { return false; }

	// set all entities to dity so they are updated
	mNumberOfDirtyFramesPerEntity.resize(mSceneManager->GetEntityCount()); 
	mNumberOfDirtyFramesPerEntity.assign(
		mSceneManager->GetEntityCount(), 
		Engine::NumberOfFrameResources
	);

	if (!SetupPipeline()) { return false; }

	LoadGeometry();

	mRenderer->FinishInitialize();

	return true;
}

bool Engine::SetupPipeline() {
	return mRenderer->SetupPipeline(
		(uint32_t)mSceneManager->GetEntityCount(),
		mSceneManager->GetConstantBufferDataByteSizeOfEachEntity(),
		mSceneManager->GetConstantBufferDataByteSizeOfEachPerPassObject()
	);
}

void Engine::UpdateInputSystemAndCamera(float deltaTime) {
	mInputSystem->Update();

	mCamera->UpdateWithInputSystem(
		deltaTime,
		mInputSystem->GetRightStickX(),
		mInputSystem->GetRightStickY()
	);
}

void Engine::Update(float deltaTime, DirectX::XMFLOAT4 playerPosition) {
	// todo: 

	// update the camera with player pos
	mCamera->UpdateWithTarget(playerPosition);

	// update the scene manager
	mSceneManager->Update(
		mInputSystem.get(),
		deltaTime,
		mAnimationSpeed
	);

	// prepare the renderer for updates
	mRenderer->PrepareForUpdate();

	// Update per-pass constant buffers.
	
	// DirectXMath uses row-major alignment in CPU memory, but 
	// HLSL defaults to column-major storage for matrix packing. 
	// We transpose here to prevent skewed vector transformations on the GPU.
	const DirectX::XMFLOAT4X4* viewProj = mCamera->GetViewProjection();
	DirectX::XMMATRIX viewProjTranspose = DirectX::XMLoadFloat4x4(viewProj);
	viewProjTranspose = DirectX::XMMatrixTranspose(viewProjTranspose);
	DirectX::XMFLOAT4X4 viewProjTranspose44;
	DirectX::XMStoreFloat4x4(
		&viewProjTranspose44,
		viewProjTranspose
	);
	mRenderer->UpdatePerPassCb(&viewProjTranspose44, sizeof(DirectX::XMFLOAT4X4));

	// update per entity cb
	for (auto& entity : *mSceneManager->GetEntities()) {
		if (entity->GetIsDirty()) {
			mNumberOfDirtyFramesPerEntity[entity->GetID()] = Engine::NumberOfFrameResources;
			entity->SetIsDirty(false);
		}

		if (mNumberOfDirtyFramesPerEntity[entity->GetID()] > 0) {
			auto transposedData = entity->GetConstantBufferDataTransposed();
			mRenderer->UpdatePerRenderItemCb(
				entity->GetID(),
				&transposedData,
				mSceneManager->GetConstantBufferDataByteSizeOfEachEntity()
			);
			mNumberOfDirtyFramesPerEntity[entity->GetID()]--;
		}
	}
}

BasisVectors Engine::GetCameraForwardAndRightVectors() const {
	return mCamera->GetForwardAndRightVectors();
}

void Engine::Draw() {
	mRenderer->BeginFrame();

	for (auto& entity : *mSceneManager->GetEntities()) {
		mRenderer->Draw(
			(uint32_t)entity->GetMesh()->meshID,
			(uint32_t)entity->GetMesh()->GetIndices().size(),
			entity->GetID(),
			mSceneManager->GetEntityCount()
		);
	}
	mRenderer->EndFrame();
}

const IInputSystem* const Engine::GetInputSystem() const {
	return mInputSystem.get();
}

Entity* Engine::GetPlayerEntity() const {
	return mSceneManager->GetPlayerEntity();
}

void Engine::OnResize(UINT newClientWidth, UINT newClientHeight) {
	mRenderer->OnResize(newClientWidth, newClientHeight);
	mCamera->OnResize(newClientWidth, newClientHeight);
}

bool Engine::InitializeCamera(DirectX::XMFLOAT4 playerPosition) {
	mCamera->Initialize(playerPosition);
	return true;
}

void Engine::LoadGeometry() {
	for (auto& mesh : mSceneManager->GetMeshesToLoad()) {
		mRenderer->LoadGeometry(mesh);
	}
}