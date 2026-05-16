#pragma once

#include <cstdint>
#include <windows.h>

class IRenderer {
public:
	virtual ~IRenderer() {}

	virtual bool Initialize(HWND mainHwnd, int numberOfFrameResources) = 0;
	virtual void Shutdown() = 0;
	virtual void PrepareForUpdate() = 0;
	virtual void BeginFrame() = 0;
	virtual bool Draw(uint32_t meshID, uint32_t indexCount, uint32_t entityIndex, uint32_t entityCount) = 0;
	virtual void EndFrame() = 0;
	virtual void OnResize(UINT newClientWidth, UINT newClientHeight) = 0;
	virtual void FinishInitialize() = 0;
};