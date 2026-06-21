#include "EngineCore.h"

#include "Camera/Camera.h"
#include "Debug System/DebugSystem.h"
#include <DirectXMath.h>
#include "World Manager/Scene Manager/Entity/Entity.h"
#include "../Game/Game Timer/GameTimer.h"
#include "World Manager/Scene Manager/Resource Manager/Materials Manager/Material/Material.h"
#include "World Manager/Scene Manager/Entity/Mesh/Mesh.h"
#include "../Engine/Input System/XBox/XBoxInputSystem.h"

namespace Engine {

	EngineCore::EngineCore(HINSTANCE hInstance, std::wstring caption) :
		mhAppInst(hInstance),
		mMainWndCaption(caption),
		mIsInitialized(false),
		mAnimationSpeed(.375f), // todo: move this out to somewhere else
		mRenderer(EngineRenderer::DX12Renderer::DX12Renderer()),
		mWorldManager(EngineWorld::WorldManager()),
		mCamera(Camera()),
		mInputSystem(XboxInputSystem()),
		mIsDebugBuild(false)
	{
#ifdef _DEBUG
		mIsDebugBuild = true;
#endif
	}

	EngineCore::~EngineCore() {}

	bool EngineCore::Initialize(HWND mainHwnd, UINT screenWidth, UINT screenHeight) {
		if (screenHeight == 0) { return false; }

		mhMainWnd = mainHwnd;

		if (!mRenderer.Initialize(mhMainWnd, NumberOfFrameResources, screenWidth, screenHeight)) { return false; }

		if (!mWorldManager.Initialize()) { return false; }

		if (!InitializeCamera(mWorldManager.GetPlayerCenter())) { return false; }

		// set all entities to dity so they are updated
		mNumberOfDirtyFramesPerEntity.resize(mWorldManager.GetEntityCount());
		mNumberOfDirtyFramesPerEntity.assign(
			mWorldManager.GetEntityCount(),
			NumberOfFrameResources
		);

		mNumberOfDirtyFramesPerMaterial.resize(mWorldManager.GetMaterialCount());
		mNumberOfDirtyFramesPerMaterial.assign(
			mWorldManager.GetMaterialCount(),
			NumberOfFrameResources
		);

		// Following order of operations is important
		// Load textures first
		LoadTextures();

		// then setup the pipeline
		// which internally sets up the descriptors of the textures
		if (!SetupPipeline()) { return false; }

		LoadGeometry();

		mRenderer.FinishInitialize();

		mIsInitialized = true;

		return true;
	}

	bool EngineCore::SetupPipeline() {
		bool result = mRenderer.SetupRenderPipeline(
			(uint32_t)mWorldManager.GetEntityCount(),
			// todo: configure and use EngineConfig::EngineConfig::MAX_MATERIALS
			mWorldManager.GetMaterialCount(),
			EngineConfig::EngineConfig::MAX_TEXTURES,
			mWorldManager.GetConstantBufferDataByteSizeOfEachMaterialObject(),
			1,
			DebugSystem::DebugLimits::MAX_CHARACTERS
		);

		if (!result) { return false; }

		result = mRenderer.SetupDebugPipeline(
			DebugSystem::DebugLimits::MAX_CHARACTERS,
			(uint32_t)EngineResources::TextureID::FONT
		);

		if (!result) { return false; }

		return mRenderer.SetupBlurPipeline();
	}

	void EngineCore::UpdateInputSystemAndCamera(float deltaTime) {
		mInputSystem.Update();

		mCamera.UpdateWithInputSystem(
			deltaTime,
			mInputSystem.GetRightStickX(),
			mInputSystem.GetRightStickY()
		);
	}

	void EngineCore::Update(float deltaTime) {
		// todo: 

		// calculate our geometry and related data of Debug System
		DebugSystem::DebugSystem::GetInstance().CalculateFramePositions();

		// update the input system first
		UpdateInputSystemAndCamera(deltaTime);

		// update the world manager
		mWorldManager.Update(
			&mInputSystem,
			deltaTime,
			mAnimationSpeed,
			mCamera.GetBasisVectors()
		);

		// update the camera with updated player center
		mCamera.UpdateWithTarget(mWorldManager.GetPlayerCenter());

		// prepare the renderer for updates
		mRenderer.PrepareForUpdate();

		// Update constant buffers.
		UpdateConstantBuffers();
	}

