#pragma once

#include "../Helper/Helper.h"
#include <memory>
#include "../Helper/MathHelper.h"
#include <vector>

class Camera;
class DX12Renderer;
class GameTimer;
class IInputSystem;
class SceneManager;

class Engine {
public:
	Engine(HINSTANCE hInstance, std::wstring caption, int clientWidth, int clientHeight);
	Engine(const Engine& rhs) = delete;
	Engine& operator=(const Engine& rhs) = delete;
	~Engine();

	bool Initialize(HWND mainWnd);
	bool SetupPipeline();

	void Update(const GameTimer* const mTimer, const IInputSystem* const inputSystem);
	void Draw();

	void OnResize(UINT newClientWidth, UINT newClientHeight);

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

private:
	bool InitializeCamera();
	void LoadGeometry();
};