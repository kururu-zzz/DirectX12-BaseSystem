#include "Window.h"
#include "d3d.h"
#include "dxgi.h"
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


// メイン関数 
int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE,
	LPTSTR,
	INT
	) {
	// アプリケーションの初期化
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// アプリケーション名 
	TCHAR* AppName = _T("DirectX12 BaseSystem");
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
		{ { 0.0f, 0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.45f, -0.5, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.45f, -0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }
	};
	D3D12_INPUT_ELEMENT_DESC layoutElem[] =
	{
		{ "IN_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "IN_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	D3D12_INPUT_LAYOUT_DESC layout = { layoutElem,_countof(layoutElem) };
	UINT descriptorSize;
	auto descriptorHeap = d3d::CreateDescriptorHeap(device.get(),&descriptorSize);

	descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	const int renderTargetNum = 2;
	std::vector<std::shared_ptr<ID3D12Resource>> renderTargets;
	renderTargets.reserve(renderTargetNum);
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	handle.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	for (UINT i = 0; i < renderTargetNum; ++i)
	{
		auto renderTarget = d3d::CreateRenderTarget(device.get(), i, swapChain.get(), handle);
		renderTargets.emplace_back(renderTarget);
		handle.ptr += descriptorSize;
	}

	auto pipeLine = d3d::CreatePipeLineState(device.get(), layout,rootSignature.get(), vertexBlob.get(), geometryBlob.get(), pixelBlob.get(), d3d::CreateRasterizerDesc(), d3d::CreateBlendDesc(d3d::BlendMode::default));
	auto commandList = d3d::CreateCommandList(device.get(), commandAllocator.get(), pipeLine.get());

	commandList->Close();

	auto vertexResource = d3d::CreateResoruce(device.get(), sizeof(triangleVerts)*sizeof(Vertex));
	auto vertexBufferView = d3d::CreateVetexBufferView(vertexResource.get(), triangleVerts, sizeof(Vertex), _countof(triangleVerts));

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

			d3d::PopulateCommandList(commandAllocator.get(), commandList.get(), pipeLine.get(), rootSignature.get(), transRenderTarget.data(), descriptorHeap.get(), descriptorSize, viewport, rect, &vertexBufferView, frameIndex);

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