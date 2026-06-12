#pragma once

#include <d3d12.h>
#include <string>
#include <wrl.h>

struct DX12Texture {
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
	uint32_t Id = -1; // wraps around to uint32_t max
	bool IsLoaded = false;

	void DisposeUploader() { UploadHeap.Reset(); }
};