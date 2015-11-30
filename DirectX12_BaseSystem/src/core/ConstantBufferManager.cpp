#include "ConstantBufferManager.h"
#include <DirectXMath.h>
#include "../object/Camera.h"

namespace DX12
{
	struct Camera::CameraBuffer
	{
		DirectX::XMFLOAT4X4	mtxView;
		DirectX::XMFLOAT4X4	mtxProjection;
		DirectX::XMFLOAT4 cameraPos;
		CameraBuffer()
		{
			DirectX::XMStoreFloat4x4(&mtxView, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&mtxProjection, DirectX::XMMatrixIdentity());
			cameraPos = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f);
		}
	};
	struct MaterialParam
	{
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 emissive;
		DirectX::XMFLOAT4 specular;
	};
	namespace ConstantBuffer
	{
		static std::shared_ptr<ID3D12DescriptorHeap> descriptorHeap;

		template<typename T_n>
		std::shared_ptr<ID3D12Resource> CreateResource()
		{
			static std::shared_ptr<ID3D12Resource> resource;
			if(resource==nullptr){
				resource = d3d::CreateResource(sizeof(T_n));
			}
			return std::shared_ptr<ID3D12Resource>(resource);
		}

		template<typename T_n>
		void SetBuffer(ID3D12GraphicsCommandList* commandList, T_n& data, BufferSlot slotIndex)
		{
			if (descriptorHeap == nullptr)
			{
				descriptorHeap = d3d::CreateCBVDescriptorHeap();
			}
			auto resource = CreateResource<T_n>();
			d3d::CreateConstantBufferView(resource.get(), &data, sizeof(T_n), descriptorHeap.get(),static_cast<int>(slotIndex));
			auto cbv = descriptorHeap.get();
			commandList->SetDescriptorHeaps(1, &cbv);
			commandList->SetGraphicsRootDescriptorTable(0, descriptorHeap->GetGPUDescriptorHandleForHeapStart());
		}
		ID3D12DescriptorHeap* GetDescriptorHeap()
		{
			return descriptorHeap.get();
		}
		template void SetBuffer<DirectX::XMFLOAT4>(ID3D12GraphicsCommandList* commandList, DirectX::XMFLOAT4& data, BufferSlot slotIndex);
		template std::shared_ptr<ID3D12Resource> CreateResource<DirectX::XMFLOAT4>();
		template void SetBuffer<DirectX::XMFLOAT4X4>(ID3D12GraphicsCommandList* commandList, DirectX::XMFLOAT4X4& data, BufferSlot slotIndex);
		template std::shared_ptr<ID3D12Resource> CreateResource<DirectX::XMFLOAT4X4>();
		template void SetBuffer<Camera::CameraBuffer>(ID3D12GraphicsCommandList* commandList, Camera::CameraBuffer& data, BufferSlot slotIndex);
		template std::shared_ptr<ID3D12Resource> CreateResource<Camera::CameraBuffer>();
		template void SetBuffer<MaterialParam>(ID3D12GraphicsCommandList* commandList, MaterialParam& data, BufferSlot slotIndex);
		template std::shared_ptr<ID3D12Resource> CreateResource<MaterialParam>();
	}
}