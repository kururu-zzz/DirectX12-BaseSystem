#include "Window.h"
#include "d3d.h"
#include "dxgi.h"
#include "SafeEventHandle.h"
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

	const float wndWidth = 1200.f;
	const float wndHeight = 900.f;

	// ウィンドウの作成
	auto hWnd = InitWindow(AppName, hInstance, WndProc, static_cast<int>(wndWidth), static_cast<int>(wndHeight));

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

	std::vector<std::pair<std::string, DXGI_FORMAT>> semantics;
	semantics.emplace_back("IN_POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	semantics.emplace_back("IN_COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);
	semantics.emplace_back("IN_UV", DXGI_FORMAT_R32G32_FLOAT);

	auto layout = d3d::CreateInputLayout(semantics);

	auto textureResource = d3d::CreateTextureResoruce(device.get());

	UINT64 bufSize;
	device->GetCopyableFootprints(&textureResource->GetDesc(), 0, 1, 0, nullptr, nullptr, nullptr, &bufSize);

	auto textureData = GenerateTextureData();
	auto textureHeapResource = d3d::CreateResource(device.get(), bufSize);

	D3D12_SUBRESOURCE_DATA textureResourceData = {};
	textureResourceData.pData = &textureData[0];
	textureResourceData.RowPitch = 256 * 4;
	textureResourceData.SlicePitch = textureResourceData.RowPitch * 256;
	
	auto srvDescriptorHeap = d3d::CreateSRVDescriptorHeap(device.get());

	d3d::CreateShaderResourceView(device.get(), textureResource.get(), srvDescriptorHeap.get());

	auto rtvDescriptorHeap = d3d::CreateRTVDescriptorHeap(device.get());

	UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto renderTargets = d3d::CreateRenderTargets(device.get(),swapChain.get(),rtvDescriptorHeap.get());

	auto pipeLine = d3d::CreatePipeLineState(
		device.get(), 
		layout,
		rootSignature.get(), 
		vertexBlob.get(), 
		geometryBlob.get(), 
		pixelBlob.get(), 
		nullptr,
		nullptr,
		d3d::CreateRasterizerDesc(), 
		d3d::CreateBlendDesc(d3d::BlendMode::default));
	auto commandList = d3d::CreateCommandList(device.get(), commandAllocator.get(), pipeLine.get());

	d3d::UpdateSubresources(commandList.get(), textureResource.get(), textureHeapResource.get(), 0, 0, 1, &textureResourceData);

	auto vertexResource = d3d::CreateResource(device.get(), sizeof(triangleVerts)*sizeof(Vertex));
	auto vertexBufferView = d3d::CreateVetexBufferView(vertexResource.get(), triangleVerts, sizeof(Vertex), _countof(triangleVerts));

	ID3D12CommandList* ppCommandLists[] = { commandList.get() };
	commandQueue->ExecuteCommandLists(1, ppCommandLists);

	auto fence = d3d::CreateFence(device.get());

	auto fenceEvent = SafeEventHandle();

	d3d::WaitForPreviousFrame(swapChain.get(), &frameIndex, commandQueue.get(), fence.get(), fenceEvent.get());

	auto viewport = d3d::CreateViewport(wndWidth, wndHeight);

	auto rect = d3d::CreateRect(static_cast<LONG>(wndWidth), static_cast<LONG>(wndHeight));

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

			d3d::PrepareCommandList(
				commandAllocator.get(), 
				commandList.get(), 
				pipeLine.get(), 
				rootSignature.get(), 
				transRenderTarget.data(),
				srvDescriptorHeap.get(), 
				rtvDescriptorHeap.get(), 
				rtvDescriptorSize, 
				viewport, 
				rect, 
				&vertexBufferView, 
				frameIndex);

			// Execute the command list.
			ID3D12CommandList* ppCommandLists[] = { commandList.get() };
			commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			// Present the frame.
			swapChain->Present(1, 0);

			d3d::WaitForPreviousFrame(
				swapChain.get(),
				&frameIndex,
				commandQueue.get(),
				fence.get(),
				fenceEvent.get());
		}
	} while (msg.message != WM_QUIT);

	d3d::WaitForPreviousFrame(swapChain.get(), &frameIndex, commandQueue.get(), fence.get(), fenceEvent.get());

	DestroyWindow(hWnd);

	::UnregisterClass(AppName, hInstance);

	return msg.wParam;
}