	void EngineCore::Draw(bool drawDebugLayer) {
		// tell renderer to prepare for this frame
		mRenderer.BeginFrame(mWorldManager.GetMaterialCount());

		// TODO: Implement a DAG where nodes are the Pipeline Passes and edges are their 
		// resource dependencies so that we can automate the overall Pipeline 
		// instead of manually having to execute passes like so below

		// draw our 3D objects
		auto& entities = mWorldManager.GetEntities();
		const uint32_t entityCount = mWorldManager.GetEntityCount();

		EngineRenderer::DX12Renderer::DX12PipelinePassExecuteContext context;
		std::array<EngineRenderer::DX12Renderer::DX12RenderItemExecuteContext, EngineConfig::EngineConfig::MAX_ENTITIES> itemsExecuteContext = {};
		context.NumberOfItems = (uint32_t)entities.size();

		for (uint32_t i = 0; i < entities.size(); ++i) {
			if (i >= EngineRenderer::DX12Renderer::DX12RendererConfig::MAX_ITEMS_PER_PASS) { break; }

			auto& entity = entities[i];

			// set ID
			itemsExecuteContext[i].ID = entity.GetID();

			// set Mesh
			auto* mesh = entity.GetMesh();
			itemsExecuteContext[i].IndexCount = (uint32_t)mesh->GetIndices().size();
			itemsExecuteContext[i].MeshID = (uint32_t)mesh->GetMeshID();
		}
		context.RenderItems = itemsExecuteContext.data();
		context.NumberOfMaterials = mWorldManager.GetMaterialCount();
		context.PipelinePass = EngineRenderer::RendererPipelinePass::OPAQUE_RENDER_PASS;
		mRenderer.Execute(context);

		// draw our debug system
		uint32_t noCharsToDraw = DebugSystem::DebugSystem::GetInstance().GetTotalNumberOfCharacersToDraw();

		if (mIsDebugBuild && drawDebugLayer && noCharsToDraw > 0) {
			context.NumberOfItems = noCharsToDraw;
			context.PipelinePass = EngineRenderer::RendererPipelinePass::DEBUG_SYSTEM_PASS;
			mRenderer.Execute(context);
		}

		// tell renderer to wrap up this frame
		mRenderer.EndFrame();

		// clear the cache for next frame
		DebugSystem::DebugSystem::GetInstance().ClearFrameCache();
	}

	void EngineCore::OnResize(UINT newClientWidth, UINT newClientHeight) {
		if (!mIsInitialized) { return; }

		mRenderer.OnResize(newClientWidth, newClientHeight);
		mCamera.OnResize(newClientWidth, newClientHeight);
		DebugSystem::DebugSystem::GetInstance().UpdateWindowDimensions(newClientWidth, newClientHeight);
	}

	bool EngineCore::InitializeCamera(const Vector3& playerPosition) {
		mCamera.Initialize(playerPosition);
		return true;
	}

	void EngineCore::LoadGeometry() {
		std::vector<const Mesh*> meshesToLoad;
		mWorldManager.GetMeshesToLoad(meshesToLoad);

		for (const Mesh* mesh : meshesToLoad) {
			mRenderer.LoadGeometry(
				(uint32_t)mesh->GetMeshID(),
				sizeof(Vertex),
				mesh->GetVbByteSize(),
				(void*)mesh->GetVertices().data(),
				mesh->GetIbByteSize(),
				(void*)mesh->GetIndices().data()
			);
		}
	}

	void EngineCore::LoadTextures() {
		for (EngineResources::TextureAsset& texture : mWorldManager.GetTextures()) {
			if (texture.isLoaded) { continue; }
			if (!texture.isReadyToLoad) { continue; }

			bool result = mRenderer.LoadTexture(
				texture.FileName,
				(uint32_t)texture.id
			);

			if (result) {
				texture.isLoaded = true;
				texture.isReadyToLoad = false;
			}
		}
	}

