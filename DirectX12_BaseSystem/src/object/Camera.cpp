#include "Camera.h"

namespace DX12
{
	inline bool operator!= (const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
	{
		return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
	}

	void Camera::Init(
		const DirectX::XMFLOAT3& pos,
		const DirectX::XMFLOAT3& focus,
		const DirectX::XMFLOAT2& screenSize)
	{
		this->upDirection.x = this->upDirection.z = 0;
		this->upDirection.y = 1;

		const float cameraAngle = 45.f;
		const float screenAspect = screenSize.x / screenSize.y;

		this->SetPos(pos);
		this->SetFocus(focus);

		DirectX::XMStoreFloat4x4(&camera.mtxProjection,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixPerspectiveFovLH(
					DirectX::XMConvertToRadians(cameraAngle),
					screenAspect,
					10.0f,
					1000.0f)
				));
	}

	void Camera::SetPos(const DirectX::XMFLOAT3& pos)
	{
		if (this->pos != pos)
		{
			this->pos = pos;
			if ((pos.x == focus.x) && (pos.y == focus.y) && (pos.z == focus.z))//LookAtLH—p‚Ì•â³
				this->pos.z += 0.01f;
			DirectX::XMStoreFloat4x4(&camera.mtxView,
				DirectX::XMMatrixTranspose(
					DirectX::XMMatrixLookAtLH(
						DirectX::XMLoadFloat3(&this->pos),
						DirectX::XMLoadFloat3(&this->focus),
						DirectX::XMLoadFloat3(&this->upDirection)
						)));
		}
	}

	void Camera::SetFocus(const DirectX::XMFLOAT3& focus)
	{
		if (this->focus != focus)
		{
			this->focus = focus;
			if ((pos.x == focus.x) && (pos.y == focus.y) && (pos.z == focus.z))
				pos.z += 0.01f;
			DirectX::XMStoreFloat4x4(&camera.mtxView,
				DirectX::XMMatrixTranspose(
					DirectX::XMMatrixLookAtLH(
						DirectX::XMLoadFloat3(&this->pos),
						DirectX::XMLoadFloat3(&this->focus),
						DirectX::XMLoadFloat3(&this->upDirection)
						)));
		}
	}
	
	void Camera::SetUpDirection(const DirectX::XMFLOAT3& upDirection)
	{
		DirectX::XMStoreFloat3(&this->upDirection, DirectX::XMVector3Normalize(XMLoadFloat3(&upDirection)));
		DirectX::XMStoreFloat4x4(&camera.mtxView,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixLookAtLH(
					DirectX::XMLoadFloat3(&this->pos),
					DirectX::XMLoadFloat3(&this->focus),
					DirectX::XMLoadFloat3(&this->upDirection)
					)));
	}
}