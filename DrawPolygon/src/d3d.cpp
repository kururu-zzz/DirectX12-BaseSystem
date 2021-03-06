#pragma comment( lib, "d3d12.lib" )

#include "d3d.h"
#include "dxgi.h"
#include <d3dcompiler.h>
#include <PlatformHelpers.h>

namespace d3d
{
	void ReleaseIUnknown(IUnknown* p)
	{
		p->Release();
	}
	std::shared_ptr<ID3D12Device> CreateDevice()
	{
#ifdef _DEBUG
		// Enable the D3D12 debug layer.
		// cf.MicroSoft Sample Source
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
		}
#endif
		ID3D12Device* device;

		auto hr = D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&device)
			);

		if (!SUCCEEDED(hr))
		{
			auto factory = dxgi::CreateFactory();
			auto adapter = dxgi::CreateAdapter(factory.get());
			DirectX::ThrowIfFailed(
				D3D12CreateDevice(
					adapter.get(),
					D3D_FEATURE_LEVEL_12_0,
					IID_PPV_ARGS(&device)
					));
		}
		return std::shared_ptr<ID3D12Device>(device, ReleaseIUnknown);
	}
	std::shared_ptr<ID3D12CommandQueue> CreateCommandQueue(ID3D12Device* device, const D3D12_COMMAND_QUEUE_DESC* queueDesc)
	{
		ID3D12CommandQueue* commandQueue;
		D3D12_COMMAND_QUEUE_DESC defaultDesc = {};
		if (!queueDesc)
		{
			defaultDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			defaultDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			queueDesc = &defaultDesc;
		}
		DirectX::ThrowIfFailed(
			device->CreateCommandQueue(
				queueDesc,
				IID_PPV_ARGS(&commandQueue)
				));
		return std::shared_ptr<ID3D12CommandQueue>(commandQueue, ReleaseIUnknown);
	}

	std::shared_ptr<ID3D12CommandAllocator> CreateCommandAllocator(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE listType) 
	{
		ID3D12CommandAllocator* commandAllocator;
		DirectX::ThrowIfFailed(
			device->CreateCommandAllocator(
				listType,
				IID_PPV_ARGS(&commandAllocator)
				));
		return std::shared_ptr<ID3D12CommandAllocator>(commandAllocator, ReleaseIUnknown);
	}


	std::shared_ptr<ID3DBlob> CreateBlob(const std::string& fileName, const std::string& shaderName, const std::string& shaderType)
	{
		ID3DBlob* blob;
		WCHAR	path[100];

		MultiByteToWideChar(CP_ACP, 0, fileName.c_str(), -1, path, MAX_PATH);

#ifdef _DEBUG
		// Enable better shader debugging with the graphics debugging tools.
		// cf.MicroSoft Sample Source
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		DirectX::ThrowIfFailed(
			D3DCompileFromFile(
				path, nullptr, nullptr, shaderName.c_str(), shaderType.c_str(), compileFlags, 0, &blob, nullptr
				));
		return std::shared_ptr<ID3DBlob>(blob, ReleaseIUnknown);
	}

	std::shared_ptr<ID3D12RootSignature> CreateRootSignature(ID3D12Device* device,D3D12_ROOT_SIGNATURE_DESC* rootDesc)
	{
		ID3D12RootSignature* rootSignature;
		ID3DBlob* pOutBlob;
		ID3DBlob* pErrorBlob;
		D3D12_ROOT_SIGNATURE_DESC defaultDesc = {};
		if (!rootDesc)
		{
			defaultDesc.pParameters = nullptr;
			defaultDesc.pStaticSamplers = nullptr;
			defaultDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			rootDesc = &defaultDesc;
		}
		DirectX::ThrowIfFailed(
			D3D12SerializeRootSignature(
				rootDesc,
				D3D_ROOT_SIGNATURE_VERSION_1,
				&pOutBlob,
				&pErrorBlob
				));
		DirectX::ThrowIfFailed(
			device->CreateRootSignature(
				0,
				pOutBlob->GetBufferPointer(),
				pOutBlob->GetBufferSize(),
				IID_PPV_ARGS(&rootSignature)));
		pOutBlob->Release();
		pOutBlob = nullptr;
		return std::shared_ptr<ID3D12RootSignature>(rootSignature,ReleaseIUnknown);
	}

	D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode)
	{
		D3D12_BLEND_DESC blendDesc = {};

		D3D12_RENDER_TARGET_BLEND_DESC renderTarget = {};

		if (mode == BlendMode::default)
		{
			renderTarget.BlendEnable = FALSE;
			renderTarget.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			renderTarget.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			renderTarget.BlendOp = D3D12_BLEND_OP_ADD;
			renderTarget.SrcBlendAlpha = D3D12_BLEND_ZERO;
			renderTarget.DestBlendAlpha = D3D12_BLEND_ZERO;
			renderTarget.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			renderTarget.LogicOp = D3D12_LOGIC_OP_NOOP;
			renderTarget.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}
		else if (mode == BlendMode::add)
		{
			renderTarget.BlendEnable = FALSE;
			renderTarget.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			renderTarget.DestBlend = D3D12_BLEND_ONE;
			renderTarget.BlendOp = D3D12_BLEND_OP_ADD;
			renderTarget.SrcBlendAlpha = D3D12_BLEND_ZERO;
			renderTarget.DestBlendAlpha = D3D12_BLEND_ZERO;
			renderTarget.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			renderTarget.LogicOp = D3D12_LOGIC_OP_NOOP;
			renderTarget.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}
		::ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.AlphaToCoverageEnable = FALSE;

		blendDesc.IndependentBlendEnable = FALSE;

		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			blendDesc.RenderTarget[i] = renderTarget;

		return blendDesc;
	}

	D3D12_RASTERIZER_DESC CreateRasterizerDesc()
	{
		D3D12_RASTERIZER_DESC rasterizerDesc = {};
		
		rasterizerDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
		rasterizerDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
		rasterizerDesc.DepthClipEnable = TRUE;

		return rasterizerDesc;
	}

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout(const std::vector<std::pair<std::string, DXGI_FORMAT>>& semantics)
	{
		static std::vector<D3D12_INPUT_ELEMENT_DESC> layoutElem;
		for (auto& semantic : semantics)
		{
			D3D12_INPUT_ELEMENT_DESC element = { semantic.first.c_str(), 0, semantic.second, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			layoutElem.emplace_back(element);
		};
		D3D12_INPUT_LAYOUT_DESC inputLayout = { layoutElem.data(),layoutElem.size() };
		return inputLayout;
	}

	std::shared_ptr<ID3D12PipelineState> CreatePipelineState(
		ID3D12Device * device, 
		const D3D12_INPUT_LAYOUT_DESC& layout,
		ID3D12RootSignature* rootSignature,
		ID3DBlob* vertexBlob,
		ID3DBlob* geometryBlob,
		ID3DBlob* pixelBlob,
		ID3DBlob* hullBlob,
		ID3DBlob* domainBlob,
		const D3D12_RASTERIZER_DESC& rasterizeDesc,
		const D3D12_BLEND_DESC& blend)
	{
		ID3D12PipelineState* pipelineState;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
		gpsDesc.InputLayout = layout;
		gpsDesc.pRootSignature = rootSignature;
		if (vertexBlob != nullptr)
			gpsDesc.VS = { reinterpret_cast<UINT8*>(vertexBlob->GetBufferPointer()), vertexBlob->GetBufferSize() };
		if (geometryBlob != nullptr)
			gpsDesc.GS = { reinterpret_cast<UINT8*>(geometryBlob->GetBufferPointer()), geometryBlob->GetBufferSize() };
		if (pixelBlob != nullptr)
			gpsDesc.PS = { reinterpret_cast<UINT8*>(pixelBlob->GetBufferPointer()), pixelBlob->GetBufferSize() };
		if(hullBlob != nullptr)
			gpsDesc.HS = { reinterpret_cast<UINT8*>(hullBlob->GetBufferPointer()), hullBlob->GetBufferSize() };
		if (domainBlob != nullptr)
			gpsDesc.DS = { reinterpret_cast<UINT8*>(domainBlob->GetBufferPointer()), domainBlob->GetBufferSize() };
		gpsDesc.RasterizerState = rasterizeDesc;
		gpsDesc.BlendState = blend;
		gpsDesc.DepthStencilState.DepthEnable = FALSE;
		gpsDesc.DepthStencilState.StencilEnable = FALSE;
		gpsDesc.SampleMask = UINT_MAX;
		gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		gpsDesc.NumRenderTargets = 1;
		gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		gpsDesc.SampleDesc.Count = 1;

		DirectX::ThrowIfFailed(
			device->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&pipelineState)));
		return std::shared_ptr<ID3D12PipelineState>(pipelineState,ReleaseIUnknown);
	}

	std::shared_ptr<ID3D12DescriptorHeap> CreateRTVDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC* descriptHeapDesc)
	{
		ID3D12DescriptorHeap* descriptorHeap;
		D3D12_DESCRIPTOR_HEAP_DESC defaultDesc = {};
		if (!descriptHeapDesc)
		{
			defaultDesc.NumDescriptors = 2;
			defaultDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			defaultDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			descriptHeapDesc = &defaultDesc;
		}
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		DirectX::ThrowIfFailed(
			device->CreateDescriptorHeap(
				descriptHeapDesc, 
				IID_PPV_ARGS(&descriptorHeap)
				));
		return std::shared_ptr<ID3D12DescriptorHeap>(descriptorHeap, ReleaseIUnknown);
	}

	std::shared_ptr<ID3D12GraphicsCommandList> CreateCommandList(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator,ID3D12PipelineState* pipeLineState, D3D12_COMMAND_LIST_TYPE listType)
	{
		ID3D12GraphicsCommandList* commandList;
		DirectX::ThrowIfFailed(
			device->CreateCommandList(
				0,
				listType,
				commandAllocator,
				pipeLineState,
				IID_PPV_ARGS(&commandList)
				));
		commandList->Close();
		return std::shared_ptr<ID3D12GraphicsCommandList>(commandList, ReleaseIUnknown);
	}

	std::vector<std::shared_ptr<ID3D12Resource>> CreateRenderTargets(ID3D12Device * device,IDXGISwapChain * swapChain, ID3D12DescriptorHeap* rtvDescriptorHeap, UINT renderTargetNum)
	{
		std::vector<std::shared_ptr<ID3D12Resource>> renderTargets;
		renderTargets.reserve(renderTargetNum);
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;
		for (UINT i = 0; i < renderTargetNum; ++i)
		{
			ID3D12Resource* renderTarget;
			DirectX::ThrowIfFailed(
				swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTarget)));
			device->CreateRenderTargetView(renderTarget, nullptr, handle);

			renderTargets.emplace_back(std::shared_ptr<ID3D12Resource>(renderTarget, ReleaseIUnknown));
			handle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}
		return renderTargets;
	}

	std::shared_ptr<ID3D12Resource> CreateResource(ID3D12Device* device, size_t size)
	{
		ID3D12Resource* resource;
		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc = { D3D12_RESOURCE_DIMENSION_BUFFER, 0, size, 1, 1, 1,
			DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };


		DirectX::ThrowIfFailed(
			device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&resource)
				));
		return std::shared_ptr<ID3D12Resource>(resource, ReleaseIUnknown);
	}
	D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView(ID3D12Resource * resource,void* data, size_t vertexSize, UINT vertexNum)
	{
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

		UINT8* dataBegin;
		resource->Map(0, nullptr, reinterpret_cast<void**>(&dataBegin));
		memcpy(dataBegin, data, vertexSize*vertexNum);
		resource->Unmap(0, nullptr);

		vertexBufferView.BufferLocation = resource->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = vertexSize;
		vertexBufferView.SizeInBytes = vertexSize*vertexNum;

		return vertexBufferView;
	}

	std::shared_ptr<ID3D12Fence> CreateFence(ID3D12Device * device, D3D12_FENCE_FLAGS flag)
	{
		ID3D12Fence* fence;
		DirectX::ThrowIfFailed(
			device->CreateFence(
				0, 
				D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(&fence)));
		return std::shared_ptr<ID3D12Fence>(fence,ReleaseIUnknown);
	}

	D3D12_VIEWPORT CreateViewport(float width, float height)
	{
		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = viewport.TopLeftY = 0.f;
		viewport.Width = width;
		viewport.Height = height;
		viewport.MaxDepth = 1.0f;
		viewport.MinDepth = 0.0f;
		return viewport;
	}

	D3D12_RECT CreateRect(LONG width,LONG height)
	{
		D3D12_RECT rect;
		rect.left = rect.top = 0;
		rect.right = width;
		rect.bottom = height;
		return rect;
	}

	void WaitForPreviousFrame(
		IDXGISwapChain3* swapChain, 
		UINT *frameIndex, 
		ID3D12CommandQueue* commandQueue, 
		ID3D12Fence* fence, 
		HANDLE* fenceEvent)
	{
		// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
		// This is code implemented as such for simplicity. More advanced samples 
		// illustrate how to use fences for efficient resource usage.
		// cf.MicroSoft Sample Source

		static UINT fenceValue = 1;

		// Signal and increment the fence value.
		// cf.MicroSoft Sample Source

		const UINT64 cfenceValue = fenceValue;
		DirectX::ThrowIfFailed(
			commandQueue->Signal(fence, cfenceValue));
		fenceValue += 1;

		// Wait until the previous frame is finished.
		// cf.MicroSoft Sample Source
		if (fence->GetCompletedValue() < cfenceValue)
		{
			DirectX::ThrowIfFailed(
				fence->SetEventOnCompletion(cfenceValue, *fenceEvent));
			WaitForSingleObject(*fenceEvent, INFINITE);
		}

		*frameIndex = swapChain->GetCurrentBackBufferIndex();
	}

	void PrepareCommandList(
		ID3D12CommandAllocator* commandAllocator,
		ID3D12GraphicsCommandList* commandList,
		ID3D12PipelineState* pipeLineState,
		ID3D12RootSignature* rootSignature,
		ID3D12Resource** renderTarget,
		ID3D12DescriptorHeap* rtvDescriptorHeap,
		const UINT& rtvDescriptorSize,
		const D3D12_VIEWPORT& viewport,
		const D3D12_RECT& rect,
		const D3D12_VERTEX_BUFFER_VIEW* vertexBuffer,
		const int frameIndex
		)
	{
		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		// cf.MicroSoft Sample Source
		DirectX::ThrowIfFailed(commandAllocator->Reset());

		DirectX::ThrowIfFailed(commandList->Reset(commandAllocator, pipeLineState));

		// Set necessary state.
		// cf.MicroSoft Sample Source
		commandList->SetGraphicsRootSignature(rootSignature);
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &rect);

		D3D12_RESOURCE_BARRIER result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		result.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		result.Transition.pResource = renderTarget[frameIndex];
		result.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		result.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		result.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		
		// Indicate that the back buffer will be used as a render target.
		// cf.MicroSoft Sample Source
		commandList->ResourceBarrier(1, &result);

		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + frameIndex * rtvDescriptorSize;
		commandList->OMSetRenderTargets(1, &handle, FALSE, nullptr);

		// Record commands.
		// cf.MicroSoft Sample Source
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(handle, clearColor, 0, nullptr);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, vertexBuffer);
		commandList->DrawInstanced(3, 1, 0, 0);

		result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		result.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		result.Transition.pResource = renderTarget[frameIndex];
		result.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		result.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		result.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		// Indicate that the back buffer will now be used to present.
		// cf.MicroSoft Sample Source
		commandList->ResourceBarrier(1, &result);

		DirectX::ThrowIfFailed(commandList->Close());
	}
}