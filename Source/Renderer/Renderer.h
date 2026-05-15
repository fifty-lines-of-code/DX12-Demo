#pragma once

#include <cstdint>
#include <windows.h>

class Renderer {
public:
	virtual ~Renderer() {}

	virtual bool Initialize(HWND mainHwnd) = 0;
	virtual void Shutdown() = 0;
	virtual void Update(uint32_t meshID, void* data, size_t dataSize) = 0;
	virtual void BeginFrame() = 0;
	virtual bool Draw(uint32_t meshID, uint32_t indexCount)  = 0;
	virtual void EndFrame() = 0;
	virtual void OnResize(UINT newClientWidth, UINT newClientHeight) = 0;
};