#include "Camera.h"
#include "../core/d3d.h"
#include "../core/ConstantBufferManager.h"

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

	inline bool operator!= (const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
	{
		return !((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
	}

	Camera::Camera()
	{
		camera = std::make_shared<CameraBuffer>();
	}

	void Camera::Init(
		const DirectX::XMFLOAT3& pos,
		const DirectX::XMFLOAT3& focus,
		const DirectX::XMFLOAT3& upDirection,
		const float& cameraAngle,
		const DirectX::XMFLOAT2& screenSize,
		const float& nearZ,
		const float& farZ)
	{
		this->SetPos(pos);
		this->SetFocus(focus);
		this->SetUpDirection(upDirection);

		this->SetCameraAngle(cameraAngle);
		this->SetScreenAspect(screenSize);

		this->SetNearZ(nearZ);
		this->SetFarZ(farZ);

		this->ApplyLookAtLH();
		this->ApplyPerspectiveFovLH();
	}

	void Camera::SetPos(const DirectX::XMFLOAT3& pos)
	{
		if (this->pos != pos)
		{
			this->pos = pos;
			if ((pos.x == focus.x) && (pos.y == focus.y) && (pos.z == focus.z))//LookAtLH—p‚Ì•â³
				this->pos.z += 0.01f;
			camera->cameraPos = DirectX::XMFLOAT4(pos.x,pos.y,pos.z,1.f);
		}
	}

	void Camera::SetFocus(const DirectX::XMFLOAT3& focus)
	{
		if (this->focus != focus)
		{
			this->focus = focus;
			if ((pos.x == focus.x) && (pos.y == focus.y) && (pos.z == focus.z))
				pos.z += 0.01f;
		}
	}
	
	void Camera::SetUpDirection(const DirectX::XMFLOAT3& upDirection)
	{
		DirectX::XMStoreFloat3(&this->upDirection, DirectX::XMVector3Normalize(XMLoadFloat3(&upDirection)));
	}

	void Camera::SetCameraAngle(const float& cameraAngle)
	{
		if (this->cameraAngle != cameraAngle)
		{
			this->cameraAngle = cameraAngle;
		}
	}

	void Camera::SetScreenAspect(const float& screenAspect)
	{
		if (this->screenAspect != screenAspect)
		{
			this->screenAspect = screenAspect;
		}
	}

	void Camera::SetNearZ(const float& nearZ)
	{
		if (this->nearZ != nearZ)
		{
			this->nearZ = nearZ;
		}
	}

	void Camera::SetFarZ(const float& farZ)
	{
		if (this->farZ != farZ)
		{
			this->farZ = farZ;
		}
	}

	void Camera::SetScreenAspect(const DirectX::XMFLOAT2& screenSize)
	{
		auto screenAspect = screenSize.x / screenSize.y;
		SetScreenAspect(screenAspect);
	}

	void Camera::ApplyLookAtLH()
	{
		DirectX::XMStoreFloat4x4(&camera->mtxView,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixLookAtLH(
					DirectX::XMLoadFloat3(&this->pos),
					DirectX::XMLoadFloat3(&this->focus),
					DirectX::XMLoadFloat3(&this->upDirection)
					)));
	}

	void Camera::ApplyLookAtRH()
	{
		DirectX::XMStoreFloat4x4(&camera->mtxView,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixLookAtRH(
					DirectX::XMLoadFloat3(&this->pos),
					DirectX::XMLoadFloat3(&this->focus),
					DirectX::XMLoadFloat3(&this->upDirection)
					)));
	}

	void Camera::ApplyPerspectiveFovLH()
	{
		assert(nearZ <= farZ);
		DirectX::XMStoreFloat4x4(&camera->mtxProjection,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixPerspectiveFovLH(
					DirectX::XMConvertToRadians(cameraAngle),
					screenAspect,
					nearZ,
					farZ)));
	}

	void Camera::ApplyPerspectiveFovRH()
	{
		assert(nearZ <= farZ);
		DirectX::XMStoreFloat4x4(&camera->mtxProjection,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixPerspectiveFovRH(
					DirectX::XMConvertToRadians(cameraAngle),
					screenAspect,
					nearZ,
					farZ)
				));
	}

	void Camera::SetConstantBuffer(ID3D12GraphicsCommandList* commandList)
	{
		DX12::ConstantBuffer::SetBuffer(commandList, *camera, DX12::ConstantBuffer::BufferSlot::Camera);
	}
}