#include "Engine.h"

#include "../Renderer/DX12Renderer/DX12Renderer.h"
#include "Scene Manager/SceneManager.h"
#include "Scene Manager/Entities/Mesh/Mesh.h"
#include "Scene Manager/Entities/Entity.h"
#include "../Game Timer/GameTimer.h"
#include "Camera/Camera.h"

using namespace DirectX;

Engine::Engine(HINSTANCE hInstance, std::wstring caption, int clientWidth, int clientHeight) :
	mhAppInst(hInstance), 
	mMainWndCaption(caption),
	mClientWidth(clientWidth), 
	mClientHeight(clientHeight),
	mRenderer(std::make_unique<DX12Renderer>(clientWidth, clientHeight)),
	mSceneManager(std::make_unique<SceneManager>()),
	mCamera(std::make_unique<Camera>(clientWidth / (float) clientHeight))
{}

Engine::~Engine() {}

bool Engine::Initialize(HWND mainHwnd) {
	mhMainWnd = mainHwnd;

	if (!mRenderer->Initialize(mhMainWnd, Engine::NumberOfFrameResources)) { return false; }

	if (!mSceneManager->Initialize()) { return false; }

	if (!InitializeCamera()) { return false; }

	if (!mSceneManager->LoadScene()) { return false; }

	// set all entities to dity so they are updated
	mNumberOfDirtyFramesPerEntity.resize(mSceneManager->GetEntityCount()); mNumberOfDirtyFramesPerEntity.assign(mSceneManager->GetEntityCount(), 3);

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

void Engine::OnResize(UINT newClientWidth, UINT newClientHeight) {
	mRenderer->OnResize(newClientWidth, newClientHeight);
	mCamera->OnResize(newClientWidth, newClientHeight);
}

void Engine::Update(const GameTimer* const mTimer) {
	// todo: 

	// update stats
	CalculateFrameStats(mTimer);

	//controller update
	//mController->Update();
	
	mCamera->Update();

	// update the scene manager
	const DirectX::XMFLOAT4X4* viewProj = mCamera->GetViewProjection();
	mSceneManager->Update(viewProj);

	// prepare the renderer for updates
	mRenderer->PrepareForUpdate();

	// update per pass cbs, always send transpose of matrices
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
			mNumberOfDirtyFramesPerEntity[entity->GetID()] = 3;
			entity->SetIsDirty(false);
		}

		if (mNumberOfDirtyFramesPerEntity[entity->GetID()] > 0) {
			auto transposedData = entity->GetConstantBufferDataTransposeIfNecessray();
			mRenderer->UpdatePerRenderItemCb(
				entity->GetID(),
				&transposedData,
				mSceneManager->GetConstantBufferDataByteSizeOfEachEntity()
			);
			mNumberOfDirtyFramesPerEntity[entity->GetID()]--;
		}
	}
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

void Engine::CalculateFrameStats(const GameTimer* const timer) {
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((timer->TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = mMainWndCaption +
			L"  fps: " + fpsStr +
			L"  mspf: " + mspfStr;

		SetWindowText(mhMainWnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

bool Engine::InitializeCamera() {
	// do something, maybe

	return true;
}

void Engine::LoadGeometry() {
	const std::vector<std::unique_ptr<Entity>>* entitiesToLoad = mSceneManager->GetEntities();

	for (auto& mesh : mSceneManager->GetMeshesToLoad()) {
		mRenderer->LoadGeometry(mesh);
	}
}