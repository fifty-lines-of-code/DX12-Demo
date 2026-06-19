#include "DX12OpaqueRenderPipelinePass.h"

#include "../../d3dx12.h"
#include "../../DX12RendererHelper.h"
#include "../../DX12 Data Structures/DX12ResourceDataStructures.h"
#include "../../../../../Helper/Helper.h"

namespace Engine::EngineRenderer::DX12Renderer {

#pragma region Private

	bool DX12OpaqueRenderPipelinePass::OnInitialize(
		const DX12PipelinePassInitArgs& args
	) {
		const DX12OpaqueRenderPipelineInitArgs& renderArgs = static_cast<const DX12OpaqueRenderPipelineInitArgs&>(args);

		if (!CreateConstantBufferDescriptors(renderArgs)) { return false; }
		if (!CreateRootSignature(renderArgs)) { return false; }
		if (!CreateShadersAndInputLayout()) { return false; }
		if (!CreatePipelineStateObject(renderArgs)) { return false; }

		return true;
	}

	void DX12OpaqueRenderPipelinePass::OnShutdown() {
		if (mPsByteCode != nullptr) { mPsByteCode.Reset(); }
		if (mVsByteCode != nullptr) { mVsByteCode.Reset(); }
		mInputLayout.clear();
	}

	void DX12OpaqueRenderPipelinePass::OnExecute(const DX12PipelinePassExecuteArgs& args) {
		const DX12OpaqueRenderPipelineExecuteArgs& dArgs = static_cast<const DX12OpaqueRenderPipelineExecuteArgs&>(args);
		Execute(dArgs);
	}

	void DX12OpaqueRenderPipelinePass::Execute(
		const DX12OpaqueRenderPipelineExecuteArgs& args
	) {
		if (args.NumberOfItems == 0) { return; }

		// Grab the command allocator for the current frame
		ID3D12CommandAllocator* allocator = mCommandAllocators[args.CurrentFrameIndex].Get();
		ThrowIfFailed(allocator->Reset());

		ThrowIfFailed(mCommandList->Reset(allocator, mPipelineStateObject.Get()));
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

		// set the per pass cb, materials cb
		ID3D12DescriptorHeap* descriptorHeaps[] = { mDescriptorHeap.Get() };
		mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		// per pass cb
		mCommandList->SetGraphicsRootConstantBufferView(1, args.PerPassCBResourceAddress);

		// materials cb
		auto materialsCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			mDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
		);
		int globalMaterialHeapOffset = (args.NumberOfMaterials * args.CurrentFrameIndex);
		materialsCbvHandle.Offset(
			globalMaterialHeapOffset, args.CbvSrvUavDescriptorSize
		);
		mCommandList->SetGraphicsRootDescriptorTable(2, materialsCbvHandle);

