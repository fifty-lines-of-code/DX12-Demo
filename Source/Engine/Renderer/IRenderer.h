#pragma once

#include <cstdint>
#include "../EngineConfig.h"
#include <string>
#include <windows.h>


namespace Engine::EngineRenderer {

	enum class RendererPipelinePass : uint8_t {
		SHADOW_PASS,
		OPAQUE_RENDER_PASS,
		DEBUG_SYSTEM_PASS,
		BLUR_UI_PASS,
		COUNT,
		INVALID
	};

	struct IPipelinePassExecuteContext {
		virtual ~IPipelinePassExecuteContext() = default;
		virtual RendererPipelinePass GetPipelinePassType() const = 0;
	};

	class IRenderer {

	public:
		IRenderer() = default;
		virtual ~IRenderer() {}

		// delete the copy and assignment operators
		IRenderer(const IRenderer&) = delete;
		IRenderer& operator=(const IRenderer&) = delete;

		virtual bool Initialize(
			HWND mainHwnd, 
			int numberOfFrameResources,
			UINT fullscreenWidth, 
			UINT fullscreenHeight
		) = 0;
		virtual void FinishInitialize() = 0;
		virtual void Shutdown() = 0;
		virtual bool LoadTexture(std::wstring& filename, uint32_t id) = 0;
		virtual void LoadGeometry(
			uint32_t meshID, 
			uint16_t sizeOfVertex, 
			uint32_t vertexBufferByteSize, 
			const void* vertices, 
			uint32_t indexBufferByteSize, 
			const void* indices
		) = 0;
		virtual void PrepareForUpdate() = 0;
		virtual void UpdateOpaqueRenderItemsPerPassCb(
			const void* data, 
			size_t dataSize
		) const = 0;
		virtual void UpdateOpaqueRenderItemCb(
			uint32_t renderItemIndex,
			const void* data, 
			uint32_t perRenderItemCbSize
		) = 0;
		virtual void UpdateOpaqueRenderItemSubMeshCb(
			uint32_t renderItemIndex,
			uint32_t maxNumberSubMeshes,
			const void* data,
			uint32_t renderItemPerSubMeshCbSize
		) = 0;
		virtual void UpdatePerMaterialCb(
			uint32_t materialIndex,
			const void* data, 
			uint32_t perMaterialCbSize
		) = 0;
		virtual void UpdateDebugSystemPerPassCb(const void* data) = 0;
		virtual void BeginFrame(uint32_t numberOfMaterials) = 0;
		virtual void Execute(const IPipelinePassExecuteContext& context) = 0;
		virtual bool DrawDebugSystem(uint32_t numberOfCharacters) = 0;
		virtual void EndFrame() = 0;
		virtual void OnResize(UINT width, UINT height) = 0;
		virtual float QueryPipelinePassPerformance(
			RendererPipelinePass pipelinPass
		) = 0;
	};
}