	void EngineCore::UpdateConstantBuffers() {
		// update per-pass constant buffers
		UpdatePerPassConstantBuffers();

		// update per entity cb
		UpdatePerEntityConstantBuffers();
		
		// update per material cb
		UpdatePerMaterialConstantBuffers();

		// update debug system cb
		UpdateDebugSystemConstantBuffers();
	}

	void EngineCore::UpdatePerPassConstantBuffers() const {
		PerPassConstantBufferData perPassCB;

		// DirectXMath uses row-major alignment in CPU memory, but 
		// HLSL defaults to column-major storage for matrix packing. 
		// We transpose here to prevent skewed vector transformations on the GPU.
		const Matrix4x4& viewProj = mCamera.GetViewProjection();
		DirectX::XMMATRIX viewProjTranspose = DirectX::XMMatrixTranspose(
			DirectX::XMLoadFloat4x4(&viewProj.AsXMFLOAT4X4())
		);
		DirectX::XMStoreFloat4x4(
			&perPassCB.ViewProjectionTranspose.AsXMFLOAT4X4(),
			viewProjTranspose
		);

		// set camera pos
		const Vector3& cameraPos = mCamera.GetPosition();
		perPassCB.EyePosW = cameraPos;

		// set ambient light
		perPassCB.AmbientLight = mWorldManager.GetAmbientLight();

		// set light data
		mWorldManager.GetLightsData(perPassCB.Lights);

		mRenderer.UpdateOpaqueRenderItemsPerPassCb(
			&perPassCB,
			sizeof(PerPassConstantBufferData)
		);
	}

	void EngineCore::UpdatePerEntityConstantBuffers() {
		EntityConstantBufferData bufferData;

		for (auto& entity : mWorldManager.GetEntities()) {
			uint32_t id = entity.GetID();

			if (entity.GetIsDirty()) {
				mNumberOfDirtyFramesPerEntity[id] = NumberOfFrameResources;
				entity.SetIsDirty(false);
			}

			if (mNumberOfDirtyFramesPerEntity[id] > 0) {
				entity.CopyToDestinationConstantBufferDataTransposed(bufferData);

				mRenderer.UpdateOpaqueRenderItemCb(
					id,
					&bufferData,
					mWorldManager.GetConstantBufferDataByteSizeOfEachEntity()
				);
				mNumberOfDirtyFramesPerEntity[id]--;
			}
		}
	}

	void EngineCore::UpdatePerMaterialConstantBuffers() {
		for (auto& material : mWorldManager.GetMaterials()) {
			uint16_t id = (uint16_t)material.GetType();

			if (material.GetIsDirty()) {
				mNumberOfDirtyFramesPerMaterial[id] = NumberOfFrameResources;
				material.SetIsDirty(false);
			}

			if (mNumberOfDirtyFramesPerMaterial[id] > 0) {
				mRenderer.UpdatePerMaterialCb(
					id,
					&material.GetData(),
					mWorldManager.GetConstantBufferDataByteSizeOfEachMaterialObject()
				);
				mNumberOfDirtyFramesPerMaterial[id]--;
			}
		}
	}

	void EngineCore::UpdateDebugSystemConstantBuffers() {
		DebugSystem::DebugSystem& dSystem = DebugSystem::DebugSystem::GetInstance();

		// update per pass buffer
		DebugSystem::DebugSystemPerPassCbData perPassCb;
		dSystem.GetPerPassCbData(perPassCb);
		mRenderer.UpdateDebugSystemPerPassCb(&perPassCb);

		// update per glyph buffers
		if (dSystem.GetIsDirty()) {
			mNumberOfDirtyFramesDebugSystem = NumberOfFrameResources;
			dSystem.SetIsDirty(false);
		}

		if (mNumberOfDirtyFramesDebugSystem > 0) {
			mNumberOfDirtyFramesDebugSystem--;
			// fire off the data to the renderer to upload to gpu
			const DebugSystem::TextVerticesArray& vertices = dSystem.GetVertices();
			uint32_t numberOfGlyphsToDraw = dSystem.GetTotalNumberOfCharacersToDraw();

			mRenderer.UpdateDebugSystemStructuredBuffer(numberOfGlyphsToDraw, &vertices);
		}
	}
}