#include "Window.h"
#include "d3d.h"
#include "dxgi.h"
#include "SafeEventHandle.h"
#include "DDSTextureLoader.h"
#include <ComUtil.h>
#include <crtdbg.h>
#include <vector>

#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 uv;
};

struct ConstantBuffer
{
	DirectX::XMFLOAT4 offset;
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

	float wndWidth = 1200.f;
	float wndHeight = 900.f;
	float wndAspect = static_cast<float>(wndWidth) / static_cast<float>(wndHeight);

	auto hWnd = InitWindow(AppName, hInstance, WndProc, static_cast<int>(wndWidth), static_cast<int>(wndHeight));

	auto device = d3d::CreateDevice();
	auto commandQueue = d3d::CreateCommandQueue(device.get());
	auto swapChain = dxgi::CreateSwapChain(device.get(), commandQueue.get(), &hWnd);

	UINT frameIndex = swapChain->GetCurrentBackBufferIndex();

	auto commandAllocator = d3d::CreateCommandAllocator(device.get());
	auto bundleAllocator = d3d::CreateCommandAllocator(device.get(), D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_BUNDLE);
	auto vertexBlob = d3d::CreateBlob("resource/shader/Sample.hlsl", "RenderVS", "vs_5_0");
	auto geometryBlob = d3d::CreateBlob("resource/shader/Sample.hlsl", "RenderGS", "gs_5_0");
	auto pixelBlob = d3d::CreateBlob("resource/shader/Sample.hlsl", "RenderPS", "ps_5_0");

	auto rootSignature = d3d::CreateRootSignature(device.get());
	auto side = 0.2f;
	Vertex triangleVerts[] =
	{
		{ {-side, side*wndAspect, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.f,0.f } },
		{ { side, side*wndAspect, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 1.f,0.f } },
		{ {-side,-side*wndAspect, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.f,1.f } },
		{ { side,-side*wndAspect, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.f,1.f } },
	};
	std::unordered_map<std::string, DXGI_FORMAT> semantics;
	semantics.emplace("IN_POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	semantics.emplace("IN_COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);
	semantics.emplace("IN_UV", DXGI_FORMAT_R32G32_FLOAT);

	auto layout = d3d::CreateInputLayout(semantics);

	/*create srv*/
	auto width = 256;
	auto height = 256;
	auto textureResource = d3d::CreateTextureResoruce(device.get(),width,height);

	UINT64 bufSize;
	device->GetCopyableFootprints(&textureResource->GetDesc(), 0, 1, 0, nullptr, nullptr, nullptr, &bufSize);

	std::vector<UINT8> textureData;
	DirectX::CreateTextureData("resource\\texture\\sample.dds",&textureData);
	auto textureHeapResource = d3d::CreateResoruce(device.get(), bufSize);

	D3D12_SUBRESOURCE_DATA textureResourceData = {};
	textureResourceData.pData = &textureData[0];
	textureResourceData.RowPitch = width * 4;
	textureResourceData.SlicePitch = textureResourceData.RowPitch * height;

	auto srvDescriptorHeap = d3d::CreateSRVDescriptorHeap(device.get());

	d3d::CreateShaderResourceView(device.get(), textureResource.get(), srvDescriptorHeap.get());

	/*create rtv*/
	auto rtvDescriptorHeap = d3d::CreateRTVDescriptorHeap(device.get());

	UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	/*create cbv*/
	auto cbvDescriptorHeap = d3d::CreateCBVDescriptorHeap(device.get());

	auto renderTargets = d3d::CreateRenderTargets(device.get(), swapChain.get(), rtvDescriptorHeap.get());

	auto pipeLineState = d3d::CreatePipeLineState(
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
	auto commandList = d3d::CreateCommandList(device.get(), commandAllocator.get(), pipeLineState.get());
	auto bundleCommandList = d3d::CreateCommandList(device.get(), bundleAllocator.get(), pipeLineState.get(), D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_BUNDLE);

	d3d::ResetCommandList(commandAllocator.get(), commandList.get(), pipeLineState.get());
	d3d::UpdateSubresources(commandList.get(), commandQueue.get(), textureResource.get(), textureHeapResource.get(), 0, 0, 1, &textureResourceData);

	auto vertexResource = d3d::CreateResoruce(device.get(), sizeof(triangleVerts));
	auto vertexBufferView = d3d::CreateVetexBufferView(vertexResource.get(), triangleVerts, sizeof(Vertex), _countof(triangleVerts));

	UINT8* dataBegin;
	ConstantBuffer cBuffer;
	auto constantBufferResource = d3d::CreateResoruce(device.get(), 1024 * 64);
	d3d::CreateConstantBufferView(device.get(), constantBufferResource.get(), &cBuffer, sizeof(ConstantBuffer), &dataBegin, cbvDescriptorHeap.get());

	d3d::PrepareBundle(bundleAllocator.get(), bundleCommandList.get(), pipeLineState.get(), rootSignature.get(), cbvDescriptorHeap.get(), srvDescriptorHeap.get(),&vertexBufferView);

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

			{
				const float translationSpeed = 0.005f;
				const float offsetBounds = 1.25f;

				cBuffer.offset.x += translationSpeed;
				if (cBuffer.offset.x > offsetBounds)
				{
					cBuffer.offset.x = -offsetBounds;
				}
				memcpy(dataBegin, &cBuffer, sizeof(ConstantBuffer));
			}

			std::vector<ID3D12Resource*> transRenderTarget;
			for (auto& renderTarget : renderTargets)
			{
				transRenderTarget.emplace_back(renderTarget.get());
			}

			d3d::PrepareCommandList(
				commandAllocator.get(),
				commandList.get(),
				bundleCommandList.get(),
				pipeLineState.get(),
				rootSignature.get(),
				transRenderTarget.data(),
				rtvDescriptorHeap.get(),
				cbvDescriptorHeap.get(),
				srvDescriptorHeap.get(),
				rtvDescriptorSize,
				viewport,
				rect,
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