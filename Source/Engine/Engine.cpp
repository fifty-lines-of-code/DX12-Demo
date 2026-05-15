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

	if (!mRenderer->Initialize(mhMainWnd)) { return false; }

	if (!mSceneManager->Initialize()) { return false; }

	if (!mSceneManager->LoadScene()) { return false; }

	if (!InitializeCamera()) { return false; }

	if (!SetupPipeline()) { return false; }

	LoadGeometry();

	mRenderer->FinishInitialize();

	return true;
}

bool Engine::SetupPipeline() {
	return mRenderer->SetupPipeline(
		(uint32_t)mSceneManager->GetEntityCount(),
		mSceneManager->GetConstantBufferDataByteSizeOfEachEntity()
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
	
	//mCamera->Update();

	// update the scene manager
	DirectX::XMFLOAT4X4 viewProj = mCamera->GetViewProjection();
	mSceneManager->Update(viewProj);

	// update constant buffers of all entities after they have been updated
	for (auto& entity : mSceneManager->GetEntities()) {
		mRenderer->Update(
			(uint32_t)entity->GetMesh()->meshID,
			entity->GetConstantBufferData(),
			sizeof(EntityConstantBufferData)
		);
	}
}

void Engine::Draw() {
	mRenderer->BeginFrame();

	for (auto& entity : mSceneManager->GetEntities()) {
		mRenderer->Draw(
			(uint32_t)entity->GetMesh()->meshID,
			(uint32_t)entity->GetMesh()->GetIndices().size()
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
	std::vector<const Mesh*> meshesToLoad;

	for (auto& entity : mSceneManager->GetEntities()) {
		mRenderer->LoadGeometry(entity->GetMesh());
	}
}