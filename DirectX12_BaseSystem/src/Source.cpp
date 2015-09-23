#include "core/Window.h"
#include "core/d3d.h"
#include "core/dxgi.h"
#include "core/Config.h"
#include "core/SafeEventHandle.h"
#include "object/TextureContainer.h"
#include "object/Sprite.h"
#include "sequence/Sequence.h"
#include <ComUtil.h>
#include <crtdbg.h>
#include <vector>
#include <DirectXMath.h>


struct mtxCamera
{
	DirectX::XMFLOAT4X4 mtxView;
	DirectX::XMFLOAT4X4 mtxProjection;
	mtxCamera()
	{
		DirectX::XMStoreFloat4x4(&mtxView,		 DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&mtxProjection, DirectX::XMMatrixIdentity());
	}
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
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	TCHAR* AppName = _T("DirectX12 BaseSystem");
	auto com = ComApartment();

	auto wndWidth  = config::wndResolution.at(0).first;
	auto wndHeight = config::wndResolution.at(0).second;
	auto wndAspect = static_cast<float>(wndWidth) / static_cast<float>(wndHeight);

	auto hWnd = InitWindow(AppName, hInstance, WndProc, static_cast<int>(wndWidth), static_cast<int>(wndHeight));

	auto commandQueue = d3d::CreateCommandQueue();
	auto swapChain = dxgi::CreateSwapChain(commandQueue.get(), &hWnd);

	UINT frameIndex = swapChain->GetCurrentBackBufferIndex();

	auto commandAllocator = d3d::CreateCommandAllocator();
	auto bundleAllocator = d3d::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_BUNDLE);
	auto vertexBlob = d3d::CreateBlob("resource/shader/VS/Sprite.hlsl", "RenderVS", "vs_5_0");
	auto geometryBlob = d3d::CreateBlob("resource/shader/GS/Sprite.hlsl", "RenderGS", "gs_5_0");
	auto pixelBlob = d3d::CreateBlob("resource/shader/PS/Sprite.hlsl", "RenderPS", "ps_5_0");

	auto rootSignature = d3d::CreateRootSignature();

	std::vector<std::pair<std::string, DXGI_FORMAT>> semantics;
	semantics.emplace_back("IN_POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	semantics.emplace_back("IN_COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);
	semantics.emplace_back("IN_NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	semantics.emplace_back("IN_UV", DXGI_FORMAT_R32G32_FLOAT);

	auto layout = d3d::CreateInputLayout(semantics);
	
	auto rtvDescriptorHeap = d3d::CreateRTVDescriptorHeap();

	auto cbvDescriptorHeap = d3d::CreateCBVDescriptorHeap();

	auto renderTargets = d3d::CreateRenderTargets(swapChain.get(), rtvDescriptorHeap.get());

	auto pipelineState = d3d::CreatePipelineState(
		layout,
		rootSignature.get(),
		vertexBlob.get(),
		geometryBlob.get(),
		pixelBlob.get(),
		nullptr,
		nullptr,
		d3d::CreateRasterizerDesc(),
		d3d::CreateBlendDesc(d3d::BlendMode::default));

	auto commandList = d3d::CreateCommandList(commandAllocator.get(), pipelineState.get());
	auto bundleCommandList = d3d::CreateCommandList(bundleAllocator.get(), pipelineState.get(), D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_BUNDLE);
	DX12::InitTextureContainer(commandList);

	DirectX::XMFLOAT4X4 mtxViewport;
	auto constantBufferResource = d3d::CreateResource(sizeof(DirectX::XMFLOAT4X4));

	{
		DirectX::XMStoreFloat4x4(&mtxViewport, DirectX::XMMatrixIdentity());
		mtxViewport._11 = 2.f / wndWidth;
		mtxViewport._22 = -2.0f / wndHeight;
		mtxViewport._41 = -1.f;
		mtxViewport._42 = 1.0f;
		DirectX::XMStoreFloat4x4(&mtxViewport, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&mtxViewport)));
	}

	d3d::CreateConstantBufferView(constantBufferResource.get(), &mtxViewport, sizeof(DirectX::XMFLOAT4X4), cbvDescriptorHeap.get());

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

			std::vector<ID3D12Resource*> transRenderTarget;
			for (auto& renderTarget : renderTargets)
			{
				transRenderTarget.emplace_back(renderTarget.get());
			}
			d3d::UpdateD3D(
				commandAllocator.get(),
				commandList.get(),
				pipelineState.get(),
				rootSignature.get(),
				rtvDescriptorHeap.get(),
				transRenderTarget.data(),
				viewport,
				rect,
				frameIndex
				);

			static SequenceManager sequenceManager;

			sequenceManager.Update();

			d3d::ResetCommandList(bundleAllocator.get(), bundleCommandList.get(), pipelineState.get());
			bundleCommandList->SetGraphicsRootSignature(rootSignature.get());
			{
				auto tmp = cbvDescriptorHeap.get();
				bundleCommandList->SetDescriptorHeaps(1, &tmp);
				bundleCommandList->SetGraphicsRootDescriptorTable(0, cbvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			}
			sequenceManager.Draw(bundleCommandList.get());

			bundleCommandList->Close();

			auto cbv = cbvDescriptorHeap.get();
			commandList->SetDescriptorHeaps(1, &cbv);
			auto srv = DX12::GetLastUseSRV();
			commandList->SetDescriptorHeaps(1, &srv);

			commandList->ExecuteBundle(bundleCommandList.get());

			d3d::EndRendering(
				commandAllocator.get(),
				commandList.get(),
				commandQueue.get(),
				pipelineState.get(),
				rootSignature.get(),
				swapChain.get(),
				transRenderTarget.data(),
				frameIndex
				);

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