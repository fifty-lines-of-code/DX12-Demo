#pragma once

#include "Camera/Camera.h"
#include "Renderer/DX12Renderer/DX12Renderer.h"
#include "Math/EngineMath.h"
#include "../Helper/Helper.h"
#include <memory>
#include <vector>
#include "World Manager/WorldManager.h"
#include "Input System/Xbox/XboxInputSystem.h"

struct BasisVectors;
class Entity;
class GameTimer;

namespace Engine {
	class EngineCore {
	public:
		EngineCore(HINSTANCE hInstance, std::wstring caption, int clientWidth, int clientHeight);
		EngineCore(const EngineCore& rhs) = delete;
		EngineCore& operator=(const EngineCore& rhs) = delete;
		~EngineCore();

		bool Initialize(HWND mainWnd);
		bool SetupPipeline();

		void UpdateInputSystemAndCamera(float deltaTime);
		void Update(float deltaTime);

		void Draw();

		void OnResize(UINT newClientWidth, UINT newClientHeight);
		void SetWindowed(UINT clientWidth, UINT clientHeight);
		void SetFullscreen();

	private:
		static const int NumberOfFrameResources = 3;
		HINSTANCE mhAppInst = nullptr; // application instance handle
		HWND mhMainWnd = nullptr;
		std::wstring mMainWndCaption;
		int mWindowedClientWidth;
		int mWindowedClientHeight;
		bool mIsInitialized;
		const float mAnimationSpeed;

		DX12Renderer mRenderer;
		WorldManager mWorldManager;
		Camera mCamera;
		std::vector<uint32_t> mNumberOfDirtyFramesPerEntity;
		XboxInputSystem mInputSystem;
		// per pass cb
		Engine::Matrix4x4 mViewProjectionTranspose;

	private:
		bool InitializeCamera(const Engine::Vector3& playerPosition);
		void LoadGeometry();
		void UpdateConstantBuffers();
	};
}