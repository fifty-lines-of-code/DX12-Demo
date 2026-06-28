#include "DX12DebugSystemPipelinePass.h"

#include "../../d3dx12.h"
#include "../../DX12RendererHelper.h"
#include "../../DX12 Data Structures/DX12ResourceDataStructures.h"
#include "../../../../../Helper/Helper.h"

namespace Engine::EngineRenderer::DX12Renderer {

#pragma region Private

	bool DX12DebugSystemPipelinePass::OnInitialize(
		const DX12PipelinePassInitArgs& args
	) {
		const DX12DebugSystemPipelinePassInitArgs& debugArgs = static_cast<const DX12DebugSystemPipelinePassInitArgs&>(args);

		if (!CreateConstantBufferDescriptors(debugArgs)) { return false; }
		if (!CreateRootSignature(debugArgs)) { return false; }
		if (!CreateShaders()) { return false; }
		if (!CreatePipelineStateObject(debugArgs)) { return false; }

		return true;
	}

	void DX12DebugSystemPipelinePass::OnShutdown() {
		if (mPsByteCode != nullptr) { mPsByteCode.Reset(); }
		if (mVsByteCode != nullptr) { mVsByteCode.Reset(); }
		if (mDescriptorHeap != nullptr) { mDescriptorHeap.Reset(); }
	}

	void DX12DebugSystemPipelinePass::OnExecute(const DX12PipelinePassExecuteArgs& args) {
		const DX12DebugSystemPipelinePassExecuteArgs& dArgs = static_cast<const DX12DebugSystemPipelinePassExecuteArgs&>(args);
		Execute(dArgs);
	}

	void DX12DebugSystemPipelinePass::Execute(
		const DX12DebugSystemPipelinePassExecuteArgs& args
	) {
		// Grab the command allocator for the current frame
		ID3D12CommandAllocator* allocator = mCommandAllocators[args.CurrentFrameIndex].Get();
		ThrowIfFailed(allocator->Reset());

		ThrowIfFailed(mCommandList->Reset(
			allocator, 
			mPipelineStateObject.Get()
		));

		// tell the profiler to start profiling
		args.GpuProfiler.BeginPass(
			mCommandList.Get(),
			(uint8_t)RendererPipelinePass::DEBUG_SYSTEM_PASS
		);

		mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

		// set view port and scissor rect
		mCommandList->RSSetViewports(1, &args.Viewport);
		mCommandList->RSSetScissorRects(1, &args.ScissorRect);

		// set the render target
		mCommandList->OMSetRenderTargets(
			1,
			&args.BackBufferView,
			true,
			&args.DepthStencilView
		);

		// 1. Bind ONLY the debug heap—holding both the structured buffer and the font copy!
		ID3D12DescriptorHeap* activeHeaps[] = { mDescriptorHeap.Get() };
		mCommandList->SetDescriptorHeaps(1, activeHeaps);

		// 2. Parameter 0: Direct Virtual Address for Constant Buffer
		auto perPassResource = args.Resource;
		mCommandList->SetGraphicsRootConstantBufferView(
			0,
			perPassResource->GetGPUVirtualAddress()
		);

		// 3. Parameter 1: Structured Buffer table
		CD3DX12_GPU_DESCRIPTOR_HANDLE sbHandle(
			mDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
		);
		sbHandle.Offset(args.CurrentFrameIndex, args.CbvSrvUavDescriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(1, sbHandle);

		// 4. Parameter 2: Font Texture table
		CD3DX12_GPU_DESCRIPTOR_HANDLE fontHandle(
			mDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
		);
		fontHandle.Offset(
			DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES,
			args.CbvSrvUavDescriptorSize
		);
		mCommandList->SetGraphicsRootDescriptorTable(2, fontHandle);

		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// 5. Fire off the procedural magic
		mCommandList->DrawInstanced(4, args.NumberOfCharacters, 0, 0);

		// tell the profiler to end profiling
		args.GpuProfiler.EndPass(
			mCommandList.Get(),
			(uint8_t)RendererPipelinePass::DEBUG_SYSTEM_PASS
		);

		// don't close the command list, the aggregator will close it.
	}

	bool DX12DebugSystemPipelinePass::CreateConstantBufferDescriptors(
		const DX12DebugSystemPipelinePassInitArgs& args
	) {
		uint32_t alignedSizeOfPerPassCb = DX12RendererHelper::CalculateAlignedConstantBufferByteSize(sizeof(DX12DebugSystemPerPassConstants));

		// Compute the global total of descriptors across all frames
		// (1 for SRV) * NoFrames + 1 for Font Atlaas
		UINT numberOfDescriptors = (1 * DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES) + 1;

		// Describe the CBV descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = numberOfDescriptors;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		// Create the CBV descriptor heap
		ThrowIfFailed(
			args.Device->CreateDescriptorHeap(
				&cbvHeapDesc,
				IID_PPV_ARGS(&mDescriptorHeap)
			)
		);

		return CreateConstantBufferViews(args);
	}

	bool DX12DebugSystemPipelinePass::CreateConstantBufferViews(
		const DX12DebugSystemPipelinePassInitArgs& args
	) {
		// create our srv per frame
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

		srvDesc.Buffer.FirstElement = 0;
		// The structural upper boundary limit of your glyph storage pool
		srvDesc.Buffer.NumElements = args.DebugSystemMaxCharacters;
		// Tell the hardware the exact stride width of a single character vertex element
		srvDesc.Buffer.StructureByteStride = sizeof(DX12DebugSystemPerCharacterData);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		// Loop through each frame resource and bake the view into its assigned slot
		for (UINT frameIndex = 0; frameIndex < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++frameIndex)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(
				mDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
			);
			cpuHandle.Offset(
				frameIndex, 
				args.CbvSrvUavDescriptorSize
			);

			// Grab the raw GPU resource pointer from the active frame tracking element
			auto perCharacterCbResource = args.CBResources[frameIndex];

			// Instantiate the SRV hardware descriptor directly into the heap slot
			args.Device->CreateShaderResourceView(
				perCharacterCbResource,
				&srvDesc,
				cpuHandle
			);
		}

		// create Font Atlas View

		// 1. Point to the new 4th slot at the very end of the debug heap
		CD3DX12_CPU_DESCRIPTOR_HANDLE heapFontCpuHandle(
			mDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
		);
		heapFontCpuHandle.Offset(
			DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES, 
			args.CbvSrvUavDescriptorSize
		);

		// 2. Set up Font texture view description
		// todo: pass in the Font Atlas index
		ID3D12Resource* resource;
		D3D12_SHADER_RESOURCE_VIEW_DESC fontSrvDesc = {};

		// if unloaded 
		if (!args.FontAtlasTexture.IsLoaded || args.FontAtlasTexture.Resource == nullptr) {
			fontSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Generic pixel format
			fontSrvDesc.Texture2D.MipLevels = 1;
			resource = nullptr;
		}
		else {
			// Grab format from the DDS file
			fontSrvDesc.Format = args.FontAtlasTexture.Resource->GetDesc().Format; 
			fontSrvDesc.Texture2D.MipLevels = args.FontAtlasTexture.Resource->GetDesc().MipLevels;
			resource = args.FontAtlasTexture.Resource.Get();
		}

		// Create the Shader Resource View
		fontSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		fontSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		fontSrvDesc.Texture2D.MostDetailedMip = 0;
		fontSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		args.Device->CreateShaderResourceView(
			resource, 
			&fontSrvDesc, 
			heapFontCpuHandle
		);

		return true;
	}

