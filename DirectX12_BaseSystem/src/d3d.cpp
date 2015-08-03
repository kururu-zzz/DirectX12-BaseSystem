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
	// !! FEATURE_LEVELÇÃéÊìæÇ‚ÇÎÇ§
	std::shared_ptr<ID3D12Device> CreateDevice()
	{
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
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&device)
					));
		}
		return std::shared_ptr<ID3D12Device>(device, ReleaseIUnknown);
	}
	std::shared_ptr<ID3D12CommandQueue> CreateCommandQueue(ID3D12Device* device, const D3D12_COMMAND_QUEUE_DESC* queueDesc)
	{
		ID3D12CommandQueue* commandQueue;
		D3D12_COMMAND_QUEUE_DESC defaultDesc;
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
	std::shared_ptr<ID3DBlob> CreateBlob(const std::string& fileName, const std::string& shaderName, const std::string& shaderType)
	{
		ID3DBlob* blob;
		WCHAR	path[100];
		//ï∂éöÉRÅ[Éhïœä∑
		MultiByteToWideChar(CP_ACP, 0, fileName.c_str(), -1, path, MAX_PATH);

		DirectX::ThrowIfFailed(
			D3DCompileFromFile(
				path, nullptr, nullptr, shaderName.c_str(), shaderType.c_str(), 0, 0, &blob, nullptr
				));
		return std::shared_ptr<ID3DBlob>(blob, ReleaseIUnknown);
	}

}