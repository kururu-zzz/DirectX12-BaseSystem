#ifndef CAMERA___
#define CAMERA___

#include <DirectXMath.h>
#include <unordered_map>

namespace DX12
{
	struct CameraBuffer
	{
		DirectX::XMFLOAT4X4	mtxView;
		DirectX::XMFLOAT4X4	mtxProjection;
		DirectX::XMFLOAT4 eyePos;
		CameraBuffer()
		{
			DirectX::XMStoreFloat4x4(&mtxView, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&mtxProjection, DirectX::XMMatrixIdentity());
			eyePos = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f);
		}
	};

	class Camera{
	private:
		CameraBuffer camera;
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 focus;
		DirectX::XMFLOAT3 upDirection;
	public:
		void Init(
			const DirectX::XMFLOAT3& pos, 
			const DirectX::XMFLOAT3& focus, 
			const DirectX::XMFLOAT2& screenSize);
		void SetPos(const DirectX::XMFLOAT3& pos);
		void SetFocus(const DirectX::XMFLOAT3& focus);
		void SetUpDirection(const DirectX::XMFLOAT3& upDirection);
	};
}

#endif