#include "Window.h"
#include "d3d.h"
#include "dxgi.h"
#include <ComUtil.h>
#include <crtdbg.h>

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
	auto descriptorHeap = d3d::CreateDescriptorHeap(device.get());
	auto renderTarget = d3d::CreateRenderTarget(device.get(), swapChain.get(), descriptorHeap.get());
	auto pipeLine = d3d::CreatePipeLineState(device.get(), layout,rootSignature.get(), vertexBlob.get(), geometryBlob.get(), pixelBlob.get(), d3d::CreateRasterizerDesc(), d3d::CreateBlendDesc(d3d::BlendMode::default));
	auto commandList = d3d::CreateCommandList(device.get(), commandAllocator.get(), pipeLine.get());

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
		}
	} while (msg.message != WM_QUIT);

	DestroyWindow(hWnd);

	::UnregisterClass(AppName, hInstance);

	return msg.wParam;
}