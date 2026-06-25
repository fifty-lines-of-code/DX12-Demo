#include "EngineCore.h"

#include "Camera/Camera.h"
#include "Debug System/DebugSystem.h"
#include <DirectXMath.h>
#include "World Manager/Scene Manager/Entity/Entity.h"
#include "../Game/Game Timer/GameTimer.h"
#include "World Manager/Scene Manager/Resource Manager/Materials Manager/Material/Material.h"
#include "World Manager/Scene Manager/Resource Manager/Mesh Generator/Mesh/Mesh.h"
#include "../Engine/Input System/XBox/XBoxInputSystem.h"

namespace Engine {

	EngineCore::EngineCore(HINSTANCE hInstance, std::wstring caption) :
		mhAppInst(hInstance),
		mMainWndCaption(caption),
		mIsInitialized(false),
		mAnimationSpeed(.375f),
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

		if (!mAudioSystem.Initialize()) { return false; }

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
		if (!SetupPipelines()) { return false; }

		LoadGeometry();

		mRenderer.FinishInitialize();

		// start the background audio
		mAudioSystem.StartBackgroundLoop(EngineAudio::SoundBG::BOSSFIGHT);

		mIsInitialized = true;

		return true;
	}

	bool EngineCore::SetupPipelines() {

		// opaque render pipeline
		bool result = mRenderer.SetupOpaqueRenderPipeline(
			(uint32_t)mWorldManager.GetEntityCount(),
			EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH,
			// todo: configure and use EngineConfig::EngineConfig::MAX_MATERIALS
			mWorldManager.GetMaterialCount(),
			EngineConfig::EngineConfig::MAX_TEXTURES,
			mWorldManager.GetConstantBufferDataByteSizeOfEachMaterialObject(),
			1,
			DebugSystem::DebugLimits::MAX_CHARACTERS
		);

		if (!result) { return false; }

		// debug pipeline
		result = mRenderer.SetupDebugPipeline(
			DebugSystem::DebugLimits::MAX_CHARACTERS,
			(uint32_t)EngineResources::TextureID::FONT
		);

		if (!result) { return false; }

		// blur pipeline
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

		// update the input system first
		UpdateInputSystemAndCamera(deltaTime);

		// update the audio system
		mAudioSystem.Update(mInputSystem, deltaTime);

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

		// draw the time profiling data once gpu has finished previous frame
		if (mIsDebugBuild) { LogProfilingData(); }

		// calculate our geometry and related data of Debug System
		DebugSystem::DebugSystem::GetInstance().CalculateFramePositions();

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
		DrawOpaqueRenderPass();

		// draw our debug system
		uint32_t noCharsToDraw = DebugSystem::DebugSystem::GetInstance().GetTotalNumberOfCharacersToDraw();

		if (mIsDebugBuild && drawDebugLayer && noCharsToDraw > 0) {
			EngineRenderer::DX12Renderer::DX12PipelinePassExecuteContext context;
			context.NumberOfItems = noCharsToDraw;
			context.PipelinePass = EngineRenderer::RendererPipelinePass::DEBUG_SYSTEM_PASS;
			mRenderer.Execute(context);
		}

		// tell renderer to wrap up this frame
		mRenderer.EndFrame();

		// clear the debug cache for next frame
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
		const EngineResources::MeshArray& meshesToLoad = mWorldManager.GetMeshesToLoad();

		for (const EngineResources::Mesh& mesh : meshesToLoad) {
			mRenderer.LoadGeometry(
				(uint32_t)mesh.GetMeshID(),
				sizeof(Vertex),
				mesh.GetVbByteSize(),
				(void*)mesh.GetVertices().data(),
				mesh.GetIbByteSize(),
				(void*)mesh.GetIndices().data()
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
		EngineWorld::PerPassConstantBufferData perPassCB;

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
			sizeof(EngineWorld::PerPassConstantBufferData)
		);
	}

	void EngineCore::UpdatePerEntityConstantBuffers() {
		EngineWorld::EntityConstantBufferData bufferData;

		for (EngineWorld::Entity& entity : mWorldManager.GetEntities()) {
			uint32_t id = entity.GetID();

			if (entity.GetIsDirty()) {
				mNumberOfDirtyFramesPerEntity[id] = NumberOfFrameResources;
				entity.SetIsDirty(false);
			}

			if (mNumberOfDirtyFramesPerEntity[id] > 0) {
				entity.CopyToDestinationEntityConstantBufferDataTransposed(bufferData);

				// update per entity data
				mRenderer.UpdateOpaqueRenderItemCb(
					id,
					&bufferData,
					mWorldManager.GetConstantBufferDataByteSizeOfEachEntity()
				);

				const EngineResources::Mesh* mesh = entity.GetMesh();
				std::array<EngineWorld::EntitySubMeshConstantBufferData, EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH> subMeshData;

				// update per mesh data
				for (uint8_t i = 0; i < mesh->GetActiveSubMeshCount(); ++i) {
					entity.CopyToDestinationSubMeshConstantBufferData(
						i,
						subMeshData[i]
					);
				}

				// fire off sub mesh data per entity
				mRenderer.UpdateOpaqueRenderItemSubMeshCb(
					id,
					EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH,
					&subMeshData,
					sizeof(EngineWorld::EntitySubMeshConstantBufferData)
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

	void EngineCore::DrawOpaqueRenderPass() {
		auto& entities = mWorldManager.GetEntities();
		const uint32_t entityCount = mWorldManager.GetEntityCount();

		// excute context of all render items
		std::array<EngineRenderer::DX12Renderer::DX12OpaqueRenderItemExecuteContext, EngineConfig::EngineConfig::MAX_ENTITIES> itemsExecuteContext;

		// opaque pipeline pass execute context
		EngineRenderer::DX12Renderer::DX12OpaquePipelinePassExecuteContext context;

		// load up the context
		context.NumberOfItems = (uint32_t)entities.size();
		context.MaxNumSubMeshesPerItem = EngineConfig::EngineConfig::MAX_SUBMESHES_PER_MESH;
		context.NumberOfMaterials = mWorldManager.GetMaterialCount();
		context.PipelinePass = EngineRenderer::RendererPipelinePass::OPAQUE_RENDER_PASS;

		for (uint32_t i = 0; i < entities.size(); ++i) {
			if (i >= EngineRenderer::DX12Renderer::DX12RendererConfig::MAX_ITEMS_PER_PASS) { break; }

			auto& entity = entities[i];

			// set ID
			itemsExecuteContext[i].ID = entity.GetID();

			// set Mesh ID
			auto* mesh = entity.GetMesh();
			itemsExecuteContext[i].MeshID = (uint32_t)mesh->GetMeshID();

			// set submesh count
			uint8_t subMeshCount = mesh->GetActiveSubMeshCount();
			itemsExecuteContext[i].SubMeshCount = mesh->GetActiveSubMeshCount();

			// update per entity sub mesh data
			for (uint8_t j = 0; j < subMeshCount; ++j) {
				EngineRenderer::DX12Renderer::DX12OpaqueRenderItemPerSubMeshExecuteContext subMeshExecuteContext;
				const EngineResources::SubMesh& subMeshAtJ = mesh->GetSubMeshAtIndex(j);

				subMeshExecuteContext.ID = j;
				subMeshExecuteContext.IndexCount = subMeshAtJ.IndexCount;
				subMeshExecuteContext.StartIndexLocation = subMeshAtJ.StartIndexLocation;
				subMeshExecuteContext.BaseVertexLocation = subMeshAtJ.BaseVertexLocation;
				itemsExecuteContext[i].SubMeshExecuteContext.push_back(subMeshExecuteContext);
			}
		}
		context.RenderItems = itemsExecuteContext.data();

		// execute the opaque render
		mRenderer.Execute(context);
	}

	void EngineCore::LogProfilingData() {
		const EngineRenderer::DX12Renderer::DX12GpuProfilerResults& profilerResults = mRenderer.GetProfilerResults();

		std::string msString = " Ms:";
		for (uint8_t i = 0; i < profilerResults.PassTimes.size(); ++i) {

			float msValue = profilerResults.PassTimes[i];
			if (msValue == 0.f) { continue; }


			const std::string& passName = mRenderer.RendererPipelinePass_ToString(
				EngineRenderer::RendererPipelinePass(i)
			);

			std::string opaqueProfilingMs =
				passName + msString +
				std::to_string(msValue) +
				"\n";
			Engine::DebugSystem::DebugSystem::GetInstance().LogText(opaqueProfilingMs);
		}
	}
}