		// textures
		auto texturesCbHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			mDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
		);
		texturesCbHandle.Offset(mTexturesCbHeapOffset, args.CbvSrvUavDescriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(3, texturesCbHandle);

		// now draw each item
		for (uint32_t i = 0; i < args.NumberOfItems; ++i) {
			const DX12OpaqueRenderPipelinePerItemExecuteArgs itemContext = args.PipelineItemsExecuteArgs[i];

			// set vertex buffer
			mCommandList->IASetVertexBuffers(
				0,
				1,
				&itemContext.VertexBufferView
			);

			// set index buffer
			mCommandList->IASetIndexBuffer(
				&itemContext.IndexBufferView
			);

			// set the primitive topology
			mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Offset to the CBV in the CBV heap for this object and for this frame resource.
			D3D12_GPU_VIRTUAL_ADDRESS currentEntityAddress =
				args.PerRenderItemCBResourceAddress + (itemContext.ID * args.AlignedSizeOfPerElementCb);

			mCommandList->SetGraphicsRootConstantBufferView(0, currentEntityAddress);

			// draw call
			mCommandList->DrawIndexedInstanced(
				itemContext.IndexCount,
				1, 0, 0, 0
			);
		}

		// don't close the command list, the aggregator will close it.
	}

	bool DX12OpaqueRenderPipelinePass::CreateConstantBufferDescriptors(
		const DX12OpaqueRenderPipelineInitArgs& args
	) {
		// Textures sit at the very end after the 3 frames of per material cb
		mTexturesCbHeapOffset = args.NumberOfMaterials * DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES;

		// Compute the global total of descriptors across all frames
		// (1 per pass + N entities + M materials) * Total Frames + No Textures
		UINT numberOfDescriptors = args.NumberOfMaterials * DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES + args.NumberOfTextures;

		// Describe the CBV descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
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

		// Pass the offsets and sizes forward to generate the views
		return CreateConstantBufferViews(args);
	}

	bool DX12OpaqueRenderPipelinePass::CreateConstantBufferViews(
		const DX12OpaqueRenderPipelineInitArgs& args
	) {		
		// per material cbs are laid out first per frame
		// so 
		// PerMatCB0(F0), PerMatCB1(F0), PerMatCB0(F1)..., PerMatCBN-1(FN-1)
		// then the textures are laid out
		// T0, T1..., TN-1

		// our per material cb
		// ((alignedPerMaterialCB) * numberOfMaterials * numberOfFrames
		for (UINT frameIndex = 0; frameIndex < DX12RendererConfig::NUMBER_OF_FRAME_RESOURCES; ++frameIndex)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = args.PerMaterialCBAddress[frameIndex];

			for (uint32_t i = 0; i < args.NumberOfMaterials; ++i) {
				// Offset to this material cbv in the descriptor heap.
				int heapIndex = (args.NumberOfMaterials * frameIndex) + i;
				auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
					mDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
				);
				handle.Offset(heapIndex, args.CbvSrvUavDescriptorSize);

				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
				cbvDesc.BufferLocation = cbAddress + (i * args.AlignedSizeOfPerMaterialCb);
				cbvDesc.SizeInBytes = args.AlignedSizeOfPerMaterialCb;

				args.Device->CreateConstantBufferView(&cbvDesc, handle);
			}
		}

		// then our textures

		// ensure TexturesData is a valid pointer
		if (args.TexturesData == nullptr) { return false; }

		CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(
			mDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
		);
		// offset to the start of textures
		hDescriptor.Offset(mTexturesCbHeapOffset, args.CbvSrvUavDescriptorSize);

		for (uint32_t i = 0; i < args.NumberOfTextures; ++i) {
			// walk to this descriptor in the heap
			CD3DX12_CPU_DESCRIPTOR_HANDLE currentHandle(
				hDescriptor, 
				(INT)i,
				args.CbvSrvUavDescriptorSize
			);

			// todo: find a way to ensure we don't overflow
			const DX12Texture& tex = args.TexturesData[i];
			ID3D12Resource* resource;
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

			// if unloaded 
			if (!tex.IsLoaded || tex.Resource == nullptr) {
				srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Generic pixel format
				srvDesc.Texture2D.MipLevels = 1;
				resource = nullptr;
			}
			else {
				srvDesc.Format = tex.Resource->GetDesc().Format; // Grab format from the DDS file
				srvDesc.Texture2D.MipLevels = tex.Resource->GetDesc().MipLevels;
				resource = tex.Resource.Get();
			}

			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			// Bake the view configuration straight into the descriptor heap slot
			args.Device->CreateShaderResourceView(resource, &srvDesc, currentHandle);
		}

		return true;
	}

	bool DX12OpaqueRenderPipelinePass::CreateRootSignature(
		const DX12OpaqueRenderPipelineInitArgs& args
	) {
		// TODO: Update to Roott Signature 1.1 for dynamic indexing inside 
		// Materials and Textures array

		// we have 4 parameters for this root signature
		CD3DX12_ROOT_PARAMETER slotRootParameter[4];

		// Create a two CBVs inlined into the Root Signature
		// per object cb
		slotRootParameter[0].InitAsConstantBufferView(0);
		// per pass cb
		slotRootParameter[1].InitAsConstantBufferView(1);

		// create two descriptor tables for Materials and Textures
		// per material cb
		CD3DX12_DESCRIPTOR_RANGE cbvTable1;
		cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, args.NumberOfMaterials, 2); //(b2)

		// textures buffer
		CD3DX12_DESCRIPTOR_RANGE cbvTable2;
		cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, args.NumberOfTextures, 0);
		slotRootParameter[2].InitAsDescriptorTable(1, &cbvTable1);
		slotRootParameter[3].InitAsDescriptorTable(
			1, 
			&cbvTable2,
			D3D12_SHADER_VISIBILITY_PIXEL
		);

		// static samplers
		std::array<CD3DX12_STATIC_SAMPLER_DESC, DX12RendererHelper::DX12_MAX_SAMPLERS> samplers;
		DX12RendererHelper::GetStaticSamplers(samplers);

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
		rootSigDesc.Init(
			_countof(slotRootParameter),
			slotRootParameter,
			(UINT)samplers.size(),
			samplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr) {
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

	bool DX12OpaqueRenderPipelinePass::CreateShadersAndInputLayout() {
		mVsByteCode = DX12RendererHelper::CompileShader(L"Source\\Resources\\Shaders\\color.hlsl", nullptr, "VS", "vs_5_1");
		mPsByteCode = DX12RendererHelper::CompileShader(L"Source\\Resources\\Shaders\\color.hlsl", nullptr, "PS", "ps_5_1");

		mInputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
			  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		return true;
	}

	bool DX12OpaqueRenderPipelinePass::CreatePipelineStateObject(
		const DX12OpaqueRenderPipelineInitArgs& args
	) {
		// describe the pso
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
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
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
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

		return true;
	}

#pragma endregion
}