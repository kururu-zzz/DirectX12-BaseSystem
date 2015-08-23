#include "Window.h"
#include "d3d.h"
#include "dxgi.h"
#include "SafeEventHandle.h"
#include <ComUtil.h>
#include <crtdbg.h>
#include <vector>

#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

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

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE,
	LPTSTR,
	INT
	) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	TCHAR* AppName = _T("DrawPolygon");
	auto com = ComApartment();

	const float wndWidth = 1200.f;
	const float wndHeight = 900.f;

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
		{ { 0.0f, 0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.45f, -0.5, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.45f, -0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	std::unordered_map<std::string, DXGI_FORMAT> semantics;
	semantics.emplace("IN_POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	semantics.emplace("IN_COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);

	auto layout = d3d::CreateInputLayout(semantics);

	auto rtvDescriptorHeap = d3d::CreateRTVDescriptorHeap(device.get());

	UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto renderTargets = d3d::CreateRenderTargets(device.get(), swapChain.get(), rtvDescriptorHeap.get());

	auto pipeLine = d3d::CreatePipeLineState(
		device.get(),
		layout, rootSignature.get(),
		vertexBlob.get(),
		geometryBlob.get(),
		pixelBlob.get(),
		nullptr,
		nullptr,
		d3d::CreateRasterizerDesc(),
		d3d::CreateBlendDesc(d3d::BlendMode::default));

	auto commandList = d3d::CreateCommandList(device.get(), commandAllocator.get(), pipeLine.get());

	auto vertexResource = d3d::CreateResoruce(device.get(), sizeof(triangleVerts)*sizeof(Vertex));
	auto vertexBufferView = d3d::CreateVetexBufferView(vertexResource.get(), triangleVerts, sizeof(Vertex), _countof(triangleVerts));

	auto fence = d3d::CreateFence(device.get());
	auto fenceEvent = SafeEventHandle();

	d3d::WaitForPreviousFrame(
		swapChain.get(),
		&frameIndex,
		commandQueue.get(),
		fence.get(),
		fenceEvent.get());

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
				rtvDescriptorHeap.get(),
				rtvDescriptorSize,
				viewport,
				rect,
				&vertexBufferView,
				frameIndex);

			ID3D12CommandList* ppCommandLists[] = { commandList.get() };
			commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			swapChain->Present(1, 0);

			d3d::WaitForPreviousFrame(
				swapChain.get(),
				&frameIndex,
				commandQueue.get(),
				fence.get(),
				fenceEvent.get());
		}
	} while (msg.message != WM_QUIT);

	DestroyWindow(hWnd);

	d3d::WaitForPreviousFrame(swapChain.get(), &frameIndex, commandQueue.get(), fence.get(), fenceEvent.get());

	::UnregisterClass(AppName, hInstance);

	return static_cast<int>(msg.wParam);
}