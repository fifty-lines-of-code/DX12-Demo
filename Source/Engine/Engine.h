#pragma once

#include "../Helper/Helper.h"
#include <memory>
#include "../Helper/MathHelper.h"

class DX12Renderer;
class SceneManager;
class Camera;
class GameTimer;

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
	HINSTANCE mhAppInst = nullptr; // application instance handle
	HWND mhMainWnd = nullptr;
	std::wstring mMainWndCaption;
	int mClientWidth;
	int mClientHeight;

	std::unique_ptr<DX12Renderer> mRenderer;
	std::unique_ptr<SceneManager> mSceneManager;
	std::unique_ptr<Camera> mCamera;

private:
	bool InitializeCamera();
	void LoadGeometry();
};