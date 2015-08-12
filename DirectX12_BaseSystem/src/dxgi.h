#ifndef DXGI___
#define DXGI___

#include <Windows.h>
#include <dxgi1_4.h>
#include <memory>

struct ID3D12Device;
struct ID3D12CommandQueue;

namespace dxgi
{
	std::shared_ptr<IDXGIFactory4> CreateFactory();
	std::shared_ptr<IDXGIAdapter3> CreateAdapter(IDXGIFactory4* factory);

	std::shared_ptr<IDXGISwapChain3> CreateSwapChain(ID3D12Device* device, ID3D12CommandQueue* commandQueue, const HWND* hWnd, DXGI_SWAP_CHAIN_DESC* swapChainDesc = nullptr);
}

#endif