#include "DX12MirrorRenderPipelinePass.h"

#include "../../d3dx12.h"
#include <DirectXColors.h>
#include "../../DX12RendererHelper.h"
#include "../../DX12 Data Structures/DX12ResourceDataStructures.h"
#include "../../../../../Helper/Helper.h"

namespace Engine::EngineRenderer::DX12Renderer {

#pragma region Private

	bool DX12MirrorRenderPipelinePass::OnInitialize(
		const DX12PipelinePassInitArgs& args
	) {
		const DX12MirrorRenderPipelineInitArgs& renderArgs = static_cast<const DX12MirrorRenderPipelineInitArgs&>(args);

		if (!CreateRootSignature(renderArgs)) { return false; }
		if (!CreateShadersAndInputLayout()) { return false; }
		if (!CreatePipelineStateObject(renderArgs)) { return false; }

		return true;
	}

	void DX12MirrorRenderPipelinePass::OnShutdown() {
		if (mPsByteCode != nullptr) { mPsByteCode.Reset(); }
		if (mVsByteCode != nullptr) { mVsByteCode.Reset(); }
		mInputLayout.clear();
	}

	void DX12MirrorRenderPipelinePass::OnExecute(const DX12PipelinePassExecuteArgs& args) {
		const DX12MirrorRenderPipelineExecuteArgs& dArgs = static_cast<const DX12MirrorRenderPipelineExecuteArgs&>(args);
		Execute(dArgs);
	}

	void DX12MirrorRenderPipelinePass::Execute(
		const DX12MirrorRenderPipelineExecuteArgs& args
	) {
		if (args.NumberOfItems == 0) { return; }

		// Grab the command allocator for the current frame
		ID3D12CommandAllocator* allocator = mCommandAllocators[args.CurrentFrameIndex].Get();
		ThrowIfFailed(allocator->Reset());

		ThrowIfFailed(mCommandList->Reset(allocator, mPipelineStateObject.Get()));

		// tell the profiler to start profiling
		args.GpuProfiler.BeginPass(
			mCommandList.Get(),
			(uint8_t)RendererPipelinePass::MIRROR_RENDER_PASS
		);

		// set the root signature
		mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

		// TODO:
		// transition the back buffer view to render target from pixel source

		// set view port and scissor rect
		mCommandList->RSSetViewports(1, &args.Viewport);
		mCommandList->RSSetScissorRects(1, &args.ScissorRect);

		// transition this frame mirror resource buffer to render target
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			args.CurrentBackBufferResource,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);

		// Indicate the transition via a command.
		mCommandList->ResourceBarrier(
			1,
			&barrier
		);

		// Clear the render target buffer and associated depth/stencil buffer
		mCommandList->ClearRenderTargetView(
			args.BackBufferView,
			DirectX::Colors::LightSteelBlue,
			0,
			nullptr
		);

		mCommandList->ClearDepthStencilView(
			args.DepthStencilView,
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.0f,
			0,
			0,
			nullptr
		);

		// Specify the buffers we are going to render to.
		mCommandList->OMSetRenderTargets(
			1,
			&args.BackBufferView,
			true,
			&args.DepthStencilView
		);

		// set the descriptor heap
		ID3D12DescriptorHeap* descriptorHeaps[] = { args.DescriptorHeap };
		mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		// per pass cb
		mCommandList->SetGraphicsRootConstantBufferView(
			mObjectsPerPassCBIndex,
			args.PerPassCBResourceAddress
		);

		// materials cb
		mCommandList->SetGraphicsRootDescriptorTable(
			mMaterialsCBIndex,
			args.MaterialsCbvHandle
		);

		// textures
		mCommandList->SetGraphicsRootDescriptorTable(
			mTexturesCBIndex,
			args.TexturesCbvHandle
		);

