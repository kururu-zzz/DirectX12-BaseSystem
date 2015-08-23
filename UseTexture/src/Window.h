#pragma once

#include <cstdio>       // C/C++標準ヘッダー
#include <Windows.h>    // Windows APIヘッダー
#include <tchar.h>      // マルチバイト文字ヘッダー

// ウィンドウ作成 
HWND InitWindow(TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, int width, int height){
	// ウィンドウモード 
	DWORD WindowStyle = WS_BORDER | WS_CAPTION | WS_SYSMENU;

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(DWORD);
	wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = nullptr;
	wc.hIconSm = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = AppName;
	wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
	wc.hInstance = hInstance;
	::RegisterClassEx(&wc);

	RECT rc;
	::SetRect(&rc, 0, 0, width, height);
	// ウィンドウモードのときタイトルバーを考慮して表示領域を設定するために必要なサイズを取得する 
	::AdjustWindowRect(&rc, WindowStyle, FALSE);

	// ウィンドウを作成 
	auto hWnd = ::CreateWindow(
		wc.lpszClassName,
		wc.lpszClassName,
		WindowStyle,
		0,
		0,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (hWnd == nullptr) throw(_T("InitWindow()でエラーが発生しました。ウィンドウが作成できません。"));
	return hWnd;
}