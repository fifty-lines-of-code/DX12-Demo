#pragma once

#include "Audio System/AudioSystem.h"
#include "Camera Manager/CameraManager.h"
#include "Renderer/DX12Renderer/DX12Renderer.h"
#include "Math/EngineMath.h"
#include "../Helper/Helper.h"
#include "Camera Manager/Main Camera/MainCamera.h"
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
		~EngineCore();

		EngineCore(const EngineCore& rhs) = delete;
		EngineCore& operator=(const EngineCore& rhs) = delete;
		EngineCore(EngineCore&&) = delete;
		EngineCore& operator=(EngineCore&&) = delete;

		bool Initialize(
			HWND mainWnd, 
			UINT fullscreenWidth, 
			UINT fullscreenHeight
		);
		void Update(float deltaTime);
		void Draw(bool drawDebugLayer);
		void OnResize(
			UINT newClientWidth,
			UINT newClientHeight
		);
		void LogToDebugSystem(const std::string& log);

	private:
		EngineAudio::AudioSystem mAudioSystem;
		EngineWorld::WorldManager mWorldManager;
		EngineRenderer::DX12Renderer::DX12Renderer mRenderer;
		EngineCamera::CameraManager mCameraManager;
		XboxInputSystem mInputSystem;
		std::wstring mMainWndCaption;
		std::vector<uint32_t> mNumberOfDirtyFramesPerEntity;
		std::vector<uint32_t> mNumberOfDirtyFramesPerMaterial;
		// application instance handle
		HINSTANCE mhAppInst = nullptr;
		// main window handle
		HWND mhMainWnd = nullptr;
		uint32_t mNumberOfDirtyFramesDebugSystem = NumberOfFrameResources;
		static const int NumberOfFrameResources = 3;
		const float mAnimationSpeed;
		bool mIsInitialized;
		bool mIsDebugBuild;

	private:
		bool InitializeCameras(const Vector3& playerPosition);
		bool InitializeRenderer(
			UINT screenWidth, 
			UINT screenHeight
		);
		void LoadTextures();
		bool SetupPipelines();
		void LoadGeometry();
		void UpdateInputSystemAndMainCamera(float deltaTime);
		void UpdateConstantBuffers();
		void UpdateOpaquePassEntitiesPerPassConstantBuffers() const;
		void UpdateMirrorPassEntitiesPerPassConstantBuffers() const;
		void UpdatePerEntityConstantBuffers();
		void UpdatePerMaterialConstantBuffers();
		void UpdateDebugSystemConstantBuffers();
		void DrawMirrorRenderPass();
		void DrawOpaqueRenderPass();
		void LogProfilingData();
	};
}