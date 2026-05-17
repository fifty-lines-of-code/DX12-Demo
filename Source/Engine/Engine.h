#pragma once

#include "../Helper/Helper.h"
#include <memory>
#include "../Helper/MathHelper.h"
#include <vector>

class Camera;
class DX12Renderer;
class Entity;
class GameTimer;
class SceneManager;
class IInputSystem;
class XBoxInputSystem;

class Engine {
public:
	Engine(HINSTANCE hInstance, std::wstring caption, int clientWidth, int clientHeight);
	Engine(const Engine& rhs) = delete;
	Engine& operator=(const Engine& rhs) = delete;
	~Engine();

	bool Initialize(HWND mainWnd, DirectX::XMFLOAT4 playerPosition);
	bool SetupPipeline();

	void Update(float deltaTime, DirectX::XMFLOAT4 playerPosition);
	void Draw();

	const IInputSystem* const GetInputSystem() const;
	Entity* GetPlayerEntity() const;

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
	std::unique_ptr<XBoxInputSystem> mInputSystem;

private:
	bool InitializeCamera(DirectX::XMFLOAT4 playerPosition);
	void LoadGeometry();
};