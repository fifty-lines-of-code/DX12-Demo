#pragma once

#include "Camera/Camera.h"
#include "../Renderer/DX12Renderer/DX12Renderer.h"
#include "Math/EngineMath.h"
#include "../Helper/Helper.h"
#include "Scene Manager/SceneManager.h"
#include "Input System/Xbox/XboxInputSystem.h"
#include <memory>
#include <vector>

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

		bool Initialize(HWND mainWnd, const Engine::Vector3* playerPosition);
		bool SetupPipeline();

		void UpdateInputSystemAndCamera(float deltaTime);
		void Update(float deltaTime, const Engine::Vector3* playerPosition);
		const BasisVectors* GetCameraBasisVectors() const;

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

		DX12Renderer mRenderer;
		SceneManager mSceneManager;
		Camera mCamera;
		std::vector<uint32_t> mNumberOfDirtyFramesPerEntity;
		XboxInputSystem mInputSystem;
		// per pass cb
		Engine::Matrix4x4 mViewProjectionTranspose;

	private:
		bool InitializeCamera(const Engine::Vector3* playerPosition);
		void LoadGeometry();
	};
}