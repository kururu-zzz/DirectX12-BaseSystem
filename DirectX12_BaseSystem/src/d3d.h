#ifndef D3D___
#define D3D___

#include <d3d12.h>
#include <memory>
#include <string>

namespace d3d
{
	/**
	*	@brief create ID3D12Device and return it;
	*	@return value succeeded:return std::shared_ptr<ID3D12Device>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12Device> CreateDevice();

	/**
	*	@brief create ID3D12CommandQueue and return it;
	*	@param ID3D12Device*
	*	@param ID3D12_COMMAND_QUEUE* user defined
	*	@return value succeeded:return std::shared_ptr<ID3D12CommandQueue>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12CommandQueue> CreateCommandQueue(ID3D12Device* device, const D3D12_COMMAND_QUEUE_DESC* queueDesc = nullptr);

	/**
	*	@brief create ID3D12CommandQueue and return it;
	*	@param fileName : hlsl fileName
	*	@param shaderName : shader function name
	*	@param shaderType : shader type(e.g.vs_5_0)
	*	@return value succeeded:return std::shared_ptr<ID3DBlob>,failed:throw std::exception
	*/
	std::shared_ptr<ID3DBlob> CreateBlob(const std::string& fileName, const std::string& shaderName, const std::string& shaderType);
}

#endif