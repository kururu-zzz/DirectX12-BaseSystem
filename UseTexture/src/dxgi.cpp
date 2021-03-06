#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxgi.lib" )

#include "d3d.h"
#include "dxgi.h"
#include <PlatformHelpers.h>

namespace dxgi
{
	void ReleaseIUnknown(IUnknown* p)
	{
		p->Release();
	}
	std::shared_ptr<IDXGIFactory4> CreateFactory()
	{
		IDXGIFactory4* factory;
		DirectX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
		return std::shared_ptr<IDXGIFactory4>(factory, ReleaseIUnknown);
	}
	std::shared_ptr<IDXGIAdapter3> CreateAdapter(IDXGIFactory4* factory)
	{
		IDXGIAdapter3* adapter;
		DirectX::ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)));
		return std::shared_ptr<IDXGIAdapter3>(adapter, ReleaseIUnknown);
	}
	std::shared_ptr<IDXGISwapChain3> CreateSwapChain( ID3D12Device * device, ID3D12CommandQueue * commandQueue, const HWND* hWnd, DXGI_SWAP_CHAIN_DESC* swapChainDesc)
	{
		IDXGISwapChain* swapChain;
		IDXGISwapChain3* swapChain3;

		DXGI_SWAP_CHAIN_DESC defaultDesc = {};
		if (!swapChainDesc)
		{
			defaultDesc.BufferCount = 2;
			defaultDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			defaultDesc.BufferDesc.Width = 1200;
			defaultDesc.BufferDesc.Height = 900;
			defaultDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			defaultDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			defaultDesc.OutputWindow = *hWnd;
			defaultDesc.SampleDesc.Count = 1;
			defaultDesc.Windowed = TRUE;
			swapChainDesc = &defaultDesc;
		}

		DirectX::ThrowIfFailed(CreateFactory()->CreateSwapChain(
			commandQueue,
			swapChainDesc,
			&swapChain
			));
		swapChain->QueryInterface(IID_PPV_ARGS(&swapChain3));
		swapChain->Release();
		return std::shared_ptr<IDXGISwapChain3>(swapChain3, ReleaseIUnknown);
	}
}