		// now draw each item
		for (uint32_t i = 0; i < args.NumberOfItems; ++i) {
			const DX12MirrorRenderPipelinePerItemExecuteArgs& itemContext = args.PipelineItemsExecuteArgs[i];

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
			uint32_t entityOffset = itemContext.ID * args.AlignedSizeOfPerRenderItemCb;
			D3D12_GPU_VIRTUAL_ADDRESS currentEntityAddress =
				args.PerRenderItemCBResourceAddress +
				entityOffset;

			mCommandList->SetGraphicsRootConstantBufferView(
				mPerObjectCBIndex,
				currentEntityAddress
			);

			uint32_t entitySubMeshesOffset = itemContext.ID *
				args.NumberOfSubMeshesPerItem *
				args.AlignedSizeOfPerRenderItemSubMeshCb;

			// calculate submesh CB address
			D3D12_GPU_VIRTUAL_ADDRESS subMeshBaseAddress =
				args.PerRenderItemSubMeshCBResourceAddress +
				entitySubMeshesOffset;

			// now draw per sub mesh
			for (uint8_t j = 0; j < itemContext.SubMeshCount; ++j) {
				const DX12MirrorRenderPipelinePerItemPerSubMeshExecuteArgs& subMeshContext =
					itemContext.SubMeshExecuteArgs[j];

				uint32_t subMeshOffset = j * args.AlignedSizeOfPerRenderItemSubMeshCb;

				// get this submeshes address
				D3D12_GPU_VIRTUAL_ADDRESS subMeshAddress =
					subMeshBaseAddress +
					subMeshOffset;

				// bind it
				mCommandList->SetGraphicsRootConstantBufferView(
					mPerObjectPerSubMeshCBIndex,
					subMeshAddress
				);

				// draw call
				mCommandList->DrawIndexedInstanced(
					subMeshContext.IndexCount,
					1, // no of instances
					subMeshContext.StartIndexLocation,
					subMeshContext.BaseVertexLocation,
					0 // start instance location
				);
			}
		}

		// transition the back buffer to pixel source
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			args.CurrentBackBufferResource,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);

		// Indicate the transition via a command.
		mCommandList->ResourceBarrier(
			1,
			&barrier
		);

		// tell the profiler to end profiling
		args.GpuProfiler.EndPass(
			mCommandList.Get(),
			(uint8_t)RendererPipelinePass::MIRROR_RENDER_PASS
		);

		// don't close the command list, the aggregator will close it.
	}

	bool DX12MirrorRenderPipelinePass::CreateRootSignature(
		const DX12MirrorRenderPipelineInitArgs& args
	) {
		// TODO: Update to Root Signature 1.1 for dynamic indexing inside 
		// Materials and Textures array

		// we have 5 parameters for this root signature
		CD3DX12_ROOT_PARAMETER slotRootParameter[5];

		// Create a two CBVs inlined into the Root Signature
		// per object cb
		slotRootParameter[mPerObjectCBIndex].InitAsConstantBufferView(0);
		// per submesh cb
		slotRootParameter[mPerObjectPerSubMeshCBIndex].InitAsConstantBufferView(1);
		// per pass cb
		slotRootParameter[mObjectsPerPassCBIndex].InitAsConstantBufferView(2);

		// create two descriptor tables for Materials and Textures
		// per material cb
		CD3DX12_DESCRIPTOR_RANGE cbvTable1;
		//(b3)
		cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, args.NumberOfMaterials, 3);
		slotRootParameter[mMaterialsCBIndex].InitAsDescriptorTable(1, &cbvTable1);

		// textures buffer
		CD3DX12_DESCRIPTOR_RANGE cbvTable2;
		// (t0)
		cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 256, 0);
		slotRootParameter[mTexturesCBIndex].InitAsDescriptorTable(
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

	bool DX12MirrorRenderPipelinePass::CreateShadersAndInputLayout() {
		mVsByteCode = DX12RendererHelper::CompileShader(L"Source\\Resources\\Shaders\\opaque_vs_ps.hlsl", nullptr, "VS", "vs_5_1");
		mPsByteCode = DX12RendererHelper::CompileShader(L"Source\\Resources\\Shaders\\opaque_vs_ps.hlsl", nullptr, "PS", "ps_5_1");

		mInputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
			  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		return true;
	}

	bool DX12MirrorRenderPipelinePass::CreatePipelineStateObject(
		const DX12MirrorRenderPipelineInitArgs& args
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

		// For mirror pass, we want front CCW to be True
		// as the triangle winding is inverted 
		psoDesc.RasterizerState.FrontCounterClockwise = true;

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