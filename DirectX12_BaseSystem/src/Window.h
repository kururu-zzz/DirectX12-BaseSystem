#pragma once

#include <cstdio>       // C/C++�W���w�b�_�[
#include <Windows.h>    // Windows API�w�b�_�[
#include <tchar.h>      // �}���`�o�C�g�����w�b�_�[
#include "../resource/resource.h"

// �E�B���h�E�쐬 
HWND InitWindow(TCHAR* AppName, HINSTANCE hInstance, WNDPROC WndProc, int width, int height){
	// �E�B���h�E���[�h 
	DWORD WindowStyle = WS_BORDER | WS_CAPTION | WS_SYSMENU;

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(DWORD);
	wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hIconSm = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = AppName;
	wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
	wc.hInstance = hInstance;
	::RegisterClassEx(&wc);

	RECT rc;
	::SetRect(&rc, 0, 0, width, height);
	// �E�B���h�E���[�h�̂Ƃ��^�C�g���o�[���l�����ĕ\���̈��ݒ肷�邽�߂ɕK�v�ȃT�C�Y���擾���� 
	::AdjustWindowRect(&rc, WindowStyle, FALSE);

	// �E�B���h�E���쐬 
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

	if (hWnd == nullptr) throw(_T("InitWindow()�ŃG���[���������܂����B�E�B���h�E���쐬�ł��܂���B"));
	return hWnd;
}