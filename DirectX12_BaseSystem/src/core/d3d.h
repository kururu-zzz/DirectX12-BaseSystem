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
	*	@brief create ID3D12CommandQueue and return it;
	*	
	*	@param queueDesc : D3D12_COMMAND_QUEUE_DESC* user defined.if nullptr,use default;
	*	@return succeeded:return std::shared_ptr<ID3D12CommandQueue>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12CommandQueue> CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* queueDesc = nullptr);

	/**
	*	@brief create ID3D12CommandAllocator and return it;
	*	
	*	@param commandList : D3D12_COMMAND_LIST_TYPE user defined;
	*	@return succeeded:return std::shared_ptr<ID3D12CommandAllocator>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE listType = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

	/**
	*	@brief create ID3D12Blob and return it;
	*	@param fileName : hlsl fileName
	*	@param shaderName : shader function name
	*	@param shaderType : shader type(e.g.vs_5_0)
	*	@return succeeded:return std::shared_ptr<ID3DBlob>,failed:throw std::exception
	*/
	std::shared_ptr<ID3DBlob> CreateBlob(const std::string& fileName, const std::string& shaderName, const std::string& shaderType);

	/**
	*	@brief create ID3D12RootSignature and return it;
	*	
	*	@param rootDesc : D3D12_ROOT_SIGNATURE_DESC* user defined.if nullptr,use default;
	*	@return succeeded:return std::shared_ptr<ID3D12RootSignature>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12RootSignature> CreateRootSignature(D3D12_ROOT_SIGNATURE_DESC* rootDesc = nullptr);

	/**
	*	@brief create D3D12_BLEND_DESC and return it;
	*	@param blendMode : default,add;
	*	@return D3D12_BLEND_DESC
	*/
	D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode = BlendMode::default);

	/**
	*	@brief create D3D12_RASTERIZER_DESC and return it;
	*	@return D3D12_RASTERIZER_DESC
	*/
	D3D12_RASTERIZER_DESC CreateRasterizerDesc();

	/**
	*	@brief create D3D12_INPUT_LAYOUT_DESC and return it;
	*   @param semantics : vector of std::pair<std::string of semantics name,DXGI_FORMAT of vertex element>
	*	@return D3D12_INPUT_LAYOUT_DESC
	*/
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout(const std::vector<std::pair<std::string, DXGI_FORMAT>>& semantics);

	/**
	*	@brief create ID3D12PipelineState and return it;
	*	;
	*	@param layout : const D3D12_INPUT_LAYOUT_DESC&;
	*	@param rootSignature : ID3D12RootSignature*;
	*	@param vertexBlob : ID3DBlob* for vertexShader;
	*	@param geomeryBlob : ID3DBlob* for geometryShader;
	*	@param pixelBlob : ID3DBlob* for pixelShader;
	*	@param hullBlob : ID3DBlob* for hullShader;
	*	@param domainBlob : ID3DBlob* for domainShader;
	*	@return succeeded:return std::shared_ptr<ID3D12PipelineState>,failed:throw std::exception;
	*/
	std::shared_ptr<ID3D12PipelineState> CreatePipelineState(
		const D3D12_INPUT_LAYOUT_DESC& layout,
		ID3D12RootSignature* rootSignature,
		ID3DBlob* vertexBlob,
		ID3DBlob* geometryBlob,
		ID3DBlob* pixelBlob,
		ID3DBlob* hullBlob,
		ID3DBlob* domainBlob,
		const D3D12_RASTERIZER_DESC& rasterizeDesc,
		const D3D12_BLEND_DESC& blend);

	/**
	*	@brief create ID3D12DescriptorHeap for RenderTargetView and return it;
	*	;
	*	@param descriptHeapDesc : D3D12_DESCRIPTOR_HEAP_DESC* user defined.if nullptr,use default;
	*	@return succeeded:return std::shared_ptr<ID3D12DescriptorHeap>,failed:throw std::exception;
	*/
	std::shared_ptr<ID3D12DescriptorHeap> CreateRTVDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC* descriptHeapDesc = nullptr);

	/**
	*	@brief create ID3D12DescriptorHeap for ConstantBuffertView and return it;
	*	;
	*	@param descriptHeapDesc : D3D12_DESCRIPTOR_HEAP_DESC* user defined.if nullptr,use default;
	*	@return succeeded:return std::shared_ptr<ID3D12DescriptorHeap>,failed:throw std::exception;
	*/
	std::shared_ptr<ID3D12DescriptorHeap> CreateCBVDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC* descriptHeapDesc = nullptr);

	/**
	*	@brief create ID3D12DescriptorHeap for ShaderResourceView and return it;
	*	;
	*	@param descriptHeapDesc : D3D12_DESCRIPTOR_HEAP_DESC* user defined.if nullptr,use default;
	*	@return succeeded:return std::shared_ptr<ID3D12DescriptorHeap>,failed:throw std::exception;
	*/
	std::shared_ptr<ID3D12DescriptorHeap> CreateSRVDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC* descriptHeapDesc = nullptr);

	/**
	*	@brief create ID3D12GraphicsCommandList and return it;
	*	;
	*	@param commandAllocator : ID3D12CommandAllocator*;
	*	@param pipeLineState : ID3D12PipelineState*;
	*	@param listType : D3D12_COMMAND_LIST_TYPE,use same type when create ID3D12CommandList;
	*	@return succeeded:return std::shared_ptr<ID3D12GraphicsCommandList>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12GraphicsCommandList> CreateCommandList(ID3D12CommandAllocator* commandAllocator, ID3D12PipelineState* pipeLineState, D3D12_COMMAND_LIST_TYPE listType = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

	/**
	*	@brief create ID3D12Resource for RenderTarget and return it;
	*	;
	*	@param swapChain : IDXGISwapChain;
	*	@param rtvDescriptorHeap : ID3D12DescriptorHeap for renderTarget;
	*	@param renderTargetNum : num of renderTarget : default value is 2;
	*	@return succeeded:return std::vector<std::shared_ptr<ID3D12Resource>>,failed:throw std::exception
	*/
	std::vector<std::shared_ptr<ID3D12Resource>> CreateRenderTargets(IDXGISwapChain* swapChain, ID3D12DescriptorHeap* rtvDescriptorHeap, UINT renderTargetNum = 2);

	/**
	*	@brief create ID3D12Resource for RenderTarget and return it;
	*	@param size : size of resource data
	*	@return succeeded:return std::shared_ptr<ID3D12Resource>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12Resource> CreateResource(size_t size);

	/**
	*	@brief create ID3D12Resource for RenderTarget and return it;
	*	@param textureResource : resource data for texture
	*	@return succeeded:return std::shared_ptr<ID3D12Resource>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12Resource> CreateResource(ID3D12Resource* textureResource);

	/**
	*	@brief create ID3D12Resource for Texture and return it;
	*	
	*	@param width : width of texture
	*	@param height : height of texture
	*	@return succeeded:return std::shared_ptr<ID3D12Resource>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12Resource> CreateTextureResoruce(UINT width,UINT height);

	/**
	*	@brief create D3D12_VERTEX_BUFFER_VIEW and return it;
	*	@param resource : ID3D12Resource*
	*	@param data : vertex data
	*	@param vertexSize : size of vertex struct
	*	@param vertexNum : num of vertex
	*	@return D3D12_VERTEX_BUFFER_VIEW
	*/
	D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView(ID3D12Resource* resource, void* data, size_t vertexSize, UINT vertexNum);

	/**
	*	@brief create D3D12_INDEX_BUFFER_VIEW and return it;
	*	@param resource : ID3D12Resource*
	*	@param data : vertex data
	*	@param indexSize : size of index
	*	@param vertexNum : num of vertex
	*	@return D3D12_INDEX_BUFFER_VIEW
	*/
	D3D12_INDEX_BUFFER_VIEW CreateIndexBufferView(ID3D12Resource* resource, void* data, size_t vertexSize, UINT vertexNum);

	/**
	*	@brief create ConstantBufferView;
	*	;
	*	@param resource : ID3D12Resource*;
	*	@param data : vertex data;
	*	@param constantBufferSize : size of constantBuffer struct;
	*	@param dataBegin : begining of constantBuffer data;
	*	@param cbvDescriptorHeap : ID3D12DescriptorHeap;
	*/
	void CreateConstantBufferView(ID3D12Resource* resource, void* data, size_t constantBufferSize, ID3D12DescriptorHeap* cbvDescriptorHeap);

	/**
	*	@brief create ShaderResourceView;
	*	
	*	@param resource : ID3D12Resource*
	*	@param srvDescriptorHeap : ID3D12DescriptorHeap*
	*/
	void CreateShaderResourceView(ID3D12Resource* resource, ID3D12DescriptorHeap* srvDescriptorHeap);

	/**
	*	@brief update D3D12_SUBRESOURCE_DATA;
	*	@param commandList : ID3D12GraphicsCommandList*;
	*	@param textureResource : ID3D12Resource*
	*	@param textureHeapResource : ID3D12Resource*
	*	@param offset : data offset of subResource
	*	@param subResourceFirstIndex : first index of subResource
	*	@param subResourceNum : num of subResource
	*	@param subResource : D3D12_SUBRESOURCE_DATA*
	*/
	void UpdateSubresources(
		ID3D12GraphicsCommandList* commandList,
		ID3D12Resource* textureResource,
		ID3D12Resource* textureHeapResource,
		UINT64 offset,
		UINT subResourceFirstIndex,
		UINT subResourceNum,
		D3D12_SUBRESOURCE_DATA* subResource);

	/**
	*	@brief create ID3D12Fence and return it;
	*	
	*	@param flag : D3D12_FENCE_FLAGS ,default D3D12_FENCE_FLAG_NONE
	*	@return succeeded:return std::shared_ptr<ID3D12Fence>,failed:throw std::exception
	*/
	std::shared_ptr<ID3D12Fence> CreateFence(D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE);

	/**
	*	@brief create D3D12_VIEWPORT and return it;
	*	@param width : viewport width;
	*	@param height : viewport height;
	*	@return D3D12_VIEWPORT
	*/
	D3D12_VIEWPORT CreateViewport(float width, float height);

	/**
	*	@brief create D3D12_RECT and return it;
	*	@param width : rect width:
	*	@param height : rect height;
	*	@return D3D12_RECT
	*/
	D3D12_RECT CreateRect(LONG width, LONG height);

	void UpdateD3D(
		ID3D12CommandAllocator* commandAllocator,
		ID3D12GraphicsCommandList* commandList,
		ID3D12PipelineState* pipeLineState,
		ID3D12RootSignature* rootSignature,
		ID3D12DescriptorHeap* rtvDescriptorHeap,
		ID3D12Resource** renderTarget, 
		const D3D12_VIEWPORT& viewport,
		const D3D12_RECT& rect,
		const int frameIndex);

	void EndRendering(
		ID3D12CommandAllocator* commandAllocator,
		ID3D12GraphicsCommandList* commandList,
		ID3D12CommandQueue* commandQueue,
		ID3D12PipelineState* pipeLineState,
		ID3D12RootSignature* rootSignature,
		IDXGISwapChain* swapChain,
		ID3D12Resource** renderTarget,
		const int frameIndex);

	/**
	*	@brief wait execution of commandList in previous frame ;
	*	@param swapChain : IDXGISwapChain3*;
	*	@param frameIndex : currendFrameIndex;
	*	@param commandQueue : ID3D12CommandQueue* stored command;
	*	@param fence : ID3D12Fence*;
	*	@param fenceEvent : eventHandle for fence;
	*/
	void WaitForPreviousFrame(
		IDXGISwapChain3* swapChain,
		UINT* frameIndex,
		ID3D12CommandQueue* commandQueue,
		ID3D12Fence* fence,
		HANDLE* fenceEvent);

	/**
	*	@brief reset commandList if closed;
	*	@param commandAllocator : ID3D12CommandAllocator*;
	*	@param commandList : ID3D12GraphicsCommandList*;
	*	@param pipeLineState : ID3D12PipelineState*;
	*/
	void ResetCommandList(ID3D12CommandAllocator* commandAllocator,ID3D12GraphicsCommandList* commandList, ID3D12PipelineState* pipeLineState);
}

#endif