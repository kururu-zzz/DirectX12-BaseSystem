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
	// !! FEATURE_LEVELの取得やろう
	std::shared_ptr<ID3D12Device> CreateDevice()
	{
#ifdef _DEBUG
		// Enable the D3D12 debug layer.
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
		//文字コード変換
		MultiByteToWideChar(CP_ACP, 0, fileName.c_str(), -1, path, MAX_PATH);

#ifdef _DEBUG
		// Enable better shader debugging with the graphics debugging tools.
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
			D3D12_DESCRIPTOR_RANGE ranges[1];
			D3D12_ROOT_PARAMETER rootParameters[1];

			ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			ranges[0].NumDescriptors = 1;
			ranges[0].BaseShaderRegister = 0;
			ranges[0].RegisterSpace = 0;
			ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
			rootParameters[0].DescriptorTable.pDescriptorRanges = &ranges[0];

			D3D12_STATIC_SAMPLER_DESC sampler = {};
			sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.MipLODBias = 0;
			sampler.MaxAnisotropy = 0;
			sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
			sampler.MinLOD = 0.0f;
			sampler.MaxLOD = D3D12_FLOAT32_MAX;
			sampler.ShaderRegister = 0;
			sampler.RegisterSpace = 0;
			sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			defaultDesc.NumParameters = 1;
			defaultDesc.pParameters = &rootParameters[0];
			defaultDesc.NumStaticSamplers = 1;
			defaultDesc.pStaticSamplers = &sampler;

			defaultDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

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
		// TRUEの場合、マルチレンダーターゲットで各レンダーターゲットのブレンドステートの設定を個別に設定できる
		// FALSEの場合、0番目のみが使用される
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

	std::shared_ptr<ID3D12PipelineState> CreatePipeLineState(
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
		if (hullBlob != nullptr)
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

	std::shared_ptr<ID3D12DescriptorHeap> CreateSRVDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC* descriptHeapDesc)
	{
		ID3D12DescriptorHeap* descriptorHeap;
		D3D12_DESCRIPTOR_HEAP_DESC defaultDesc = {};
		if (!descriptHeapDesc)
		{
			defaultDesc.NumDescriptors = 1;
			defaultDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			defaultDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
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
		return std::shared_ptr<ID3D12GraphicsCommandList>(commandList, ReleaseIUnknown);
	}

	std::vector<std::shared_ptr<ID3D12Resource>> CreateRenderTargets(ID3D12Device * device, IDXGISwapChain * swapChain, ID3D12DescriptorHeap* rtvDescriptorHeap, UINT renderTargetNum)
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
				nullptr,    // Clear value
				IID_PPV_ARGS(&resource)
				));
		return std::shared_ptr<ID3D12Resource>(resource, ReleaseIUnknown);
	}


	std::shared_ptr<ID3D12Resource> CreateTextureResoruce(ID3D12Device* device)
	{
		ID3D12Resource* resource;
		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = 256;
		textureDesc.Height = 256;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		DirectX::ThrowIfFailed(
			device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,    // Clear value
				IID_PPV_ARGS(&resource)
				));
		return std::shared_ptr<ID3D12Resource>(resource, ReleaseIUnknown);
	}

	void CreateShaderResourceView(ID3D12Device* device,ID3D12Resource* resource, ID3D12DescriptorHeap* srvDescriptorHeap)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = resource->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		device->CreateShaderResourceView(resource, &srvDesc, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}

	D3D12_VERTEX_BUFFER_VIEW CreateVetexBufferView(ID3D12Resource * resource,void* data, size_t vertexSize, UINT vertexNum)
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

	void MemcpySubresource(
		const D3D12_MEMCPY_DEST* pDest,
		const D3D12_SUBRESOURCE_DATA* pSrc,
		SIZE_T RowSizeInBytes,
		UINT NumRows,
		UINT NumSlices)
	{
		for (UINT z = 0; z < NumSlices; ++z)
		{
			BYTE* pDestSlice = reinterpret_cast<BYTE*>(pDest->pData) + pDest->SlicePitch * z;
			const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(pSrc->pData) + pSrc->SlicePitch * z;
			for (UINT y = 0; y < NumRows; ++y)
			{
				memcpy(pDestSlice + pDest->RowPitch * y,
					pSrcSlice + pSrc->RowPitch * y,
					RowSizeInBytes);
			}
		}
	}

	UINT64 UpdateSubresources(
		ID3D12GraphicsCommandList* commandList,
		ID3D12Resource* pDestinationResource,
		ID3D12Resource* pIntermediate,
		UINT FirstSubresource,
		UINT NumSubresources,
		UINT64 RequiredSize,
		const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
		const UINT* pNumRows,
		const UINT64* pRowSizesInBytes,
		const D3D12_SUBRESOURCE_DATA* pSrcData)
	{
		// Minor validation
		D3D12_RESOURCE_DESC IntermediateDesc = pIntermediate->GetDesc();
		D3D12_RESOURCE_DESC DestinationDesc = pDestinationResource->GetDesc();
		if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
			IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset ||
			RequiredSize >(SIZE_T) - 1 ||
			(DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&
				(FirstSubresource != 0 || NumSubresources != 1)))
		{
			return 0;
		}

		BYTE* pData;
		HRESULT hr = pIntermediate->Map(0, NULL, reinterpret_cast<void**>(&pData));
		if (FAILED(hr))
		{
			return 0;
		}

		for (UINT i = 0; i < NumSubresources; ++i)
		{
			if (pRowSizesInBytes[i] >(SIZE_T)-1) return 0;
			D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, pLayouts[i].Footprint.RowPitch * pNumRows[i] };
			MemcpySubresource(&DestData, &pSrcData[i], (SIZE_T)pRowSizesInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
		}
		pIntermediate->Unmap(0, NULL);

		if (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			D3D12_BOX srcBox;
			srcBox.left = UINT(pLayouts[0].Offset);
			srcBox.top = 0;
			srcBox.front = 0;
			srcBox.right = UINT(pLayouts[0].Offset + pLayouts[0].Footprint.Width);
			srcBox.bottom = 1;
			srcBox.back = 1;
			commandList->CopyBufferRegion(
				pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
		}
		else
		{
			for (UINT i = 0; i < NumSubresources; ++i)
			{
				D3D12_TEXTURE_COPY_LOCATION dst;
				D3D12_TEXTURE_COPY_LOCATION src;
				dst.pResource = pDestinationResource;
				dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				dst.SubresourceIndex = i + FirstSubresource;

				src.pResource = pIntermediate;
				src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				src.PlacedFootprint = pLayouts[i];
				commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
			}
		}
		return RequiredSize;
	}

	// Heap-allocating UpdateSubresources implementation
	void UpdateSubresources(
		ID3D12GraphicsCommandList* commandList,
		ID3D12Resource* textureResource,
		ID3D12Resource* textureHeapResource,
		UINT64 offset,
		UINT subResourceFirstIndex,
		UINT subResourceNum,
		D3D12_SUBRESOURCE_DATA* subResource)
	{
		UINT64 RequiredSize = 0;
		UINT64 MemToAlloc = static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * subResourceNum;
		if (MemToAlloc > SIZE_MAX)
		{
			throw std::exception();
		}
		void* pMem = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T>(MemToAlloc));
		if (pMem == NULL)
		{
			throw std::exception();
		}
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(pMem);
		UINT64* pRowSizesInBytes = reinterpret_cast<UINT64*>(pLayouts + subResourceNum);
		UINT* pNumRows = reinterpret_cast<UINT*>(pRowSizesInBytes + subResourceNum);

		D3D12_RESOURCE_DESC Desc = textureResource->GetDesc();
		ID3D12Device* device;
		textureResource->GetDevice(__uuidof(*device), reinterpret_cast<void**>(&device));
		device->GetCopyableFootprints(&Desc, subResourceFirstIndex, subResourceNum, offset, pLayouts, pNumRows, pRowSizesInBytes, &RequiredSize);
		device->Release();

		UINT64 Result = UpdateSubresources(commandList, textureResource, textureHeapResource, subResourceFirstIndex, subResourceNum, RequiredSize, pLayouts, pNumRows, pRowSizesInBytes, subResource);
		HeapFree(GetProcessHeap(), 0, pMem);

		D3D12_RESOURCE_BARRIER result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		result.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		result.Transition.pResource = textureResource;
		result.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		result.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		result.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &result);
		commandList->Close();
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

	D3D12_RECT CreateRect(LONG width, LONG height)
	{
		D3D12_RECT rect;
		rect.left = rect.top = 0;
		rect.right = width;
		rect.bottom = height;
		return rect;
	}

	void WaitForPreviousFrame(IDXGISwapChain3* swapChain, UINT *frameIndex, ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, HANDLE* fenceEvent)
	{
		// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
		// This is code implemented as such for simplicity. More advanced samples 
		// illustrate how to use fences for efficient resource usage.

		static UINT64 fenceValue = 1;
		// Signal and increment the fence value.
		const UINT64 cfenceValue = fenceValue;
		DirectX::ThrowIfFailed(
			commandQueue->Signal(fence, cfenceValue));
		fenceValue += 1;

		// Wait until the previous frame is finished.
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
		ID3D12DescriptorHeap* srvDescriptorHeap,
		ID3D12DescriptorHeap* rtvDescriptorHeap,
		const UINT& descriptorSize,
		const D3D12_VIEWPORT& viewport,
		const D3D12_RECT& rect,
		const D3D12_VERTEX_BUFFER_VIEW* vertexBuffer,
		const int frameIndex
		)
	{
		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		DirectX::ThrowIfFailed(commandAllocator->Reset());

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		DirectX::ThrowIfFailed(commandList->Reset(commandAllocator, pipeLineState));

		// Set necessary state.
		commandList->SetGraphicsRootSignature(rootSignature);

		commandList->SetDescriptorHeaps(1, &srvDescriptorHeap);

		commandList->SetGraphicsRootDescriptorTable(0, srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

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
		commandList->ResourceBarrier(1, &result);

		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + frameIndex * descriptorSize;
		commandList->OMSetRenderTargets(1, &handle, FALSE, nullptr);

		// Record commands.
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
		commandList->ResourceBarrier(1, &result);

		DirectX::ThrowIfFailed(commandList->Close());
	}
}