	bool DX12DebugSystemPipelinePass::CreateRootSignature(
		const DX12DebugSystemPipelinePassInitArgs& args
	) {
		CD3DX12_DESCRIPTOR_RANGE slotRootRanges[2];

		// Range 0: The Structured Buffer SRV -> register(t0)
		slotRootRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		// Range 1: The Font Atlas Texture SRV -> register(t1)
		slotRootRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		CD3DX12_ROOT_PARAMETER rootParameters[3];

		// Parameter 0: Direct Root CBV -> register(b0)
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

		// Parameter 1: Descriptor Table pointing to your Structured Buffer range
		rootParameters[1].InitAsDescriptorTable(1, &slotRootRanges[0], D3D12_SHADER_VISIBILITY_VERTEX);

		// Parameter 2: Descriptor Table pointing to your Font Texture range
		rootParameters[2].InitAsDescriptorTable(1, &slotRootRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);

		// Static Sampler for smooth texel mapping interpolation
		std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> samplers;
		DX12RendererHelper::GetStaticSamplers(samplers);

		// index 3 is the linear clamp we want
		CD3DX12_STATIC_SAMPLER_DESC fontSampler = samplers[3];
		// map to register(s0) since by default it's mapped to register 3
		fontSampler.ShaderRegister = 0;

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
			_countof(rootParameters),
			rootParameters,
			1,
			&fontSampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr)
		{
			// todo: Move to Logger
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(
			args.Device->CreateRootSignature(
				0,
				serializedRootSig->GetBufferPointer(),
				serializedRootSig->GetBufferSize(),
				IID_PPV_ARGS(&mRootSignature)
			)
		);

		return true;
	}

	bool DX12DebugSystemPipelinePass::CreateShaders() {
		mVsByteCode = DX12RendererHelper::CompileShader(
			L"Source\\Resources\\Shaders\\Debug\\debug_vs.hlsl",
			nullptr,
			"VS_Main",
			"vs_5_1"
		);

		mPsByteCode = DX12RendererHelper::CompileShader(
			L"Source\\Resources\\Shaders\\Debug\\debug_ps.hlsl",
			nullptr,
			"PS_Main",
			"ps_5_1"
		);

		return true;
	}

	bool DX12DebugSystemPipelinePass::CreatePipelineStateObject(
		const DX12DebugSystemPipelinePassInitArgs& args
	) {
		// describe the debug pso
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mVsByteCode->GetBufferPointer()),
			mVsByteCode->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mPsByteCode->GetBufferPointer()),
			mPsByteCode->GetBufferSize()
		};

		// disable culling
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

		// enable blending
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// don't write depth values
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		psoDesc.DepthStencilState.DepthEnable = FALSE;

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = args.BackBufferFormat;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.DSVFormat = args.DepthStencilFormat;

		// build the pso
		ThrowIfFailed(
			args.Device->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(&mPipelineStateObject)
			)
		);
		mPipelineStateObject->SetName(L"Debug System Pipeline State Object");

		return true;
	}

#pragma endregion
}