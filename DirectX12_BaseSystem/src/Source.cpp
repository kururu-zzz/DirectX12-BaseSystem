#include "Window.h"
#include "d3d.h"
#include "dxgi.h"
#include <ComUtil.h>
#include <crtdbg.h>


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
	auto swapChain = dxgi::CreateSwapChain(hWnd, device.get(), commandQueue.get());

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