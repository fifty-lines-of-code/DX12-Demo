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
		EngineCore(HINSTANCE hInstance, std::wstring caption);
		EngineCore(const EngineCore& rhs) = delete;
		EngineCore& operator=(const EngineCore& rhs) = delete;
		~EngineCore();

		bool Initialize(HWND mainWnd, UINT fullscreenWidth, UINT fullscreenHeight);
		bool SetupPipeline();

		void UpdateInputSystemAndCamera(float deltaTime);
		void Update(float deltaTime);

		void Draw();

		void OnResize(UINT newClientWidth, UINT newClientHeight);

	private:
		WorldManager mWorldManager;
		DX12Renderer mRenderer;
		Camera mCamera;
		XboxInputSystem mInputSystem;
		std::wstring mMainWndCaption;
		std::vector<uint32_t> mNumberOfDirtyFramesPerEntity;
		std::vector<uint32_t> mNumberOfDirtyFramesPerMaterial;
		HINSTANCE mhAppInst = nullptr; // application instance handle
		HWND mhMainWnd = nullptr;
		static const int NumberOfFrameResources = 3;
		bool mIsInitialized;
		const float mAnimationSpeed;

	private:
		bool InitializeCamera(const Vector3& playerPosition);
		void LoadGeometry();
		void UpdateConstantBuffers();
		void UpdatePerPassConstantBuffers() const;
		void UpdatePerEntityConstantBuffers();
		void UpdatePerMaterialConstantBuffers();
	};
}