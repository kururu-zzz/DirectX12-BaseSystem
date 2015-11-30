#include "core/Window.h"
#include "core/d3d.h"
#include "core/dxgi.h"
#include "core/Config.h"
#include "core/SafeEventHandle.h"
#include "core/ConstantBufferManager.h"
#include "object/TextureContainer.h"
#include "object/Sprite.h"
#include "sequence/Sequence.h"
#include <ComUtil.h>
#include <crtdbg.h>
#include <vector>
#include <DirectXMath.h>

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
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	TCHAR* AppName = _T("DirectX12 BaseSystem");
	auto com = ComApartment();

	auto wndWidth  = config::wndResolution.at(0).first;
	auto wndHeight = config::wndResolution.at(0).second;
	auto wndAspect = static_cast<float>(wndWidth) / static_cast<float>(wndHeight);

	auto hWnd = InitWindow(AppName, hInstance, WndProc, static_cast<int>(wndWidth), static_cast<int>(wndHeight));

	d3d::CreateDevice();

	auto commandQueue = d3d::CreateCommandQueue();
	auto swapChain = dxgi::CreateSwapChain(commandQueue.get(), &hWnd);

	UINT frameIndex = swapChain->GetCurrentBackBufferIndex();

	auto commandAllocator = d3d::CreateCommandAllocator();
	auto bundleAllocator = d3d::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_BUNDLE);

	auto rootSignature = d3d::CreateRootSignature();

	std::vector<std::pair<std::string, DXGI_FORMAT>> semantics;
	semantics.emplace_back("IN_POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	semantics.emplace_back("IN_COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);
	semantics.emplace_back("IN_NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	semantics.emplace_back("IN_UV", DXGI_FORMAT_R32G32_FLOAT);

	/*auto spritePipelineState = d3d::CreatePipelineState(
		d3d::CreateInputLayout(semantics),
		rootSignature.get(),
		d3d::CreateBlob("resource/shader/VS/Sprite.hlsl", "RenderVS", "vs_5_0").get(),
		d3d::CreateBlob("resource/shader/GS/Sprite.hlsl", "RenderGS", "gs_5_0").get(),
		d3d::CreateBlob("resource/shader/PS/Sprite.hlsl", "RenderPS", "ps_5_0").get(),
		nullptr,
		nullptr,
		d3d::CreateRasterizerDesc(),
		d3d::CreateBlendDesc(d3d::BlendMode::default));*/

	//semantics.emplace_back("IN_BONEINDEX", DXGI_FORMAT_R32G32B32A32_UINT);
	//semantics.emplace_back("IN_BONEWEIGHT", DXGI_FORMAT_R32G32B32A32_FLOAT);

	auto modelPipelineState = d3d::CreatePipelineState(
		d3d::CreateInputLayout(semantics),
		rootSignature.get(),
		d3d::CreateBlob("resource/shader/VS/Model.hlsl", "RenderVS", "vs_5_0").get(),
		d3d::CreateBlob("resource/shader/GS/Model.hlsl", "RenderGS", "gs_5_0").get(),
		d3d::CreateBlob("resource/shader/PS/Model.hlsl", "RenderPS", "ps_5_0").get(),
		nullptr,
		nullptr,
		d3d::CreateRasterizerDesc(),
		d3d::CreateBlendDesc(d3d::BlendMode::default));

	auto rtvDescriptorHeap = d3d::CreateRTVDescriptorHeap();

	auto renderTargets = d3d::CreateRenderTargets(swapChain.get(), rtvDescriptorHeap.get());

	auto dsvDescriptorHeap = d3d::CreateDSVDescriptorHeap();

	auto dsvResource = d3d::CreateDepthStencilResoruce(wndWidth,wndHeight);

	d3d::CreateDepthStencilView(dsvResource.get(), dsvDescriptorHeap.get());

	auto commandList = d3d::CreateCommandList(commandAllocator.get(), modelPipelineState.get());
	auto bundleCommandList = d3d::CreateCommandList(bundleAllocator.get(), modelPipelineState.get(), D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_BUNDLE);
	DX12::InitTextureContainer(commandList);

	auto fence = d3d::CreateFence();

	auto fenceEvent = SafeEventHandle();

	d3d::WaitForPreviousFrame(swapChain.get(), &frameIndex, commandQueue.get(), fence.get(), fenceEvent.get());

	auto viewport = d3d::CreateViewport(static_cast<float>(wndWidth),static_cast<float>(wndHeight));

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
			d3d::UpdateD3D(
				commandAllocator.get(),
				commandList.get(),
				modelPipelineState.get(),
				rootSignature.get(),
				rtvDescriptorHeap.get(),
				renderTargets.at(frameIndex).get(),
				dsvDescriptorHeap.get(),
				viewport,
				rect,
				frameIndex);

			static SequenceManager sequenceManager;

			sequenceManager.Update();

			/*DirectX::XMFLOAT4X4 mtxViewport;
			DirectX::XMStoreFloat4x4(&mtxViewport, DirectX::XMMatrixIdentity());
			mtxViewport._11 = 2.f / 1200.f;
			mtxViewport._22 = -2.0f / 900.f;
			mtxViewport._41 = -1.f;
			mtxViewport._42 = 1.0f;
			DirectX::XMStoreFloat4x4(&mtxViewport, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&mtxViewport)));
			DX12::ConstantBuffer::SetBuffer(commandList.get(), mtxViewport, DX12::ConstantBuffer::BufferSlot::Camera);*/

			sequenceManager.Draw(commandList.get());

			d3d::EndRendering(
				commandList.get(),
				commandQueue.get(),
				swapChain.get(),
				renderTargets.at(frameIndex).get());

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

	return static_cast<int>(msg.wParam);
}