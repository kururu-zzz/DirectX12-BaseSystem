#pragma comment( lib, "d3d12.lib" )

#include "d3d.h"
#include "dxgi.h"
#include <d3dcompiler.h>
#include <PlatformHelpers.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
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

		
		ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
		rasterizerDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
		rasterizerDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.ForcedSampleCount = 0;
		rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		rasterizerDesc.DepthClipEnable = TRUE;

		return rasterizerDesc;
	}

	std::shared_ptr<ID3D12PipelineState> CreatePipeLineState(
		ID3D12Device * device,
		ID3D12RootSignature* rootSignature,
		ID3DBlob* vertexBlob,
		ID3DBlob* geometryBlob,
		ID3DBlob* pixelBlob,
		const D3D12_RASTERIZER_DESC& rasterizeDesc,
		const D3D12_BLEND_DESC& blend)
	{
		D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "IN_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "IN_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		ID3D12PipelineState* pipelineState;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
		gpsDesc.InputLayout = { layout, sizeof(layout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };
		gpsDesc.pRootSignature = rootSignature;
		gpsDesc.VS = { reinterpret_cast<UINT8*>(vertexBlob->GetBufferPointer()), vertexBlob->GetBufferSize() };
		gpsDesc.GS = { reinterpret_cast<UINT8*>(geometryBlob->GetBufferPointer()), geometryBlob->GetBufferSize() };
		gpsDesc.PS = { reinterpret_cast<UINT8*>(pixelBlob->GetBufferPointer()), pixelBlob->GetBufferSize() };
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

	std::shared_ptr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC* descriptHeapDesc)
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
		DirectX::ThrowIfFailed(
			device->CreateDescriptorHeap(
				descriptHeapDesc, 
				IID_PPV_ARGS(&descriptorHeap)
				));
		return std::shared_ptr<ID3D12DescriptorHeap>(descriptorHeap, ReleaseIUnknown);
	}

	std::shared_ptr<ID3D12CommandList> CreateCommandList(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator,ID3D12PipelineState* pipeLineState, D3D12_COMMAND_LIST_TYPE listType)
	{
		ID3D12CommandList* commandList;
		DirectX::ThrowIfFailed(
			device->CreateCommandList(
				0,
				listType,
				commandAllocator,
				pipeLineState,
				IID_PPV_ARGS(&commandList)
				));
		return std::shared_ptr<ID3D12CommandList>(commandList, ReleaseIUnknown);
	}

	std::shared_ptr<ID3D12Resource> CreateRenderTarget(ID3D12Device * device, IDXGISwapChain * swapChain, ID3D12DescriptorHeap * descriptorHeap)
	{
		ID3D12Resource* renderTarget;
		DirectX::ThrowIfFailed(
			swapChain->GetBuffer(0, IID_PPV_ARGS(&renderTarget)));
		device->CreateRenderTargetView(renderTarget, nullptr, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
		return std::shared_ptr<ID3D12Resource>(renderTarget,ReleaseIUnknown);
	}

	std::shared_ptr<ID3D12Resource> CreateResoruce(ID3D12Device* device, void* pData, size_t size)
	{
		ID3D12Resource* resource;
		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc;
		resourceDesc.Width = size;
		resourceDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
		resourceDesc.Alignment = 0;
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
}