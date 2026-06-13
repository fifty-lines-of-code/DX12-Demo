#pragma once

#include <cstdint>
#include "../EngineConfig.h"
#include <string>
#include <windows.h>

class IRenderer {
public:
	virtual ~IRenderer() {}

	virtual bool Initialize(HWND mainHwnd, int numberOfFrameResources, UINT fullscreenWidth, UINT fullscreenHeight) = 0;
	virtual void FinishInitialize() = 0;
	virtual bool LoadTexture(std::wstring& filename, uint32_t id) = 0;
	virtual void LoadGeometry(uint32_t meshID, uint16_t sizeOfVertex, uint32_t vertexBufferByteSize, void* vertices, uint32_t indexBufferByteSize, void* indices) = 0;
	virtual void Shutdown() = 0;
	virtual void PrepareForUpdate() = 0;
	virtual void UpdatePerPassCb(void* data, size_t dataSize) const = 0;
	virtual void UpdatePerRenderItemCb(uint32_t renderItemIndex, void* data, uint32_t perRenderItemCbSize) = 0;
	virtual void UpdatePerMaterialCb(uint32_t materialIndex, void* data, uint32_t perMaterialCbSize) = 0;
	virtual void UpdateDebugSystemPerPassCb(void* data) = 0;
	virtual void BeginFrame(uint32_t numberOfMaterials) = 0;
	virtual bool Draw(uint32_t meshID, uint32_t indexCount, uint32_t entityIndex, uint32_t entityCount) = 0;
	virtual bool DrawDebugSystem(uint32_t numberOfCharacters) = 0;
	virtual void EndFrame() = 0;
	virtual void OnResize(UINT width, UINT height) = 0;
};