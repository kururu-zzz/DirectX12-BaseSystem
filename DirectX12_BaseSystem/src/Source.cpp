#include "core/Window.h"
#include "core/d3d.h"
#include "core/dxgi.h"
#include "core/Config.h"
#include "core/SafeEventHandle.h"
#include "object/TextureContainer.h"
#include "object/Sprite.h"
#include <ComUtil.h>
#include <crtdbg.h>
#include <vector>

#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
};

struct mtxCamera
{
	DirectX::XMFLOAT4X4 mtxViewport;
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
	float wndAspect = static_cast<float>(wndWidth) / static_cast<float>(wndHeight);

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
	
	/*create rtv*/
	auto rtvDescriptorHeap = d3d::CreateRTVDescriptorHeap();

	/*create cbv*/
	auto cbvDescriptorHeap = d3d::CreateCBVDescriptorHeap();

	auto renderTargets = d3d::CreateRenderTargets(swapChain.get(), rtvDescriptorHeap.get());

	auto pipeLineState = d3d::CreatePipeLineState(
		layout,
		rootSignature.get(),
		vertexBlob.get(),
		geometryBlob.get(),
		pixelBlob.get(),
		nullptr,
		nullptr,
		d3d::CreateRasterizerDesc(),
		d3d::CreateBlendDesc(d3d::BlendMode::default));
	auto commandList = d3d::CreateCommandList(commandAllocator.get(), pipeLineState.get());
	auto bundleCommandList = d3d::CreateCommandList(bundleAllocator.get(), pipeLineState.get(), D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_BUNDLE);
	DX12::InitTextureContainer(commandList);

	UINT8* dataBegin;
	mtxCamera cameraBuffer;
	auto constantBufferResource = d3d::CreateResource(sizeof(mtxCamera));
	d3d::CreateConstantBufferView(constantBufferResource.get(), &cameraBuffer, sizeof(mtxCamera), &dataBegin, cbvDescriptorHeap.get());

	auto fence = d3d::CreateFence();

	auto fenceEvent = SafeEventHandle();

	d3d::WaitForPreviousFrame(swapChain.get(), &frameIndex, commandQueue.get(), fence.get(), fenceEvent.get());

	auto viewport = d3d::CreateViewport(static_cast<float>(wndWidth),static_cast<float>(wndHeight));

	auto rect = d3d::CreateRect(static_cast<LONG>(wndWidth), static_cast<LONG>(wndHeight));

	{
		DirectX::XMStoreFloat4x4(&cameraBuffer.mtxViewport, DirectX::XMMatrixIdentity());
		cameraBuffer.mtxViewport._11 = 2.f / wndWidth;
		cameraBuffer.mtxViewport._22 = -2.0f / wndHeight;
		cameraBuffer.mtxViewport._41 = -1.f;
		cameraBuffer.mtxViewport._42 = 1.0f;
		DirectX::XMStoreFloat4x4(&cameraBuffer.mtxViewport, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&cameraBuffer.mtxViewport)));
		memcpy(dataBegin, &cameraBuffer, sizeof(mtxCamera));
	}

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
			d3d::BeginRendering(
				commandAllocator.get(),
				commandList.get(),
				pipeLineState.get(),
				rootSignature.get(),
				rtvDescriptorHeap.get(),
				transRenderTarget.data(),
				viewport,
				rect,
				frameIndex
				);

			DX12::Sprite sprite[12];

			for (int i = 0; i < 12; ++i)
			{
				sprite[i].Init(
					DirectX::XMFLOAT3(i % 4 * 300.f, i / 4 * 300.f, 0.f),
					DirectX::XMFLOAT2(300.f, 300.f),
					DirectX::XMFLOAT4(0.f, 0.f, 1.f, 1.f),
					"resource\\texture\\sample2.dds"
					);
			}

			sprite[5].ChangeImage("resource\\texture\\sample.dds");

			d3d::ResetCommandList(bundleAllocator.get(), bundleCommandList.get(), pipeLineState.get());
			bundleCommandList->SetGraphicsRootSignature(rootSignature.get());
			//SetGraphicsRootDescriptorTable call after SetDescriptorHeap 
			{
				auto tmp = cbvDescriptorHeap.get();
				bundleCommandList->SetDescriptorHeaps(1, &tmp);
				bundleCommandList->SetGraphicsRootDescriptorTable(0, cbvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			}
			for (int i = 0; i < 12; ++i)
			{
				sprite[i].Draw(bundleCommandList.get());
			}

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
				pipeLineState.get(),
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