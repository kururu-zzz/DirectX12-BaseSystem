#ifndef DXGI___
#define DXGI___

#include <Windows.h>
#include <dxgi1_4.h>
#include <memory>

struct ID3D12Device;
struct ID3D12CommandQueue;

namespace dxgi
{
	/**
	*	@brief create IDXGIFactory4 and return it;
	*	@return succeeded:return std::shared_ptr<IDXGIFactory4>,failed:throw std::exception
	*/
	std::shared_ptr<IDXGIFactory4> CreateFactory();

	/**
	*	@brief create IDXGIAdapter3 and return it;
	*	@param factory : IDXGIFactory4*;
	*	@return succeeded:return std::shared_ptr<IDXGIAdapter4>,failed:throw std::exception
	*/
	std::shared_ptr<IDXGIAdapter3> CreateAdapter(IDXGIFactory4* factory);

	/**
	*	@brief create IDXGISwapChain3 and return it;
	*	@param device  : ID3D12Device*;
	*	@param commandQueue : ID3D12CommandQueue*;
	*	@param hWnd : HANDLE for Window;
	*	@param rootDesc : DXGI_SWAP_CHAIN_DESC* user defined.if nullptr,use default;
	*	@return succeeded:return std::shared_ptr<IDXGIAdapter4>,failed:throw std::exception
	*/
	std::shared_ptr<IDXGISwapChain3> CreateSwapChain(ID3D12Device* device, ID3D12CommandQueue* commandQueue, const HWND* hWnd, DXGI_SWAP_CHAIN_DESC* swapChainDesc = nullptr);
}

#endif