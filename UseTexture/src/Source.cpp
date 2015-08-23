#include "Window.h"
#include "d3d.h"
#include "dxgi.h"
#include <ComUtil.h>
#include <crtdbg.h>
#include <vector>
#include <WICTextureLoader.h>

#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 uv;
};

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> GenerateTextureData()
{
	static const UINT textureWidth = 256;
	static const UINT textureHeight = 256;
	static const UINT texturePixelSize = 4;
	const UINT rowPitch = textureWidth * texturePixelSize;
	const UINT cellPitch = rowPitch >> 3;		// The width of a cell in the checkboard texture.
	const UINT cellHeight = textureWidth >> 3;	// The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * textureHeight;
	std::vector<UINT8> data(textureSize);

	for (UINT n = 0; n < textureSize; n += texturePixelSize)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			data[n] = 0x00;		// R
			data[n + 1] = 0x00;	// G
			data[n + 2] = 0x00;	// B
			data[n + 3] = 0xff;	// A
		}
		else
		{
			data[n] = 0xff;		// R
			data[n + 1] = 0xff;	// G
			data[n + 2] = 0xff;	// B
			data[n + 3] = 0xff;	// A
		}
	}

	return data;
}

// ウィンドウプロシージャ 
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_KEYUP:
		// アプリ終了 
		if (wParam == VK_ESCAPE) DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0L;
}


// メイン関数 
int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE,
	LPTSTR,
	INT
	) {
	// アプリケーションの初期化
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// アプリケーション名 
	TCHAR* AppName = _T("UseTexture");
	auto com = ComApartment();

	// ウィンドウの作成
	auto hWnd = InitWindow(AppName, hInstance, WndProc, 1200, 900);

	auto device = d3d::CreateDevice();
	auto commandQueue = d3d::CreateCommandQueue(device.get());
	auto swapChain = dxgi::CreateSwapChain(device.get(), commandQueue.get(), &hWnd);

	UINT frameIndex = swapChain->GetCurrentBackBufferIndex();

	auto commandAllocator = d3d::CreateCommandAllocator(device.get());
	auto vertexBlob = d3d::CreateBlob("shader/Sample.hlsl", "RenderVS", "vs_5_0");
	auto geometryBlob = d3d::CreateBlob("shader/Sample.hlsl", "RenderGS", "gs_5_0");
	auto pixelBlob = d3d::CreateBlob("shader/Sample.hlsl", "RenderPS", "ps_5_0");

	auto rootSignature = d3d::CreateRootSignature(device.get());
	Vertex triangleVerts[] =
	{
		{ { 0.0f, 0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f },   {0.5f,0.5f} },
		{ { 0.45f, -0.5, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } , {1.f,1.f}   },
		{ { -0.45f, -0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f },{0.f,1.f}   }
	};

	std::unordered_map<std::string, DXGI_FORMAT> semantics;
	semantics.emplace("IN_POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	semantics.emplace("IN_COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);
	semantics.emplace("IN_UV", DXGI_FORMAT_R32G32_FLOAT);

	auto layout = d3d::CreateInputLayout(semantics);

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

	auto textureResource = d3d::CreateResoruce(device.get(), &textureDesc, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COPY_DEST);

	UINT64 bufSize;
	device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &bufSize);

	auto textureData = GenerateTextureData();
	auto textureHeapResource = d3d::CreateResoruce(device.get(), bufSize);

	D3D12_SUBRESOURCE_DATA textureResourceData = {};
	textureResourceData.pData = &textureData[0];
	textureResourceData.RowPitch = 256 * 4;
	textureResourceData.SlicePitch = textureResourceData.RowPitch * 256;

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	
	auto srvDescriptor = d3d::CreateDescriptorHeap(device.get(), &srvHeapDesc);

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(textureResource.get(), &srvDesc, srvDescriptor->GetCPUDescriptorHandleForHeapStart());


	/*Create RenderTargetView*/
	UINT rtvDescriptorSize;
	auto rtvDescriptorHeap = d3d::CreateDescriptorHeap(device.get());

	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	const int renderTargetNum = 2;
	std::vector<std::shared_ptr<ID3D12Resource>> renderTargets;
	renderTargets.reserve(renderTargetNum);
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	handle.ptr = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	for (UINT i = 0; i < renderTargetNum; ++i)
	{
		auto renderTarget = d3d::CreateRenderTarget(device.get(), i, swapChain.get(), handle);
		renderTargets.emplace_back(renderTarget);
		handle.ptr += rtvDescriptorSize;
	}

	auto pipeLine = d3d::CreatePipeLineState(device.get(), layout,rootSignature.get(), vertexBlob.get(), geometryBlob.get(), pixelBlob.get(), d3d::CreateRasterizerDesc(), d3d::CreateBlendDesc(d3d::BlendMode::default));
	auto commandList = d3d::CreateCommandList(device.get(), commandAllocator.get(), pipeLine.get());

	{
		d3d::UpdateSubresources(commandList.get(), textureResource.get(), textureHeapResource.get(), 0, 0, 1, &textureResourceData);
		D3D12_RESOURCE_BARRIER result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		result.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		result.Transition.pResource = textureResource.get();
		result.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		result.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		result.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &result);
		commandList->Close();
	}

	auto vertexResource = d3d::CreateResoruce(device.get(), sizeof(triangleVerts)*sizeof(Vertex));
	auto vertexBufferView = d3d::CreateVetexBufferView(vertexResource.get(), triangleVerts, sizeof(Vertex), _countof(triangleVerts));

	ID3D12CommandList* ppCommandLists[] = { commandList.get() };
	commandQueue->ExecuteCommandLists(1, ppCommandLists);

	auto fence = d3d::CreateFence(device.get());

	UINT64 fenceValue = 1;

	// Create an event handle to use for frame synchronization.
	auto fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (fenceEvent == nullptr)
	{
		HRESULT_FROM_WIN32(GetLastError());
	}

	d3d::WaitForPreviousFrame(swapChain.get(), &frameIndex, commandQueue.get(), fence.get(), &fenceValue, &fenceEvent);

	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = viewport.TopLeftY = 0.f;
	viewport.Width = static_cast<float>(1200.f);
	viewport.Height = static_cast<float>(900.f);
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	D3D12_RECT rect;
	rect.left = rect.top = 0.f;
	rect.right = static_cast<LONG>(1200.f);
	rect.bottom = static_cast<LONG>(900.f);

	::ShowWindow(hWnd, SW_SHOW);
	::UpdateWindow(hWnd);
	ShowCursor(FALSE);
	// メッセージループ 
	MSG msg = { 0 };
	do {
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else {
			std::vector<ID3D12Resource*> transRenderTarget;
			for (auto& renderTarget : renderTargets)
			{
				transRenderTarget.emplace_back(renderTarget.get());
			}

			d3d::PopulateCommandList(commandAllocator.get(), commandList.get(), pipeLine.get(), rootSignature.get(), transRenderTarget.data(),srvDescriptor.get(), rtvDescriptorHeap.get(), rtvDescriptorSize, viewport, rect, &vertexBufferView, frameIndex);

			// Execute the command list.
			ID3D12CommandList* ppCommandLists[] = { commandList.get() };
			commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			// Present the frame.
			swapChain->Present(1, 0);

			d3d::WaitForPreviousFrame(swapChain.get(),&frameIndex,commandQueue.get(),fence.get(),&fenceValue,&fenceEvent);
		}
	} while (msg.message != WM_QUIT);

	DestroyWindow(hWnd);

	::UnregisterClass(AppName, hInstance);

	return msg.wParam;
}