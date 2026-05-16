#pragma once

#include "../Helper/Helper.h"
#include <memory>
#include "../Helper/MathHelper.h"
#include <vector>

class Camera;
class DX12Renderer;
class GameTimer;
class SceneManager;
class XBoxInputSystem;

class Engine {
public:
	Engine(HINSTANCE hInstance, std::wstring caption, int clientWidth, int clientHeight);
	~Engine();

	bool Initialize(HWND mainWnd);
	bool SetupPipeline();
	void OnResize(UINT newClientWidth, UINT newClientHeight);
	void Update(const GameTimer* const mTimer);
	void Draw();
	void CalculateFrameStats(const GameTimer* const timer);

private:
	static const int NumberOfFrameResources = 3;
	HINSTANCE mhAppInst = nullptr; // application instance handle
	HWND mhMainWnd = nullptr;
	std::wstring mMainWndCaption;
	int mClientWidth;
	int mClientHeight;
	const float mAnimationSpeed;

	std::unique_ptr<DX12Renderer> mRenderer;
	std::unique_ptr<SceneManager> mSceneManager;
	std::unique_ptr<Camera> mCamera;
	std::vector<uint32_t> mNumberOfDirtyFramesPerEntity;
	std::unique_ptr<XBoxInputSystem> mInputSystem;

private:
	bool InitializeCamera();
	void LoadGeometry();
};