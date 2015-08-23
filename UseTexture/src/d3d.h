#ifndef D3D___
#define D3D___

#include <d3d12.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace d3d
{
	enum class BlendMode 
	{
		default,
		add,
	};
}

namespace d3d
{
	/**
	*	@brief create ID3D12Device and return it;
	*	@return value succeeded:return std::shared_ptr<ID3D12Device>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12Device> CreateDevice();

	/**
	*	@brief create ID3D12CommandQueue and return it;
	*	@param device : ID3D12Device*
	*	@param queueDesc : D3D12_COMMAND_QUEUE_DESC* user defined.if nullptr,use default;
	*	@return value succeeded:return std::shared_ptr<ID3D12CommandQueue>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12CommandQueue> CreateCommandQueue(ID3D12Device* device, const D3D12_COMMAND_QUEUE_DESC* queueDesc = nullptr);

	/**
	*	@brief create ID3D12CommandAllocator and return it;
	*	@param device : ID3D12Device*
	*	@param commandList : D3D12_COMMAND_LIST_TYPE user defined;
	*	@return value succeeded:return std::shared_ptr<ID3D12CommandAllocator>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12CommandAllocator> CreateCommandAllocator(ID3D12Device* device,D3D12_COMMAND_LIST_TYPE listType = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

	/**
	*	@brief create ID3D12CommandQueue and return it;
	*	@param fileName : hlsl fileName
	*	@param shaderName : shader function name
	*	@param shaderType : shader type(e.g.vs_5_0)
	*	@return value succeeded:return std::shared_ptr<ID3DBlob>,failed:throw std::exception
	*/
	std::shared_ptr<ID3DBlob> CreateBlob(const std::string& fileName, const std::string& shaderName, const std::string& shaderType);
	
	/**
	*	@brief create ID3D12RootSignature and return it;
	*	@param device : ID3D12Device*
	*	@param rootDesc : D3D12_ROOT_SIGNATURE_DESC* user defined.if nullptr,use default;
	*	@return value succeeded:return std::shared_ptr<ID3D12RootSignature>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12RootSignature> CreateRootSignature(ID3D12Device* device, D3D12_ROOT_SIGNATURE_DESC* rootDesc = nullptr);

	/**
	*	@brief create D3D12_BLEND_DESC and return it;
	*	@param blendMode : default,add;
	*	@return value succeeded:return std::shared_ptr<D3D12_BLEND_DESC>,failed:throw std::exception
	*/
	D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode = BlendMode::default);

	/**
	*	@brief create D3D12_RASTERIZER_DESC and return it;
	*	@return value succeeded:return std::shared_ptr<D3D12_RASTERIZER_DESC>,failed:throw std::exception
	*/
	D3D12_RASTERIZER_DESC CreateRasterizerDesc();

	/**
	*	@brief create D3D12_INPUT_LAYOUT_DESC and return it;
	*   @param semantics : unorderd_map whose key is std::string,value is DXGI_FORMAT of vertex element
	*	@return D3D12_INPUT_LAYOUT_DESC
	*/
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout(const std::unordered_map<std::string, DXGI_FORMAT>& semantics);

	/**
	*	@brief create ID3D12PipelineState and return it;
	*	@param device : ID3D12Device*
	*	@return value succeeded:return std::shared_ptr<ID3D12PipelineState>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12PipelineState> CreatePipeLineState(ID3D12Device * device,const D3D12_INPUT_LAYOUT_DESC& layout,ID3D12RootSignature* rootSignature,ID3DBlob* vertexBlob,ID3DBlob* geometryBlob,ID3DBlob* pixelBlob,const D3D12_RASTERIZER_DESC& rasterizeDesc,const D3D12_BLEND_DESC& blend);

	/**
	*	@brief create ID3D12DescriptorHeap and return it;
	*	@param device : ID3D12Device*
	*	@param descriptHeapDesc : D3D12_DESCRIPTOR_HEAP_DESC* user defined.if nullptr,use default;
	*	@return value succeeded:return std::shared_ptr<ID3D12DescriptorHeap>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC* descriptHeapDesc = nullptr);

	/**
	*	@brief create ID3D12GraphicsCommandList and return it;
	*	@param device : ID3D12Device*
	*	@param lineType : D3D12_COMMAND_LIST_TYPE,use same type when create ID3D12CommandList
	*	@param commandAllocator : ID3D12CommandAllocator*
	*	@param pipeLineState : ID3D12PipelineState*
	*	@return value succeeded:return std::shared_ptr<ID3D12GraphicsCommandList>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12GraphicsCommandList> CreateCommandList(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator, ID3D12PipelineState* pipeLineState,D3D12_COMMAND_LIST_TYPE listType = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT );

	/**
	*	@brief create ID3D12Resource for RenderTarget and return it;
	*	@param device : ID3D12Device*
	*	@param bufferIndex : index of buffer in swapChain
	*	@param swapChain : IDXGISwapChain
	*	@param handle : D3D12_CPU_DESCRIPTOR_HANDLE
	*	@return value succeeded:return std::shared_ptr<ID3D12Resource>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12Resource> CreateRenderTarget(ID3D12Device* device,UINT bufferIndex, IDXGISwapChain* swapChain,const D3D12_CPU_DESCRIPTOR_HANDLE& handle);

	/**
	*	@brief create ID3D12Resource for RenderTarget and return it;
	*	@param device : ID3D12Device*
	*	@param size : size of resource data
	*	@return value succeeded:return std::shared_ptr<ID3D12Resource>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12Resource> CreateResoruce(ID3D12Device* device, size_t size);

	
	std::shared_ptr<ID3D12Resource> CreateResoruce(ID3D12Device* device, D3D12_RESOURCE_DESC* resourceDesc,D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON);

	/**
	*	@brief create D3D12_VERTEX_BUFFER_VIEW and return it;
	*	@param resource : ID3D12Resource*
	*	@param data : vertex data
	*	@param vertexSize : size of vertex struct
	*	@param vertexNum : num of vertex
	*	@return value succeeded:return std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW>,failed:throw std::exception
	*/
	D3D12_VERTEX_BUFFER_VIEW CreateVetexBufferView(ID3D12Resource* resource,void* data, size_t vertexSize,UINT vertexNum);

	UINT64 UpdateSubresources(
		ID3D12GraphicsCommandList* commandList,
		ID3D12Resource* pDestinationResource,
		ID3D12Resource* pIntermediate,
		UINT64 IntermediateOffset,
		UINT FirstSubresource,
		UINT NumSubresources,
		D3D12_SUBRESOURCE_DATA* pSrcData);

	/**
	*	@brief create ID3D12Fence and return it;
	*	@param device : ID3D12Device*
	*	@param flag : D3D12_FENCE_FLAGS ,default D3D12_FENCE_FLAG_NONE
	*	@return value succeeded:return std::shared_ptr<ID3D12Fence>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12Fence> CreateFence(ID3D12Device* device, D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE);

	void WaitForPreviousFrame(IDXGISwapChain3* swapChain,UINT* frameIndex,ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64* fenceValue, HANDLE* fenceEvent);

	void PopulateCommandList(
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
		const int frameIndex);
}

#endif