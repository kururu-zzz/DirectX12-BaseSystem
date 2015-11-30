#include "WorldMatrix.h"
#include "../../core/ConstantBufferManager.h"
#include "../../core/d3d.h"

namespace DX12
{
	struct WorldMatrix::WorldBuffer
	{
		DirectX::XMFLOAT4X4 mtxWorld;
		WorldBuffer()
		{
			XMStoreFloat4x4(&mtxWorld, DirectX::XMMatrixIdentity());
		}
	};

	WorldMatrix::WorldMatrix(){
		world = std::make_shared<WorldBuffer>();
		scale = DirectX::XMFLOAT3(1.f,1.f,1.f);
		translate = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
		XMStoreFloat4x4(&rotate, DirectX::XMMatrixIdentity());
	}

	void WorldMatrix::SetTranslate(const DirectX::XMFLOAT3& translate){
		this->translate = translate;
	}

	void WorldMatrix::SetRotate(const DirectX::XMFLOAT3& angle){
		auto matRotateX = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(angle.x));
		auto matRotateY = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(angle.y));
		auto matRotateZ = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(angle.z));
		XMStoreFloat4x4(&rotate, matRotateX*matRotateY*matRotateZ);
	}

	void WorldMatrix::SetRotate(const DirectX::XMFLOAT4X4& rotate)
	{
		this->rotate = rotate;
	}

	void WorldMatrix::SetScale(const DirectX::XMFLOAT3& scale)
	{
		this->scale = scale;
	}

	void WorldMatrix::SetConstantBuffer(ID3D12GraphicsCommandList* commandList){
		auto mtxTranslate = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);
		auto mtxScale = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
		auto mtxRotate = DirectX::XMLoadFloat4x4(&rotate);
		DirectX::XMStoreFloat4x4(&world->mtxWorld, DirectX::XMMatrixTranspose(mtxScale*mtxRotate*mtxTranslate));
		DX12::ConstantBuffer::SetBuffer(commandList, world->mtxWorld, DX12::ConstantBuffer::BufferSlot::World);
	}
}